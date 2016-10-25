#include "hoarebot.h"

//bot command variables
char commands[NUM_CMD][32] = {"!commands","!slots","!pasta","!modspls","!raffle","!social","!healthy","!quote"};
char secretCommands[NUM_MOD_CMD][32] = {"!modcommands","!refreshmods","!ban","!updatepasta","!removepasta","!toggleraffle","!raffledraw","!updatehealthy","!removehealthy","!updatequote","!removequote"};
//slots variables
int force = 1;
char emotes[9][16] = {"Kappa","KappaPride","EleGiggle","BibleThump","PogChamp","TriHard","CoolCat","WutFace","Kreygasm"};
//raffle variables
int raffleStatus = 0;
int numEntrants = 0;
re_t *raffleNames;
//list of mods
ml_t *mods;
//social variables
char socialSetVar = 0;
char streamerName[256];
char facebook[256];
char twitter[256];
char youtube[256];
char MAL[256];

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

void getMods(struct sendMsg *botMsg)
{
    int repeat, colonCount, rawPos, curModPos;
    char rawMod[BUFSIZ];
    ml_t *current;
    strcpy(botMsg->text,"/mods");
    chat(botMsg);
    repeat = 0;
    colonCount = 3;
    rawPos = 0;
    curModPos = 0;
    do
    {
        read(botMsg->irc,rawMod,BUFSIZ);
        repeat++;
    }
    while(repeat != 100 && strstr(rawMod,"The moderators of this room are: ") == NULL);
    while(colonCount)//put rawPos at the start of the mod list
    {
        if(rawMod[rawPos] == ':') colonCount--;
        rawPos++;
    }
    mods = malloc(sizeof(ml_t));
    current = mods;
    current->next = NULL;
    for(rawPos += 1;rawMod[rawPos] != '\r';rawPos++)
    {
        if(rawMod[rawPos] != ',')
        {
            current->mod[curModPos] = rawMod[rawPos];
            curModPos++;
        }
        else
        {
            current->mod[curModPos] = '\0';
            current->next = malloc(sizeof(ml_t));
            current->next->next = NULL;
            current = current->next;
            rawPos++;//put rawPos at space in front of next name on list
            curModPos = 0;
        }
    }
    current->mod[curModPos] = '\0';//end off the last mod in the list
}

void getSocial()
{
    int linePos;
    char currentLine[256];
    FILE *socialFile;
    socialFile = fopen("socialInfo","r");
    if(socialFile == NULL)
    {
        fclose(fopen("socialInfo","w+"));//create file if it doesn't exist
        return;
    }
    while(fgets(currentLine,256,socialFile))
    {
        switch(currentLine[0])
        {
            case 'S'://streamer name
                for(linePos = 1;currentLine[linePos] != '\n';linePos++)
                {
                    streamerName[linePos - 1] = currentLine[linePos];
                }
                break;
            case 'F'://facebook link
                socialSetVar |= FACEBOOK_SET;
                for(linePos = 1;currentLine[linePos] != '\n';linePos++)
                {
                    facebook[linePos - 1] = currentLine[linePos];
                }
                break;
            case 'T'://twitter link
                socialSetVar |= TWITTER_SET;
                for(linePos = 1;currentLine[linePos] != '\n';linePos++)
                {
                    twitter[linePos - 1] = currentLine[linePos];
                }
                break;
            case 'Y'://youtube link
                socialSetVar |= YOUTUBE_SET;
                for(linePos = 1;currentLine[linePos] != '\n';linePos++)
                {
                    youtube[linePos - 1] = currentLine[linePos];
                }
                break;
            case 'M'://my anime list link
                socialSetVar |= MAL_SET;
                for(linePos = 1;currentLine[linePos] != '\n';linePos++)
                {
                    MAL[linePos - 1] = currentLine[linePos];
                }
                break;
            default:
                break;
        }
    }
    fclose(socialFile);
}

int isMod(char *username)
{
    ml_t *current;
    for(current = mods;current != NULL;current = current->next)
    {
        if(!strcmp(current->mod,username)) return 1;
    }
    return 0;
}

int inSC(char *cmd)
{
    int curSC;
    for(curSC = 0;curSC < NUM_MOD_CMD;curSC++)
    {
        if(!strstr(cmd,secretCommands[curSC])) return 1;
    }
    return 0;
}

int argPos(char *cmd, int argNum)
{
    int cmdArgPos = 0;
    while(argNum != 0)
    {
        cmdArgPos++;
        if(cmd[cmdArgPos] == ' ') argNum--;//if an argument is found decrement
        else if(cmd[cmdArgPos] == '\0') return 0;//if end of cmd is reached
    }
    return cmdArgPos + 1;//the start of the selected command argument
}

