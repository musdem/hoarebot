#ifndef HOAREBOT_H
#define HOAREBOT_H

#include "irc.h"

#define PID_DIR "/run/"

struct Channel
{
    pid_t PID;
    char name[128];
};

#endif
