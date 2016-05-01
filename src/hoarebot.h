#ifndef HOAREBOT_H
#define HOAREBOT_H

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
#define PID_DIR "/run/"

struct getMsg
{
    char username[128];
    char text[BUFSIZ];
};
struct sendInfo
{
    int irc;
    char channel[128];
};

struct Channel
{
    pid_t PID;
    char name[128];
};

int twitchChatConnect();
int joinChannel(int irc, char *channel, char *botPass);
int chat(struct sendInfo *msgInfo, char *message);
int parseRaw(char *raw, struct getMsg *chatMsg);

#endif
