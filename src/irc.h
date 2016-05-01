#ifndef IRC_H
#define IRC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER "irc.twitch.tv"
#define PORT "6667"
#define BOTNICK "hoarebot"

struct getMsg
{
    char username[128];
    char text[BUFSIZ];
};
struct sendMsg
{
    int irc;
    char channel[128];
    char text[BUFSIZ];
};

int twitchChatConnect();
int joinChannel(int irc, char *channel, char *botPass);
int chat(struct sendMsg *botMsg);
int parseRaw(char *raw, struct getMsg *chatMsg);

#endif
