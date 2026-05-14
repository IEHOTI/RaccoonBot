package main

import (
	"crypto/rsa"
	"crypto/sha256"
	"crypto/tls"
	"database/sql"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"strconv"
	"time"

	"github.com/gorilla/websocket"
)

// hub — глобальный менеджер WebSocket-клиентов, через него идёт рассылка уведомлений.
var hub = newWsHub()

// upgrader переводит обычное HTTP-соединение в WebSocket.
// CheckOrigin отключён для локальной разработки; в продакшне стоит добавить проверку.
var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func preProcessing(db *myDataBase) {
	// Настройка HTTP роутинга
	http.HandleFunc("/users", db.getUserHandler)
	http.HandleFunc("/health", db.getHealthHandler)

	// WebSocket: клиент подключается сюда, чтобы получать уведомления об обновлениях
	http.HandleFunc("/ws", db.wsHandler)

	// Эндпоинт отдачи обновления: клиент запрашивает его после получения уведомления
	// и подтверждения готовности. Возвращает файл + хеш для проверки целостности.
	http.HandleFunc("/update", db.getUpdateHandler)

	// Внутренний эндпоинт для триггера рассылки при деплое нового модуля
	http.HandleFunc("/admin/notify", db.adminNotifyHandler)

	// Запуск сервера
	port := ":8080"
	log.Printf("Server starting on %s\n", port)
	log.Printf("Available endpoints:\n")
	log.Printf("  GET  http://localhost%s/users?id=*       - get * user\n", port)
	log.Printf("  GET  http://localhost%s/health           - health check\n", port)
	log.Printf("  WS   ws://localhost%s/ws                 - update notifications\n", port)
	log.Printf("  GET  http://localhost%s/update?module=*  - get update payload\n", port)
	log.Printf("  POST http://localhost%s/admin/notify     - trigger update broadcast\n", port)

	if err := http.ListenAndServe(port, nil); err != nil {
		db.database.Close()
		log.Printf("Failed to start server: %v", err)
	}
}

func (db *myDataBase) getUserHandler(w http.ResponseWriter, r *http.Request) {
	http.DefaultTransport.(*http.Transport).TLSClientConfig = &tls.Config{
		InsecureSkipVerify: true,
	} //игнорируем https
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	// Получаем ID из query параметров
	idStr := r.URL.Query().Get("id")
	if idStr == "" {
		http.Error(w, "ID parameter is required", http.StatusBadRequest)
		return
	}

	// Преобразуем ID в число
	var id int
	id, db.err = strconv.Atoi(idStr)
	if db.err != nil {
		http.Error(w, "Invalid ID format", http.StatusBadRequest)
		return
	}

	// Выполняем запрос к БД
	db.err = db.database.QueryRow(`
        SELECT id, subscribe, duration 
        FROM test 
        WHERE id = $1
    `, id).Scan(&db.currentBot.ID, &db.currentBot.Subscribe, &db.currentBot.Duration)

	if db.err != nil {
		if db.err == sql.ErrNoRows {
			db.setNewUser(id)
		} else {
			http.Error(w, db.err.Error(), http.StatusInternalServerError)
			return
		}
	}

	w.Header().Set("Content-Type", "application/json")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	json.NewEncoder(w).Encode(db.currentBot)
}

func (db *myDataBase) getHealthHandler(w http.ResponseWriter, r *http.Request) {
	http.DefaultTransport.(*http.Transport).TLSClientConfig = &tls.Config{
		InsecureSkipVerify: true,
	}
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	if db.database != nil {
		db.pingDB()
	} else {
		http.Error(w, "Database not connected", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]string{"status": "ok", "database": "connected"})
}

// wsHandler — обрабатывает WebSocket-подключение клиента.
// После апгрейда соединения клиент висит и ждёт уведомлений об обновлениях.
// Протокол подтверждения: клиент присылает {"status":"ready","module":"<n>"},
// сервер отвечает уведомлением с версией и даёт добро на скачивание через /update.
func (db *myDataBase) wsHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("Ошибка апгрейда WebSocket: %v", err)
		return
	}

	client := &wsClient{send: make(chan []byte, 16)}
	hub.register(client)
	defer hub.unregister(client)

	// Горутина записи: отправляем клиенту всё, что приходит в его канал
	go func() {
		for msg := range client.send {
			if err := conn.WriteMessage(websocket.TextMessage, msg); err != nil {
				log.Printf("Ошибка отправки WS-сообщения: %v", err)
				conn.Close()
				return
			}
		}
	}()

	// Цикл чтения: ждём подтверждения готовности клиента к обновлению
	for {
		_, msg, err := conn.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway) {
				log.Printf("WS клиент закрыл соединение неожиданно: %v", err)
			}
			return
		}

		var ack struct {
			Status string `json:"status"`
			Module string `json:"module"`
		}
		if err := json.Unmarshal(msg, &ack); err != nil {
			log.Printf("Некорректный формат подтверждения от клиента: %v", err)
			continue
		}

		if ack.Status == "ready" && ack.Module != "" {
			log.Printf("Клиент готов к обновлению модуля: %s", ack.Module)
			// Сообщаем клиенту версию — он идёт на GET /update?module=<n>
			notification := UpdateNotification{
				Module:  ack.Module,
				Version: getCurrentVersion(ack.Module),
			}
			data, _ := json.Marshal(notification)
			client.send <- data
		}
	}
}

