CC=gcc
CFLAGS=-c -Wall

all: hoarebot clean

hoarebot: hoarebot.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot hoarebot.o

hoarebot.o: ./src/hoarebot.c ./src/hoarebot.h
	$(CC) $(CFLAGS) ./src/hoarebot.c

clean:
	rm ./*.o
