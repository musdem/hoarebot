#ifndef HOAREBOT_H
#define HOAREBOT_H

#include "irc.h"

#define NUM_CMD 9
#define NUM_MOD_CMD 11
#define PID_DIR "/run/"

struct Channel
{
    pid_t PID;
    char name[128];
};

void command(struct getMsg *chatMsg, struct sendMsg *botMsg);
void slots(char *username, struct sendMsg *botMsg);

#endif
