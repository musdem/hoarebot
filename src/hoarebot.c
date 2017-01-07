#include "hoarebot.h"

//bot command variables
char commands[NUM_CMD][32] = {"!commands","!slots","!pasta","!modspls","!raffle","!social","!healthy","!quote"};
char secretCommands[NUM_MOD_CMD][32] = {"!modcommands","!refreshmods","!ban","!updatepasta","!removepasta","!toggleraffle","!raffledraw","!updatehealthy","!removehealthy","!updatequote","!removequote"};
void (*cmd[NUM_CMD])(cmdInfo_t*) = {&listCmd,&slots,&pasta,&modsPls,&raffle,&social,&healthy,&quote};
void (*modCmd[NUM_MOD_CMD])(cmdInfo_t*) = {&listModCmd,&refreshMods,&ban,&updatePasta,&removePasta,&toggleRaffle,&raffleDraw,&updateHealthy,&removeHealthy,&updateQuote,&removeQuote};
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

void parseCommands(char *rawCmd, cmd_t *parsedCmd)
{
    int cmdPos, cmdOffset;
    cmd_t *current = parsedCmd;
    for(cmdPos = 0, cmdOffset = 0;rawCmd[cmdPos] != '\0';cmdPos++, cmdOffset++)
    {
        if(rawCmd[cmdPos] != ' ')
        {
            current->arg[cmdOffset] = rawCmd[cmdPos];
        }
        else
        {
            current->arg[cmdOffset] = '\0';
            cmdOffset = -1;
            current->next = malloc(sizeof(cmd_t));
            current = current->next;
            current->next = NULL;
        }
    }
    current->arg[cmdOffset] = '\0';
}

void command(struct getMsg *chatMsg, struct sendMsg *botMsg)
{
    int curCmd;
    cmdInfo_t commandInfo;
    commandInfo.botMsg = botMsg;
    strcpy(commandInfo.username,chatMsg->username);
    commandInfo.parsedCmd = malloc(sizeof(cmd_t));
    parseCommands(chatMsg->text,commandInfo.parsedCmd);
    for(curCmd = 0;curCmd < NUM_CMD;curCmd++)
    {
        if(!strcmp(commandInfo.parsedCmd->arg,commands[curCmd]))
        {
            cmd[curCmd](&commandInfo);
            return;
        }
    }
    for(curCmd = 0;curCmd < NUM_MOD_CMD;curCmd++)
    {
        if(!strcmp(commandInfo.parsedCmd->arg,secretCommands[curCmd]))
        {
            if(isMod(chatMsg->username))
            {
                modCmd[curCmd](&commandInfo);
            }
            else
            {
                strcpy(botMsg->text,"You aren't a mod! DansGame");
                chat(botMsg);
            }
            return;
        }
    }
    sprintf(botMsg->text,"%s is not a command; type !commands to get a list.",chatMsg->text);//command isn't in any list
    chat(botMsg);
}

void timeout(int seconds, char *username, struct sendMsg *botMsg)
{
    sprintf(botMsg->text,"/timeout %s %i",username,seconds);
    chat(botMsg);
}

//start of regular chat commands
void listCmd(cmdInfo_t *commandInfo)
{
    int i;
    for(i = 0;i < NUM_CMD - 1;i++)
    {
        strcat(commandInfo->botMsg->text,commands[i]);
        strcat(commandInfo->botMsg->text,", ");
    }
    strcat(commandInfo->botMsg->text,commands[i]);
    chat(commandInfo->botMsg);
}

