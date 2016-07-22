#ifndef HOAREBOT_H
#define HOAREBOT_H

#include <time.h>
#include "lists.h"

#define NUM_CMD 8
#define NUM_MOD_CMD 11
#define PID_DIR "./run"

#define WILL_FORCE_3 20

#define FACEBOOK_SET 8
#define TWITTER_SET 4
#define YOUTUBE_SET 2
#define MAL_SET 1

typedef struct Channel
{
    pid_t PID;
    char name[128];
    struct Channel *next;
}chnlL_t;

typedef struct RaffleEntrys
{
    char username[128];
    struct RaffleEntrys *next;
}re_t;

typedef struct ModList
{
	char mod[128];
	struct ModList *next;
}ml_t;

void createPID(struct sendMsg *botMsg);
int runningBots(chnlL_t *CL);
void getMods(struct sendMsg *botMsg);
int isMod(char *username);
int inSC(char *cmd);
int argPos(char *cmd, int argNum);
int stripCmdInput(char *cmd);
void command(struct getMsg *chatMsg, struct sendMsg *botMsg);
void timeout(int seconds, char *username, struct sendMsg *botMsg);
void ban(char *username, struct sendMsg *botMsg);
void slots(char *username, struct sendMsg *botMsg);
void raffle(char *username, struct sendMsg *botMsg);
void drawRaffle(struct sendMsg *botMsg);
void social(struct sendMsg *botMsg);

#endif
