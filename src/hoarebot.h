#ifndef HOAREBOT_H
#define HOAREBOT_H

#include <pthread.h>
#include "commands.h"

#define PID_DIR "run"

void removePound(char *channel, char *fixedChannel);
void returnPound(char *channel, char *fixedChannel);
void createPID(struct sendMsg *botMsg);
int runningBots(chnlL_t *CL);
int isRunning(char *channel, chnlL_t *running);
void killBot(char *channel);
int initialize(char *botPass, struct sendMsg *botMsg);
int run(struct sendMsg *botMsg);
int cleanup(struct sendMsg *botMsg);

#endif
