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

struct Message
{
    char username[128];
    char text[BUFSIZ];
};

struct Channel
{
    pid_t PID;
    char name[128];
};

int twitchChatConnect();
int joinChannel(int irc, char *channel, char *botPass);
int chat(int irc, char *channel, char *message);

#endif
