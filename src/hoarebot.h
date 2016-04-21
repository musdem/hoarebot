#ifndef HOAREBOT_H
#define HOAREBOT_H

#define SERVER "irc.twitch.tv"
#define PORT 6667
#define NICK "hoarebot"

struct Channel
{
    pid_t PID;
    char name[128];
};

int joinChannel(struct Channel chnl);

#endif
