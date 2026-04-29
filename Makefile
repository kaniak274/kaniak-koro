CC = gcc
CFLAGS = -Wall -Wextra

build:
	$(CC) -c sqlite3.c -o sqlite3.o -w
	$(CC) main.c sqlite3.o $(CFLAGS) -o main
