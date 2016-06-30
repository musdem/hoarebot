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
int itemIntList(char *listItem);
void addItem(char *listItem);
void removeItem(char *listItem);
void updateList(char *listItem, int listType, char mode, struct sendMsg *botMsg);

#endif
