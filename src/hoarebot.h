#ifndef HOAREBOT_H
#define HOAREBOT_H

struct Channel
{
    pid_t PID;
    char name[128];
};

int joinChannel(struct Channel chnl);

#endif
