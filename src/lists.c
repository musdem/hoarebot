#include "lists.h"
//list variables
list_t *lists[NUM_LISTS];
int numEntry[NUM_LISTS];

void populateLists()
{
    int curList = 0;
    char line[BUFSIZ], newItem;
    struct dirent *currentItem;
    list_t *current;
    DIR *hoarebotListDir;
    FILE *listFile;
    hoarebotListDir = opendir(LIST_DIR);
    newItem = 1;
    if(hoarebotListDir)
    {
        for(currentItem = readdir(hoarebotListDir);currentItem;currentItem = readdir(hoarebotListDir))
        {
            if(!strcmp("..",currentItem->d_name) || !strcmp(".",currentItem->d_name))//make sure that dir up and cur dir aren't considered a list
            {
                listFile = fopen(currentItem->d_name,"r");
                lists[curList] = malloc(sizeof(list_t));
                current = lists[curList];
                while(fgets(line,BUFSIZ,listFile))//read until the end of the file these should have no empty lines
                {
                    if(newItem)
                    {
                        strcpy(current->type,currentItem->d_name);
                        newItem = 0;
                    }
                    else
                    {
                        strcpy(current->item,currentItem->d_name);
                        numEntry[curList]++;
                    }
                    current->next = malloc(sizeof(list_t));
                    current->next->next = NULL;
                    current = current->next;
                }
                fclose(listFile);
                curList++;
                newItem = 1;
            }
        }
        closedir(hoarebotListDir);
    }
    else
    {
        mkdir(LIST_DIR,0700);
        populateLists();
    }
}

int itemInList(char *listItem)
{
    //TODO iterate over list and make sure item isn't in it
    return 0;
}

void addItem(char *listItem)
{
    //TODO add item to end of the list
}

void removeItem(char *listItem)
{
    //TODO remove item from where ever it is
}

void updateList(char *listItem, int listType, char mode, struct sendMsg *botMsg)
{
    if(mode == 'w')
    {
        if(!itemInList(listItem))
        {
            addItem(listItem);
        }
        else
        {
            strcpy(botMsg->text,"That is already in the list FailFish");
            chat(botMsg);
        }
    }
}
