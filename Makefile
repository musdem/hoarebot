CC=gcc
CFLAGS=-c -Wall
CFLAGSDEBUG=-c -Wall -g

all: hoarebot clean

debug: hoarebotDebug

hoarebot: hoarebot.o lists.o irc.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot *.o

hoarebotDebug: hoarebotDebug.o listsDebug.o ircDebug.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot *.o

hoarebot.o: ./src/hoarebot.c
	$(CC) $(CFLAGS) ./src/hoarebot.c

lists.o: ./src/lists.c
	$(CC) $(CFLAGS) ./src/lists.c

irc.o: ./src/irc.c
	$(CC) $(CFLAGS) ./src/irc.c

hoarebotDebug.o: ./src/hoarebot.c
	$(CC) $(CFLAGSDEBUG) ./src/hoarebot.c

listsDebug.o: ./src/lists.c
	$(CC) $(CFLAGSDEBUG) ./src/lists.c

ircDebug.o: ./src/irc.c
	$(CC) $(CFLAGSDEBUG) ./src/irc.c


clean:
	rm ./*.o
