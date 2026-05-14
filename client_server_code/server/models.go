package main

import (
	"database/sql"
	"log"
	"sync"
	"time"
)

type Bot struct {
	ID        int       `json:"id"`
	Subscribe string    `json:"subscribe"`
	Duration  time.Time `json:"duration"`
}

type Config struct {
	Host     string
	Port     int
	User     string
	Password string
	DBName   string
	SSLMode  string
}

type myDataBase struct {
	database   *sql.DB
	currentBot Bot
	err        error
}

// UpdateNotification — уведомление об обновлении, которое сервер рассылает клиентам.
type UpdateNotification struct {
	Module  string `json:"module"`
	Version string `json:"version"`
}

// UpdatePayload — пакет с файлами обновления и хешем для проверки целостности.
type UpdatePayload struct {
	Module  string `json:"module"`
	Version string `json:"version"`
	Hash    string `json:"hash"`
	Data    []byte `json:"data"`
}

// wsClient — одно подключённое по WebSocket клиентское соединение.
type wsClient struct {
	send chan []byte
}

// wsHub управляет всеми активными WebSocket-клиентами и рассылкой уведомлений.
type wsHub struct {
	mu      sync.RWMutex
	clients map[*wsClient]bool
}

func newWsHub() *wsHub {
	return &wsHub{
		clients: make(map[*wsClient]bool),
	}
}

func (h *wsHub) register(c *wsClient) {
	h.mu.Lock()
	defer h.mu.Unlock()
	h.clients[c] = true
	log.Printf("Клиент подключился. Активных клиентов: %d", len(h.clients))
}

func (h *wsHub) unregister(c *wsClient) {
	h.mu.Lock()
	defer h.mu.Unlock()
	delete(h.clients, c)
	close(c.send)
	log.Printf("Клиент отключился. Активных клиентов: %d", len(h.clients))
}

// broadcast рассылает сообщение всем подключённым клиентам.
func (h *wsHub) broadcast(message []byte) {
	h.mu.RLock()
	defer h.mu.RUnlock()
	for c := range h.clients {
		select {
		case c.send <- message:
		default:
			// Канал переполнен — клиент не успевает читать, пропускаем
		}
	}
}
