CC=gcc
CFLAGS=-c -Wall

all: hoarebot clean

hoarebot: hoarebot.o irc.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot *.o

hoarebot.o: ./src/hoarebot.c
	$(CC) $(CFLAGS) ./src/hoarebot.c

irc.o: ./src/irc.c
	$(CC) $(CFLAGS) ./src/irc.c

clean:
	rm ./*.o
