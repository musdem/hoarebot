#ifndef HOAREBOT_H
#define HOAREBOT_H

#include <semaphore.h>
#include <pthread.h>
#include "commands.h"

void removePound(char *channel, char *fixedChannel);
void createPID(struct sendMsg *botMsg);
int runningBots(chnlL_t *CL);
int isRunning(char *channel, chnlL_t *running);
int initialize(char *botPass, struct sendMsg *botMsg);
int run(struct sendMsg *botMsg);
int cleanup(struct sendMsg *botMsg);

#endif
