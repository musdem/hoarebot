#include "hoarebot.h"

int initialize(char *botPass, struct sendMsg *botMsg)
{
    botMsg->irc = twitchChatConnect();//the irc socket file descriptor
    if(botMsg->irc == -1)
    {
        printf("Couldn't connect to twitch servers.\n");
        return -1;
    }
    if(joinChannel(botMsg, botPass) == -1)
    {
        printf("Couldn't join the channel\n");
        return -1;
    }
    createPID(botMsg);
    getMods(botMsg);
    getSocial();
    raffleNames = NULL;
    return 0;
}

int run(struct sendMsg *botMsg)
{
    int size;
    char raw[BUFSIZ], running;
    struct getMsg chatMsg;
    running = 1;
    while(running)
    {
        size = read(botMsg->irc,raw,BUFSIZ);
        if(strstr(raw,"PING :"))//make sure the bot stays connected
        {
            size = sprintf(raw,"PONG :tmi.twitch.tv\r\n");
            if(write(botMsg->irc,raw,size) == -1)
            {
                printf("couldn't write PING back server: %i\n",errno);
                return -1;
            }
        }
        else if(parseRaw(raw,&chatMsg))
        {
            if(chatMsg.text[0] == '!')
            {
                command(&chatMsg, botMsg);
            }
        }
    }
    return 0;
}

void createPID(struct sendMsg *botMsg)
{
    int i;
    char fileData[256];
    for(i = 1;botMsg->channel[i] != '\0';i++)//remove the pound symbol from the channel
    {
        fileData[i-1] = botMsg->channel[i];
    }
    fileData[i-1] = '\0';
    //create PID file for new bot
    chdir(PID_DIR);
    FILE *newPID;
    newPID = fopen(fileData,"a");
    sprintf(fileData,"%i",(int) getpid());
    if(fputs(fileData,newPID) == -1)
    {
        printf("Failed to make a PID file.\n");
    }
    fclose(newPID);
    chdir("..");//leave PID_DIR
}

int runningBots(chnlL_t *CL)
{
    int numRunning = 0;
    char pidStr[64];
    chnlL_t *current, *previous;
    DIR *hoarebotPidDir;
    FILE *pidFile;
    struct dirent *currentItem;
    hoarebotPidDir = opendir(PID_DIR);
    if(hoarebotPidDir)//dir exists so read in running channels
    {
        chdir(PID_DIR);
        previous = NULL;
        current = CL;
        for(currentItem = readdir(hoarebotPidDir);currentItem;currentItem = readdir(hoarebotPidDir))//go over every item in run dir
        {
            if(strcmp("..",currentItem->d_name) != 0 && strcmp(".",currentItem->d_name) != 0)//make sure that dir up and cur dir aren't considered a pid
            {
                pidFile = fopen(currentItem->d_name,"r");
                fgets(pidStr,64,pidFile);
                fclose(pidFile);
                current->PID = (pid_t) atoi(pidStr);
                strcpy(current->name,currentItem->d_name);
                current->next = malloc(sizeof(chnlL_t));
                current->next->next = NULL;
                previous = current;
                current = current->next;
                numRunning++;
            }
        }
        closedir(hoarebotPidDir);
        if(previous == NULL)//if there are no bots running set the head to null
        {
            CL = NULL;
        }
        else//free the last node
        {
            free(previous->next);
            previous->next = NULL;
        }
        return numRunning;
    }
    else//make hoarebot pid directory and rerun function
    {
        mkdir(PID_DIR,0700);
        return runningBots(CL);
    }
}

int main(int argc, char *argv[])
{
    int numRunning;
    char botPass[37];
    chnlL_t channelList, *current;
    struct sendMsg botMsg;
    FILE *passFile;
    passFile = fopen("pass","r");
    if(!passFile)
    {
        printf("IRC oauth file not found.\n");
        return -1;
    }
    fgets(botPass,37,passFile);
    fclose(passFile);
    numRunning = runningBots(&channelList);
    srand(time(NULL));//seed rng with current time
    if(argc < 2)//no arguments were added to the command so list running bots
    {
        if(numRunning)
        {
            current = &channelList;
            printf("Current bots running: %i\n",numRunning);
            while(current != NULL)
            {
                printf("Channel: %s\nPID: %i\n", current->name, (int) current->PID);
                current = current->next;
            }
        }
        else
        {
            printf("No bots are currently running.\n");
        }
        return 0;
    }
    else if(argc > 2)//too many arguments were provided
    {
        printf("Too many arguments\n");
        return 0;
    }
    else//argument provided this should be a channel to join
    {
        //I'll just remove this until later
        //TODO have clean bot shutdown and clean up
        //make a channels directory for all channel data
        /*if(!mkdir(channel,0700))//make directory for bot using channel and make it currect working directory
        {
            chdir(channel);
        }
        else
        {
            printf("Could not create %s directory\nIs there a bot already running on that channel?\n",channel);
            return -1;
        }*/
        switch(daemon(1,0))
        {
            case -1:
                printf("Daemonizing has failed\n");
                return -1;
            case 0:
                strcpy(botMsg.channel,argv[1]);
                break;
            default:
                printf("You shouldn't have gotten here...\n");
                return -1;
        }
    }
    initialize(botPass, &botMsg);
    run(&botMsg);
    return 0;
}
