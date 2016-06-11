#ifndef HOAREBOT_H
#define HOAREBOT_H

#include <time.h>
#include <dirent.h>
#include "irc.h"

#define NUM_CMD 8
#define NUM_MOD_CMD 11
#define NUM_LISTS 4
#define PID_DIR "./run"
#define LIST_DIR "./lists"

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

typedef struct List
{
    char type[32];
    char item[BUFSIZ];
    struct List *next;
}list_t;

typedef struct RaffleEntrys
{
    char username[128];
    struct RaffleEntrys *next;
}re_t;

int runningBots(chnlL_t *CL);
void command(struct getMsg *chatMsg, struct sendMsg *botMsg);
void slots(char *username, struct sendMsg *botMsg);
void raffle(char *username, struct sendMsg *botMsg);
void social(struct sendMsg *botMsg);

#endif
