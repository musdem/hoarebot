#ifndef COMMANDS_H
#define COMMANDS_H

#include <semaphore.h>
#include "lists.h"

#define NUM_CMD 8
#define NUM_MOD_CMD 12

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

typedef struct Command
{
	char arg[128];
	struct Command *next;
}cmd_t;

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

typedef struct CommandInfo
{
	struct sendMsg *botMsg;
	char username[128];
	cmd_t *parsedCmd;
}cmdInfo_t;

void getMods(struct sendMsg *botMsg);
void getSocial();
int isMod(char *username);
void freeMods();
void parseCommands(char *rawCmd, cmd_t *parsedCmd);
void freeCommands(cmd_t *parsedCmd);
void command(struct getMsg *chatMsg, struct sendMsg *botMsg);
void timeout(int seconds, cmdInfo_t *commandInfo);
void listCmd(cmdInfo_t *commandInfo);
void slots(cmdInfo_t *commandInfo);
void pasta(cmdInfo_t *commandInfo);
void modsPls(cmdInfo_t *commandInfo);
void raffle(cmdInfo_t *commandInfo);
void social(cmdInfo_t *commandInfo);
void healthy(cmdInfo_t *commandInfo);
void quote(cmdInfo_t *commandInfo);
void listModCmd(cmdInfo_t *commandInfo);
void refreshMods(cmdInfo_t *commandInfo);
void kill(cmdInfo_t *commandInfo);
void ban(cmdInfo_t *commandInfo);
void updatePasta(cmdInfo_t *commandInfo);
void removePasta(cmdInfo_t *commandInfo);
void toggleRaffle(cmdInfo_t *commandInfo);
void raffleDraw(cmdInfo_t *commandInfo);
void updateHealthy(cmdInfo_t *commandInfo);
void removeHealthy(cmdInfo_t *commandInfo);
void updateQuote(cmdInfo_t *commandInfo);
void removeQuote(cmdInfo_t *commandInfo);

#endif
