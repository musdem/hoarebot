#include "lists.h"

//list variables
list_t *lists[NUM_LISTS];
int numEntry[NUM_LISTS];

void populateLists()
{
    int curList = 0;
    char line[BUFSIZ];
    struct dirent *currentItem;
    listItem_t *current;
    DIR *hoarebotListDir;
    FILE *listFile;
    hoarebotListDir = opendir(LIST_DIR);
    if(hoarebotListDir)
    {
        chdir(LIST_DIR);
        for(currentItem = readdir(hoarebotListDir);currentItem;currentItem = readdir(hoarebotListDir))
        {
            if(strcmp("..",currentItem->d_name) != 0 && strcmp(".",currentItem->d_name) != 0)//make sure that dir up and cur dir aren't considered a list
            {
                listFile = fopen(currentItem->d_name,"r");
                lists[curList] = malloc(sizeof(list_t));
                strcpy(lists[curList]->type,currentItem->d_name);
                current = lists[curList]->head;
                numEntry[curList] = 0;
                while(fgets(line,BUFSIZ,listFile))//read until the end of the file these should have no empty lines
                {
                    current = malloc(sizeof(listItem_t));
                    strcpy(current->item,currentItem->d_name);
                    numEntry[curList]++;
                    current->next = NULL;
                    current = current->next;
                }
                fclose(listFile);
                curList++;
            }
        }
        chdir("..");
        closedir(hoarebotListDir);
    }
    else
    {
        mkdir(LIST_DIR,0755);
        populateLists();
    }
}

void writeList(int listType)
{
    int listPos;
    struct dirent *currentItem;
    listItem_t *current;
    DIR *hoarebotListDir;
    FILE *listFile;
    hoarebotListDir = opendir(LIST_DIR);
    chdir(LIST_DIR);
    for(currentItem = readdir(hoarebotListDir);currentItem;currentItem = readdir(hoarebotListDir))
    {
        if(!strcmp(lists[listType]->type,currentItem->d_name))
        {
            remove(currentItem->d_name);
            listFile = fopen(currentItem->d_name,"a+");
            current = lists[listType]->head;
            for(listPos = 0;listPos < numEntry[listType];listPos++)
            {
                fputs(current->item,listFile);
                current = current->next;
            }
            fclose(listFile);
            closedir(hoarebotListDir);
            chdir("..");
            return;
        }
    }
}

int itemInList(char *listItem, int listType)
{
    listItem_t *CL;
    CL = lists[listType]->head;
    while(CL)
    {
        if(!strcmp(listItem,CL->item)) return 1;
        CL = CL->next;
    }
    return 0;
}

void addItem(char *listItem, int listType)
{
    listItem_t *CL;
    CL = lists[listType]->head;
    numEntry[listType]++;
    while(CL) CL = CL->next;
    CL = malloc(sizeof(list_t));
    strcpy(CL->item,listItem);
    CL->next = NULL;
}

int removeItem(char *listItem, int listType)
{
    listItem_t *CL, *temp;
    CL = lists[listType]->head;
    if(!strcmp(listItem,CL->item))//first item is the one to remove
    {
        lists[listType]->head = CL->next;
        free(CL);
        numEntry[listType]--;
        return 1;
    }
    while(CL->next)//everything else
    {
        if(!strcmp(listItem,CL->next->item))
        {
            temp = CL->next;
            CL->next = temp->next;
            free(temp);
            numEntry[listType]--;
            return 1;
        }
        CL = CL->next;
    }
    return 0;
}

void getRandomItem(int listType, char *item)
{
    int i, itemNum;
    listItem_t *CL;
    itemNum = rand() % numEntry[listType];
    CL = lists[listType]->head;
    for(i = 0;i < itemNum;i++) CL = CL->next;
    strcpy(item,CL->item);
}

void updateList(char *listItem, int listType, char mode, struct sendMsg *botMsg)
{
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
