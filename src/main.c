#include "hoarebot.h"

int running = 1;

void removePound(char *channel, char *fixedChannel)
{
    int i;
    for(i = 1;channel[i] != '\0';i++)//remove the pound symbol from the channel
    {
        fixedChannel[i-1] = channel[i];
    }
    fixedChannel[i-1] = '\0';
}

void createPID(struct sendMsg *botMsg)
{
    char fileData[64];
    removePound(botMsg->channel,fileData);
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
        chdir("..");
        return numRunning;
    }
    else//make hoarebot pid directory and rerun function
    {
        mkdir(PID_DIR,0644);
        return runningBots(CL);
    }
}

int initialize(char *botPass, struct sendMsg *botMsg)
{
    char botDir[64];
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
    removePound(botMsg->channel,botDir);
    if(chdir(botDir) != 0)
    {
        if(mkdir(botDir,0700) != 0)
        {
            printf("error making bot directory: %i\n",errno);
            return -1;
        }
        if(chdir(botDir) != 0)
        {
            printf("error changing to bot directory (%s): %i\n",botDir,errno);
            return -1;
        }
    }
    getMods(botMsg);
    getSocial();
    return 0;
}

int run(struct sendMsg *botMsg)
{
    int size;
    char raw[BUFSIZ];
    struct getMsg chatMsg;
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

int cleanup(struct sendMsg *botMsg)
{
    char fileData[64];
    sprintf(fileData,"../%s",PID_DIR);
    chdir(fileData);
    botMsg->channel[0] = '/';
    sem_unlink(botMsg->channel);
    botMsg->channel[0] = '#';
    removePound(botMsg->channel,fileData);
    if(remove(fileData) != 0)
    {
        printf("couldn't remove PID file: %i\n",errno);
        return -1;
    }
    return 0;
}

static void *checkRunning(void *channel)
{
    char *c = (char*) channel;
    sem_t *stopBot;
    c[0] = '/';
    sem_unlink(c);
    stopBot = sem_open(c,(O_CREAT | O_EXCL),0600,0);
    if(stopBot == NULL)
    {
        printf("couldn't create semaphore: %i\n",errno);
        running = 0;
        pthread_detach(pthread_self());
    }
    c[0] = '#';
    sem_wait(stopBot);
    sem_close(stopBot);
    running = 0;
    pthread_detach(pthread_self());
    return NULL;
}

int main(int argc, char *argv[])
{
    int numRunning,opt,verbose,kill;
    char botPass[37];
    chnlL_t channelList, *current;
    struct sendMsg botMsg;
    FILE *passFile;
    verbose = 0;
    kill = 0;
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
    while((opt = getopt(argc,argv,"kv")) != -1)
    {
        switch(opt)
        {
            case 'v':
                verbose = 1;
                break;
            case 'k':
                kill = 1;
                break;
            default:
                printf("Command usage: hoarebot [-k kill selected bot | -v enable verbose mode] [channel]\n");
                return -1;
        }
    }
    if(argc == 1)//no arguments were added to the command so list running bots
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
    else if(kill)
    {
        sem_t *stopBot;
        strcpy(botMsg.channel,argv[optind]);
        botMsg.channel[0] = '/';
        stopBot = sem_open(botMsg.channel,0);
        if(stopBot == NULL)
        {
            printf("couldn't create semaphore: %i\n",errno);
            return -1;
        }
        sem_post(stopBot);
        botMsg.channel[0] = ' ';
        printf("killing%s\n",botMsg.channel);
        sem_close(stopBot);
        return 0;
    }
    else//argument provided this should be a channel to join
    {
        switch(daemon(1,verbose))
        {
            case -1:
                printf("Daemonizing has failed\n");
                return -1;
            case 0:
                strcpy(botMsg.channel,argv[optind]);
                break;
            default:
                printf("You shouldn't have gotten here...\n");
                return -1;
        }
    }
    int runThreadStatus;
    pthread_t runThread;
    runThreadStatus = pthread_create(&runThread,NULL,checkRunning,botMsg.channel);
    if(runThreadStatus != 0)
    {
        printf("run thread failed: %i\n",errno);
        return -1;
    }
    if(initialize(botPass, &botMsg) != 0)
    {
        printf("failed to initialize the bot\n");
        return -1;
    }
    if(run(&botMsg) != 0)
    {
        printf("bot run error\n");
        return -1;
    }
    if(cleanup(&botMsg) != 0)
    {
        printf("failed to clean up bot\nyou need to manually clean up\n");
        return -1;
    }
    strcpy(botMsg.text,"/me ~rosebud~");
    chat(&botMsg);
    return 0;
}
