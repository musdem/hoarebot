#ifndef LISTS_H
#define LISTS_H

#include <dirent.h>
#include "irc.h"

#define NUM_LISTS 4
#define LIST_DIR "./lists"

typedef struct List
{
    char type[32];
    char item[BUFSIZ];
    struct List *next;
}list_t;

void populateLists();
int itemInList(char *listItem, int listType);
void addItem(char *listItem, int listType);
int removeItem(char *listItem, int listType);
void getRandomItem(int listType, char *item);
void updateList(char *rawCmd, int listType, char mode, struct sendMsg *botMsg);

#endif