int stripCmdInput(char *cmd)
{
    int curCmdPos, curArgPos;
    curCmdPos = argPos(cmd,1);
    curArgPos = 0;
    if(curCmdPos)
    {
        while(cmd[curCmdPos] != '\0')
        {
            cmd[curArgPos] = cmd[curCmdPos];
            curCmdPos++;
            curArgPos++;
        }
    }
    else return 0;
    cmd[curArgPos] = '\0';
    return 1;
}

void command(struct getMsg *chatMsg, struct sendMsg *botMsg)
{
    if(!strcmp(chatMsg->text,commands[0]))//!commands
    {
        int i;
        for(i = 0;i < NUM_CMD - 1;i++)
        {
            strcat(botMsg->text,commands[i]);
            strcat(botMsg->text,", ");
        }
        strcat(botMsg->text,commands[i]);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[1]))//!slots
    {
        slots(chatMsg->username, botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[2]))//!pasta
    {
        getRandomItem(0,botMsg->text);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[3]))//!modsPls
    {
        getRandomItem(3,botMsg->text);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[4]))//!raffle
    {
        if(raffleStatus)
        {
            raffle(chatMsg->username, botMsg);
        }
        else
        {
            strcpy(botMsg->text,"There is no draw dumbass FailFish");
            chat(botMsg);
        }
    }
    else if(!strcmp(chatMsg->text, commands[5]))//!social
    {
        social(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[6]))//!healthy
    {
        getRandomItem(1,botMsg->text);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[7]))//!quote
    {
        getRandomItem(2,botMsg->text);
        chat(botMsg);
    }
    else if(inSC(chatMsg->text))
    {
        if(isMod(chatMsg->username))//check for mod status
        {
            if(strstr(chatMsg->text,secretCommands[0]))//!modcommands
            {
                int i;
                for(i = 0;i < NUM_CMD - 1;i++)
                {
                    strcat(botMsg->text,secretCommands[i]);
                    strcat(botMsg->text,", ");
                }
                strcat(botMsg->text,secretCommands[i]);
                chat(botMsg);
            }
            else if(strstr(chatMsg->text,secretCommands[1]))//!refreshmods
            {
                getMods(botMsg);
            }
            else if(strstr(chatMsg->text,secretCommands[2]))//!ban
            {
                if(stripCmdInput(chatMsg->text))
                {
                    ban(chatMsg->text,botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the ban name! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[3]))//!updatepasta
            {
                if(stripCmdInput(chatMsg->text))
                {
                    updateList(chatMsg->text,0,'w',botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the pasta to add! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[4]))//!removepasta
            {
                if(stripCmdInput(chatMsg->text))
                {
                    updateList(chatMsg->text,0,'d',botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the pasta to remove! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[5]))//!toggleraffle
            {
                if(raffleStatus)
                {
                    raffleStatus = 0;
                    strcpy(botMsg->text,"Raffle closed.");
                    chat(botMsg);
                }
                else
                {
                    raffleStatus = 1;
                    strcpy(botMsg->text,"Raffle open!");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[6]))//!raffledraw
            {
                if(!raffleStatus)
                {
                    if(numEntrants == 0)
                    {
                        strcpy(botMsg->text,"No one has entered the draw!");
                        chat(botMsg);
                    }
                    else
                    {
                        drawRaffle(botMsg);
                    }
                }
                else
                {
                    strcpy(botMsg->text,"The raffle is still on! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[7]))//!updatehealthy
            {
                if(stripCmdInput(chatMsg->text))
                {
                    updateList(chatMsg->text,1,'w',botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the lewd to add! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[8]))//!removehealthy
            {
                if(stripCmdInput(chatMsg->text))
                {
                    updateList(chatMsg->text,1,'d',botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the lewd to remove! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[9]))//!updatequote
            {
                if(stripCmdInput(chatMsg->text))
                {
                    updateList(chatMsg->text,2,'w',botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the quote to add! FailFish");
                    chat(botMsg);
                }
            }
            else if(strstr(chatMsg->text,secretCommands[10]))//!removequote
            {
                if(stripCmdInput(chatMsg->text))
                {
                    updateList(chatMsg->text,2,'d',botMsg);
                }
                else
                {
                    strcpy(botMsg->text,"You forgot the quote to remove! FailFish");
                    chat(botMsg);
                }
            }
        }
        else//non mod trying to use mod cmd
        {
            strcpy(botMsg->text,"You aren't a mod! DansGame");
            chat(botMsg);
        }
    }
    else//command isn't real
    {
        sprintf(botMsg->text,"%s is not a command; type !commands to get a list.",chatMsg->text);
        chat(botMsg);
    }
}

void timeout(int seconds, char *username, struct sendMsg *botMsg)
{
    sprintf(botMsg->text,"/timeout %s %i",username,seconds);
    chat(botMsg);
}

void ban(char *username, struct sendMsg *botMsg)
{
    strcpy(botMsg->text,"░░░░░░░░░░░░ ▄████▄░░░░░░░░░░░░░░░░░░░░ ██████▄░░░░░░▄▄▄░░░░░░░░░░ ░███▀▀▀▄▄▄▀▀▀░░░░░░░░░░░░░ ░░░▄▀▀▀▄░░░█▀▀▄░▄▀▀▄░█▄░█░ ░░░▄▄████░░█▀▀▄░█▄▄█░█▀▄█░ ░░░░██████░█▄▄▀░█░░█░█░▀█░ ░░░░░▀▀▀▀░░░░░░░░░░░░░░░░░");
    chat(botMsg);
    sprintf(botMsg->text,"/ban %s",username);
    chat(botMsg);
}

void slots(char *username, struct sendMsg *botMsg)
{
    if((rand() % force) - 10 > WILL_FORCE_3)//crappy PRNG
    {
        force = rand() % 9;
        sprintf(botMsg->text,"%s | %s | %s",emotes[force],emotes[force],emotes[force]);
        force = 1;
    }
    else
    {
        sprintf(botMsg->text,"%s | %s | %s",emotes[rand() % 9],emotes[rand() % 9],emotes[rand() % 9]);
        force++;
    }
    chat(botMsg);
    if(!strcmp(botMsg->text,"Kappa | Kappa | Kappa"))
    {
        sprintf(botMsg->text,"Goodbye %s Kappa",username);
        chat(botMsg);
        timeout(0,username,botMsg);
    }
    else if(!strcmp(botMsg->text,"TriHard | TriHard | TriHard"))
    {
        int i, j;
        int pos = 8;
        char TH[10] = " TriHard ";
        strcpy(botMsg->text,"TriHard ");
        for(i = 0;i < strlen(username);i++)
        {
            if(username[i] >= 97)//if the chracter in the username isn't capitalized capitalize it
            {
                botMsg->text[pos] = username[i] - 32;
            }
            else//if the character is anything else
            {
                botMsg->text[pos] = username[i];
            }
            pos++;
            for(j = 0;j < 9;j++)
            {
                botMsg->text[pos] = TH[j];
                pos++;
            }
        }
        botMsg->text[pos] = '\0';//terminate string with null character
        strcat(botMsg->text,"TriHard H TriHard Y TriHard P TriHard E TriHard ! TriHard");
        chat(botMsg);
    }
    else if(!strcmp(botMsg->text,"Kreygasm | Kreygasm | Kreygasm"))
    {
        sprintf(botMsg->text,"Kreygasm For later tonight %s https://www.youtube.com/watch?v=P-Vvm7M4Lig Kreygasm",username);
        chat(botMsg);
    }
}

void raffle(char *username, struct sendMsg *botMsg)
{
    if(raffleNames == NULL)//if the list of enterants has not been made yet
    {
        raffleNames = malloc(sizeof(re_t));
        strcpy(raffleNames->username,username);
        raffleNames->next = NULL;
    }
    else
    {
        re_t *current;
        for(current = raffleNames;current != NULL;current = current->next)
        {
            if(!strcmp(current->username,username))//check if the user has already entered
            {
                sprintf(botMsg->text,"%s, you are already in the draw! FailFish",username);
                chat(botMsg);
                return;
            }
        }
        current = malloc(sizeof(re_t));
        strcpy(current->username,username);
        current->next = NULL;
    }
    sprintf(botMsg->text,"%s has entered the draw.",username);
    chat(botMsg);
    numEntrants++;
}

void drawRaffle(struct sendMsg *botMsg)
{
    int winner;
    re_t *current;
    current = raffleNames;
    winner = rand() % numEntrants;
    while(winner != 0)
    {
        current = current->next;
        winner--;
    }
    sprintf(botMsg->text,"The winner is %s!",current->username);
    chat(botMsg);
    for(;numEntrants != 0;numEntrants--)//remove all entrys from list
    {
        current = raffleNames;//reset list
        while(current->next != NULL) current = current->next;//loop to the end of the list to remove the item
        free(current);
        current = NULL;
    }
    raffleNames = NULL;
}

void social(struct sendMsg *botMsg)
{
    if(socialSetVar)
    {
        char message[512];
        if(socialSetVar & FACEBOOK_SET)
        {
            sprintf(message,"Like %s on facebook at %s ",streamerName,facebook);
            strcat(botMsg->text,message);
        }
        if(socialSetVar & TWITTER_SET)
        {
            sprintf(message,"Follow %s on twitter at %s ",streamerName,twitter);
            strcat(botMsg->text,message);
        }
        if(socialSetVar & YOUTUBE_SET)
        {
            sprintf(message,"Subscribe to %s on youtube at %s ",streamerName,youtube);
            strcat(botMsg->text,message);
        }
        if(socialSetVar & MAL_SET)
        {
            sprintf(message,"Friend %s on MAL at %s ",streamerName,MAL);
            strcat(botMsg->text,message);
        }
    }
    else
    {
        strcpy(botMsg->text,"No social links BibleThump");
    }
    chat(botMsg);
}