void slots(cmdInfo_t *commandInfo)
{
    if((rand() % force) - 10 > WILL_FORCE_3)//crappy PRNG
    {
        force = rand() % 9;
        sprintf(commandInfo->botMsg->text,"%s | %s | %s",emotes[force],emotes[force],emotes[force]);
        force = 1;
    }
    else
    {
        sprintf(commandInfo->botMsg->text,"%s | %s | %s",emotes[rand() % 9],emotes[rand() % 9],emotes[rand() % 9]);
        force++;
    }
    chat(commandInfo->botMsg);
    if(!strcmp(commandInfo->botMsg->text,"Kappa | Kappa | Kappa"))
    {
        sprintf(commandInfo->botMsg->text,"Goodbye %s Kappa",commandInfo->username);
        chat(commandInfo->botMsg);
        timeout(0,commandInfo->username,commandInfo->botMsg);
    }
    else if(!strcmp(commandInfo->botMsg->text,"TriHard | TriHard | TriHard"))
    {
        int i, j;
        int pos = 8;
        char TH[10] = " TriHard ";
        strcpy(commandInfo->botMsg->text,"TriHard ");
        for(i = 0;i < strlen(commandInfo->username);i++)
        {
            if(commandInfo->username[i] >= 97)//if the chracter in the username isn't capitalized capitalize it
            {
                commandInfo->botMsg->text[pos] = commandInfo->username[i] - 32;
            }
            else//if the character is anything else
            {
                commandInfo->botMsg->text[pos] = commandInfo->username[i];
            }
            pos++;
            for(j = 0;j < 9;j++)
            {
                commandInfo->botMsg->text[pos] = TH[j];
                pos++;
            }
        }
        commandInfo->botMsg->text[pos] = '\0';//terminate string with null character
        strcat(commandInfo->botMsg->text,"TriHard H TriHard Y TriHard P TriHard E TriHard ! TriHard");
        chat(commandInfo->botMsg);
    }
    else if(!strcmp(commandInfo->botMsg->text,"Kreygasm | Kreygasm | Kreygasm"))
    {
        sprintf(commandInfo->botMsg->text,"Kreygasm For later tonight %s https://www.youtube.com/watch?v=P-Vvm7M4Lig Kreygasm",commandInfo->username);
        chat(commandInfo->botMsg);
    }
}

void pasta(cmdInfo_t *commandInfo)
{
    getRandomItem(0,commandInfo->botMsg->text);
    chat(commandInfo->botMsg);
}

void modsPls(cmdInfo_t *commandInfo)
{
    getRandomItem(3,commandInfo->botMsg->text);
    chat(commandInfo->botMsg);
}

void raffle(cmdInfo_t *commandInfo)
{
    if(raffleStatus)
    {
        if(raffleNames == NULL)//if the list of enterants has not been made yet
        {
            raffleNames = malloc(sizeof(re_t));
            strcpy(raffleNames->username,commandInfo->username);
            raffleNames->next = NULL;
        }
        else
        {
            re_t *current;
            for(current = raffleNames;current != NULL;current = current->next)
            {
                if(!strcmp(current->username,commandInfo->username))//check if the user has already entered
                {
                    sprintf(commandInfo->botMsg->text,"%s, you are already in the draw! FailFish",commandInfo->username);
                    chat(commandInfo->botMsg);
                    return;
                }
            }
            current = malloc(sizeof(re_t));
            strcpy(current->username,commandInfo->username);
            current->next = NULL;
        }
        sprintf(commandInfo->botMsg->text,"%s has entered the draw.",commandInfo->username);
        chat(commandInfo->botMsg);
        numEntrants++;
    }
    else
    {
        strcpy(commandInfo->botMsg->text,"There is no draw dumbass FailFish");
        chat(commandInfo->botMsg);
    }
}

void social(cmdInfo_t *commandInfo)
{
    if(socialSetVar)
    {
        char message[512];
        if(socialSetVar & FACEBOOK_SET)
        {
            sprintf(message,"Like %s on facebook at %s ",streamerName,facebook);
            strcat(commandInfo->botMsg->text,message);
        }
        if(socialSetVar & TWITTER_SET)
        {
            sprintf(message,"Follow %s on twitter at %s ",streamerName,twitter);
            strcat(commandInfo->botMsg->text,message);
        }
        if(socialSetVar & YOUTUBE_SET)
        {
            sprintf(message,"Subscribe to %s on youtube at %s ",streamerName,youtube);
            strcat(commandInfo->botMsg->text,message);
        }
        if(socialSetVar & MAL_SET)
        {
            sprintf(message,"Friend %s on MAL at %s ",streamerName,MAL);
            strcat(commandInfo->botMsg->text,message);
        }
    }
    else
    {
        strcpy(commandInfo->botMsg->text,"No social links BibleThump");
    }
    chat(commandInfo->botMsg);
}

