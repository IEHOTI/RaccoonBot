package main

import (
	"database/sql"
	"fmt"
	"log"
	"time"

	_ "github.com/lib/pq"
)

func (db *myDataBase) pingDB() {
	db.err = db.database.Ping()
	if db.err != nil {
		db.database.Close()
		log.Fatal("Failed ping to db: %v", db.err)
	}
}

func (db *myDataBase) checkDB() {
	if db.database != nil {
		db.pingDB()
	}

	connStr := fmt.Sprintf("host=%s port=%d user=%s password=%s sslmode=%s dbname=%s",
		config.Host, config.Port, config.User, config.Password, config.SSLMode, config.DBName)

	db.database, db.err = sql.Open("postgres", connStr)
	if db.err != nil {
		db.database.Close()
		log.Fatal("Failed to connect to PostgreSQL: %v", db.err)
	}

	db.pingDB()

	log.Printf("successfully connected to database")
}

func (db *myDataBase) setNewUser(id int) {
	sqlStatement := `INSERT INTO test (id,subscribe,duration) VALUES ($1, $2, $3)`
	db.currentBot.ID = id
	db.currentBot.Subscribe = "trial"
	db.currentBot.Duration = time.Now().UTC().AddDate(0, 0, 3)
	_, db.err = db.database.Exec(sqlStatement, db.currentBot.ID, db.currentBot.Subscribe, db.currentBot.Duration)
	if db.err != nil {
		log.Printf("Ошибка вставки: %v\n", db.err)
		return
	}

	log.Printf("Запись " + sqlStatement + " успешно добавлена")
	return
}