// NotifyClients рассылает всем подключённым клиентам уведомление о выходе нового обновления.
// Вызывается при деплое нового модуля (через /admin/notify).
func NotifyClients(module, version string) {
	notification := UpdateNotification{
		Module:  module,
		Version: version,
	}
	data, err := json.Marshal(notification)
	if err != nil {
		log.Printf("Ошибка сериализации уведомления об обновлении: %v", err)
		return
	}
	hub.broadcast(data)
	log.Printf("Уведомление об обновлении модуля '%s' v%s отправлено всем клиентам", module, version)
}

// getUpdateHandler — отдаёт клиенту файл обновления и SHA-256 хеш для проверки целостности.
// Гибридная схема шифрования: тело зашифровано AES-ключом, который передаётся в заголовке
// X-Encrypted-Key, зашифрованным публичным RSA-ключом клиента из X-Client-Public-Key.
func (db *myDataBase) getUpdateHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	moduleName := r.URL.Query().Get("module")
	if moduleName == "" {
		http.Error(w, "module parameter is required", http.StatusBadRequest)
		return
	}

	// Читаем публичный RSA-ключ клиента из заголовка — им шифруем AES-ключ сессии
	clientPubKeyPEM := r.Header.Get("X-Client-Public-Key")
	if clientPubKeyPEM == "" {
		http.Error(w, "X-Client-Public-Key header is required", http.StatusBadRequest)
		return
	}

	clientPubKey, err := parseRSAPublicKey([]byte(clientPubKeyPEM))
	if err != nil {
		http.Error(w, "Invalid public key", http.StatusBadRequest)
		return
	}

	// Загружаем файл обновления с диска
	fileData, err := loadUpdateFile(moduleName)
	if err != nil {
		http.Error(w, fmt.Sprintf("Update not found for module: %s", moduleName), http.StatusNotFound)
		return
	}

	// Считаем SHA-256 хеш файла — клиент сравнит его с хешем после установки
	hash := sha256.Sum256(fileData)
	hashHex := fmt.Sprintf("%x", hash)

	// Шифруем: AES шифрует файл, RSA шифрует AES-ключ (гибридный подход)
	encryptedData, encryptedAESKey, err := encryptPayload(fileData, clientPubKey)
	if err != nil {
		http.Error(w, "Encryption error", http.StatusInternalServerError)
		log.Printf("Ошибка шифрования обновления для модуля %s: %v", moduleName, err)
		return
	}

	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("X-Update-Hash", hashHex)
	w.Header().Set("X-Module", moduleName)
	w.Header().Set("X-Module-Version", getCurrentVersion(moduleName))
	// Зашифрованный AES-ключ — в заголовке, зашифрованный файл — в теле
	w.Header().Set("X-Encrypted-Key", encodeBase64(encryptedAESKey))

	w.WriteHeader(http.StatusOK)
	w.Write(encryptedData)

	log.Printf("Обновление модуля '%s' отправлено клиенту. Хеш: %s", moduleName, hashHex)
}

// adminNotifyHandler — внутренний POST-эндпоинт для рассылки уведомления при деплое модуля.
// Body: {"module":"client","version":"1.2.0"}
func (db *myDataBase) adminNotifyHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req struct {
		Module  string `json:"module"`
		Version string `json:"version"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil || req.Module == "" {
		http.Error(w, "Invalid request body", http.StatusBadRequest)
		return
	}

	NotifyClients(req.Module, req.Version)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]string{
		"status":      "ok",
		"module":      req.Module,
		"version":     req.Version,
		"notified_at": time.Now().UTC().Format(time.RFC3339),
	})
}

// getCurrentVersion возвращает актуальную версию указанного модуля.
// В реальной системе версии хранятся в БД или конфиг-файле.
func getCurrentVersion(module string) string {
	versions := map[string]string{
		"client":  "1.0.0",
		"gui":     "1.0.0",
		"updater": "1.0.0",
	}
	if v, ok := versions[module]; ok {
		return v
	}
	return "unknown"
}

// loadUpdateFile загружает бинарный файл обновления для указанного модуля.
// Путь: ./updates/<module>.bin
func loadUpdateFile(module string) ([]byte, error) {
	path := fmt.Sprintf("./updates/%s.bin", module)
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("не удалось прочитать файл обновления %s: %w", path, err)
	}
	return data, nil
}

// --- Заглушки крипто-утилит. Реализацию вынести в crypto.go ---

// parseRSAPublicKey разбирает PEM-кодированный публичный ключ клиента.
// TODO: x509.ParsePKIXPublicKey после pem.Decode
func parseRSAPublicKey(pemData []byte) (*rsa.PublicKey, error) {
	return nil, fmt.Errorf("parseRSAPublicKey: not implemented")
}

// encryptPayload шифрует данные гибридным методом:
// 1. Генерирует случайный AES-256-GCM ключ и шифрует им data
// 2. Шифрует AES-ключ публичным RSA-ключом клиента (OAEP+SHA256)
// TODO: реализовать через crypto/aes, crypto/cipher, crypto/rand, crypto/rsa
func encryptPayload(data []byte, pubKey *rsa.PublicKey) (encryptedData []byte, encryptedKey []byte, err error) {
	return nil, nil, fmt.Errorf("encryptPayload: not implemented")
}

// encodeBase64 кодирует байты в строку base64 (StdEncoding).
// TODO: encoding/base64
func encodeBase64(data []byte) string {
	return ""
}
