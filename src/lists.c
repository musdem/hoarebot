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
            if(strcmp("..",currentItem->d_name) != 0 && strcmp(".",currentItem->d_name) != 0)//make sure that dir up and cur dir aren't considered a list
            {
                listFile = fopen(currentItem->d_name,"r");
                lists[curList] = malloc(sizeof(list_t));
                current = lists[curList];
                numEntry[curList] = 0;
                while(fgets(line,BUFSIZ,listFile))//read until the end of the file these should have no empty lines
                {
                    if(newItem)
                    {
                        strcpy(current->type,currentItem->d_name);
                        newItem = 0;
                    }
                    else
                    {
                        current->next = malloc(sizeof(list_t));
                        strcpy(current->next->item,currentItem->d_name);
                        numEntry[curList]++;
                        current->next->next = NULL;
                        current = current->next;
                    }
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

int itemInList(char *listItem, int listType)
{
    list_t *CL;
    CL = lists[listType];
    while(CL)//iterate over the list until the item is found then return 1
    {
        if(!strcmp(listItem,CL->item))
        {
            return 1;
        }
        CL = CL->next;
    }
    return 0;//if nothing is found then return 0
}

void addItem(char *listItem, int listType)
{
    list_t *CL;
    CL = lists[listType];
    numEntry[listType]++;
    while(CL->next)//iterate over list until the next item is NULL
    {
        CL = CL->next;
    }
    CL->next = malloc(sizeof(list_t));
    strcpy(CL->next->item,listItem);
    CL->next->next = NULL;
}

int removeItem(char *listItem, int listType)
{
    list_t *CL, *temp;
    CL = lists[listType];
    if(!strcmp(listItem,CL->item))//if the first item is the item to remove
    {
        temp = CL->next;
        free(CL);
        CL = temp;
        numEntry[listType]--;
        return 1;
    }
    while(CL)
    {
        if(!strcmp(listItem,CL->next->item))
        {
            if(CL->next->next == NULL)//if the last item in the list is the item to remove
            {
                free(CL->next);
                CL->next = NULL;
            }
            else//item in the middle of the list
            {
                temp = CL->next;
                CL->next = temp->next;
                free(temp);
            }
            numEntry[listType]--;
            return 1;
        }
        CL = CL->next;
    }
    return 0;//return -1 if item isn't in the list
}

char *getRandomItem(int listType)
{
    int i, item;
    list_t *CL;
    item = rand() % numEntry[listType];
    CL = lists[listType];
    for(i = 0;i < item;i++)
    {
        CL = CL->next;
    }
    return CL->item;
}

void updateList(char *rawCmd, int listType, char mode, struct sendMsg *botMsg)
{
    int i, j;
	j = 0;
	char listItem[BUFSIZ];
	for(i = argPost(rawCmd);rawCmd[i] != '\0';i++)//get list item from rawCmd
	{
		listItem[j] = rawCmd[i];
		j++;
	}
	if(mode == 'w')
    {
        if(!itemInList(listItem,listType))
        {
            addItem(listItem,listType);
            strcpy(botMsg->text,"Added item from the list.");
            chat(botMsg);
        }
        else
        {
            strcpy(botMsg->text,"That is already in the list FailFish");
            chat(botMsg);
        }
    }
    else if(mode == 'd')
    {
        if(removeItem(listItem,listType))
        {
            strcpy(botMsg->text,"Removed item from the list.");
            chat(botMsg);
        }
        else
        {
            strcpy(botMsg->text,"That item isn't in the list FailFish");
            chat(botMsg);
        }
    }
}
