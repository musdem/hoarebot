CC=gcc
CFLAGS=-c -Wall
CFLAGSDEBUG=-c -Wall -g

all: hoarebot clean

debug: hoarebotDebug

hoarebot: main.o hoarebot.o commands.o lists.o irc.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot *.o -pthread

hoarebotDebug: mainDebug.o hoarebotDebug.o commandsDebug.o listsDebug.o ircDebug.o
	mkdir -p bin
	$(CC) -o ./bin/hoarebot *.o -pthread

main.o: ./src/main.c
	$(CC) $(CFLAGS) ./src/main.c

hoarebot.o: ./src/hoarebot.c
	$(CC) $(CFLAGS) ./src/hoarebot.c

commands.o: ./src/commands.c
	$(CC) $(CFLAGS) ./src/commands.c

lists.o: ./src/lists.c
	$(CC) $(CFLAGS) ./src/lists.c

irc.o: ./src/irc.c
	$(CC) $(CFLAGS) ./src/irc.c

mainDebug.o: ./src/main.c
	$(CC) $(CFLAGSDEBUG) ./src/main.c

hoarebotDebug.o: ./src/hoarebot.c
	$(CC) $(CFLAGSDEBUG) ./src/hoarebot.c

commandsDebug.o: ./src/commands.c
	$(CC) $(CFLAGSDEBUG) ./src/commands.c

listsDebug.o: ./src/lists.c
	$(CC) $(CFLAGSDEBUG) ./src/lists.c

ircDebug.o: ./src/irc.c
	$(CC) $(CFLAGSDEBUG) ./src/irc.c


clean:
	rm ./*.o
