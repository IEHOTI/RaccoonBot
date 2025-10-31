package main

import (
	"crypto/tls"
	"database/sql"
	"encoding/json"
	"log"
	"net/http"
	"strconv"
)

func preProcessing(db *myDataBase) {
	// Настройка HTTP роутинга
	http.HandleFunc("/users", db.getUserHandler)
	http.HandleFunc("/health", db.getHealthHandler)
	// Запуск сервера
	port := ":8080"
	log.Printf("Server starting on %s\n", port)
	log.Printf("Available endpoints:\n")
	log.Printf("  GET http://localhost%s/users?id=* - get * user\n", port)
	log.Printf("  GET http://localhost%s/health - health check\n", port)

	if db.err = http.ListenAndServe(port, nil); db.err != nil {
		db.database.Close()
		log.Printf("Failed to start server: %v", db.err)
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
