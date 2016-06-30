CC=gcc
CFLAGS=-c -Wall

all: hoarebot clean

hoarebot: hoarebot.o lists.o irc.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot *.o

hoarebot.o: ./src/hoarebot.c
	$(CC) $(CFLAGS) ./src/hoarebot.c

lists.o: ./src/lists.c
	$(CC) $(CFLAGS) ./src/lists.c

irc.o: ./src/irc.c
	$(CC) $(CFLAGS) ./src/irc.c

clean:
	rm ./*.o
