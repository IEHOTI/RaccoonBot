package main

import (
	"database/sql"
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
