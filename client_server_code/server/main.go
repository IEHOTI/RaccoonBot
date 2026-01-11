package main

var config = Config{
	Host:     "localhost",
	Port:     5432,
	User:     "test_user",
	Password: "123456",
	DBName:   "example_test",
	SSLMode:  "disable",
}

func main() {
	db := &myDataBase{}
	db.checkDB()
	preProcessing(db)
}