void healthy(cmdInfo_t *commandInfo)
{
    getRandomItem(1,commandInfo->botMsg->text);
    chat(commandInfo->botMsg);
}

void quote(cmdInfo_t *commandInfo)
{
    getRandomItem(2,commandInfo->botMsg->text);
    chat(commandInfo->botMsg);
}
//end of regular chat commands

//start of mod chat commands
void listModCmd(cmdInfo_t *commandInfo)
{
    int i;
    for(i = 0;i < NUM_CMD - 1;i++)
    {
        strcat(commandInfo->botMsg->text,secretCommands[i]);
        strcat(commandInfo->botMsg->text,", ");
    }
    strcat(commandInfo->botMsg->text,secretCommands[i]);
    chat(commandInfo->botMsg);
}

void refreshMods(cmdInfo_t *commandInfo)
{
    //unallocate modlist from memory
    getMods(commandInfo->botMsg);
}

void ban(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the ban name! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        strcpy(commandInfo->botMsg->text,"░░░░░░░░░░░░ ▄████▄░░░░░░░░░░░░░░░░░░░░ ██████▄░░░░░░▄▄▄░░░░░░░░░░ ░███▀▀▀▄▄▄▀▀▀░░░░░░░░░░░░░ ░░░▄▀▀▀▄░░░█▀▀▄░▄▀▀▄░█▄░█░ ░░░▄▄████░░█▀▀▄░█▄▄█░█▀▄█░ ░░░░██████░█▄▄▀░█░░█░█░▀█░ ░░░░░▀▀▀▀░░░░░░░░░░░░░░░░░");
        chat(commandInfo->botMsg);
        sprintf(commandInfo->botMsg->text,"/ban %s",commandInfo->parsedCmd->next->arg);
        chat(commandInfo->botMsg);
    }
}

void updatePasta(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the pasta to add! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'w',commandInfo->botMsg);
    }
}

void removePasta(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the pasta to remove! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'d',commandInfo->botMsg);
    }
}

void toggleRaffle(cmdInfo_t *commandInfo)
{
    if(raffleStatus)
    {
        raffleStatus = 0;
        strcpy(commandInfo->botMsg->text,"Raffle closed.");
        chat(commandInfo->botMsg);
    }
    else
    {
        raffleStatus = 1;
        strcpy(commandInfo->botMsg->text,"Raffle open!");
        chat(commandInfo->botMsg);
    }
}

void raffleDraw(cmdInfo_t *commandInfo)
{
    if(!raffleStatus)
    {
        if(numEntrants == 0)
        {
            strcpy(commandInfo->botMsg->text,"No one has entered the draw!");
            chat(commandInfo->botMsg);
        }
        else
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
            sprintf(commandInfo->botMsg->text,"The winner is %s!",current->username);
            chat(commandInfo->botMsg);
            for(;numEntrants != 0;numEntrants--)//remove all entrys from list
            {
                current = raffleNames;//reset list
                while(current->next != NULL) current = current->next;//loop to the end of the list to remove the item
                free(current);
                current = NULL;
            }
            raffleNames = NULL;
        }
    }
    else
    {
        strcpy(commandInfo->botMsg->text,"The raffle is still on! FailFish");
        chat(commandInfo->botMsg);
    }
}

void updateHealthy(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the lewd to add! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'w',commandInfo->botMsg);
    }
}

void removeHealthy(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the lewd to remove! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'d',commandInfo->botMsg);
    }
}

void updateQuote(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the quote to add! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'w',commandInfo->botMsg);
    }
}

void removeQuote(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the quote to remove! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'d',commandInfo->botMsg);
    }
}
