CC=gcc
CFLAGS=-c -Wall

all: hoarebot clean

hoarebot: hoarebot.o
	mkdir -p bin
	$(CC) hoarebot.o -o ./bin/hoarebot

hoarebot.o: ./src/hoarebot.c
	$(CC) $(CFLAGS) ./src/hoarebot.c

clean:
	rm ./*.o
