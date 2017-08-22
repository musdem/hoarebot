#include "commands.h"

//bot command variables
char commands[NUM_CMD][32] = {"!commands","!slots","!pasta","!modspls","!raffle","!social","!healthy","!quote"};
char secretCommands[NUM_MOD_CMD][32] = {"!modcommands","!refreshmods","!kill","!ban","!updatepasta","!removepasta","!toggleraffle","!raffledraw","!updatehealthy","!removehealthy","!updatequote","!removequote"};
void (*cmd[NUM_CMD])(cmdInfo_t*) = {&listCmd,&slots,&pasta,&modsPls,&raffle,&social,&healthy,&quote};
void (*modCmd[NUM_MOD_CMD])(cmdInfo_t*) = {&listModCmd,&refreshMods,&kill,&ban,&updatePasta,&removePasta,&toggleRaffle,&raffleDraw,&updateHealthy,&removeHealthy,&updateQuote,&removeQuote};
//slots variables
int force = 1;
char emotes[9][16] = {"Kappa","KappaPride","EleGiggle","BibleThump","PogChamp","TriHard","CoolCat","WutFace","Kreygasm"};
//raffle variables
int raffleStatus = 0;
int numEntrants = 0;
re_t *raffleNames = NULL;
//list of mods
ml_t *mods = NULL;
//social variables
char socialSetVar = 0;
char streamerName[256];
char facebook[256];
char twitter[256];
char youtube[256];
char MAL[256];

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

void freeMods()
{
    ml_t *tmp;
    while(mods != NULL)
    {
        tmp = mods;
        mods = mods->next;
        free(tmp);
    }
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
    current->next = NULL;//make sure that next is nullified on one argument commands
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
            while(rawCmd[cmdPos + 1] == ' ') cmdPos++;//ensure that rawCmd is at the last space before a new arg
            current->next = malloc(sizeof(cmd_t));
            current = current->next;
            current->next = NULL;
        }
    }
    current->arg[cmdOffset] = '\0';
}

void freeCommands(cmd_t *parsedCmd)
{
    cmd_t *tmp;
    while(parsedCmd != NULL)
    {
        tmp = parsedCmd;
        parsedCmd = parsedCmd->next;
        free(tmp);
    }
}

void command(struct getMsg *chatMsg, struct sendMsg *botMsg)
{
    int curCmd;
    cmdInfo_t commandInfo;
    commandInfo.botMsg = botMsg;
    commandInfo.parsedCmd = malloc(sizeof(cmd_t));
    strcpy(commandInfo.username,chatMsg->username);
    parseCommands(chatMsg->text,commandInfo.parsedCmd);
    for(curCmd = 0;curCmd < NUM_CMD;curCmd++)
    {
        if(!strcmp(commandInfo.parsedCmd->arg,commands[curCmd]))
        {
            cmd[curCmd](&commandInfo);
            freeCommands(commandInfo.parsedCmd);
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
            freeCommands(commandInfo.parsedCmd);
            return;
        }
    }
    sprintf(botMsg->text,"%s is not a command; type !commands to get a list.",chatMsg->text);//command isn't in any list
    chat(botMsg);
    freeCommands(commandInfo.parsedCmd);
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
    for(i = 0;i < NUM_CMD - 1;i++)//one less than NUM_CMD in order to format nicely
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
            if(commandInfo->username[i] >= 97)//if the character in the username isn't capitalized capitalize it
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
    for(i = 0;i < NUM_MOD_CMD - 1;i++)//one less than NUM_MOD_CMD in order to format nicely
    {
        strcat(commandInfo->botMsg->text,secretCommands[i]);
        strcat(commandInfo->botMsg->text,", ");
    }
    strcat(commandInfo->botMsg->text,secretCommands[i]);
    chat(commandInfo->botMsg);
}

void refreshMods(cmdInfo_t *commandInfo)
{
    freeMods();
    getMods(commandInfo->botMsg);
    strcpy(commandInfo->botMsg->text,"Mods have been refreshed");
    chat(commandInfo->botMsg);
}

void kill(cmdInfo_t *commandInfo)
{
    sem_t *stopBot;
    commandInfo->botMsg->channel[0] = '/';
    stopBot = sem_open(commandInfo->botMsg->channel,0);
    if(stopBot == NULL)
    {
        strcpy(commandInfo->botMsg->text,"Couldn't create semaphore");
        chat(commandInfo->botMsg);
        return;
    }
    sem_post(stopBot);
    commandInfo->botMsg->channel[0] = '#';
    strcpy(commandInfo->botMsg->text,"Killing self on next message");
    chat(commandInfo->botMsg);
    sem_close(stopBot);
}

void ban(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next)
    {
        strcpy(commandInfo->botMsg->text,"░░░░░░░░░░░░ ▄████▄░░░░░░░░░░░░░░░░░░░░ ██████▄░░░░░░▄▄▄░░░░░░░░░░ ░███▀▀▀▄▄▄▀▀▀░░░░░░░░░░░░░ ░░░▄▀▀▀▄░░░█▀▀▄░▄▀▀▄░█▄░█░ ░░░▄▄████░░█▀▀▄░█▄▄█░█▀▄█░ ░░░░██████░█▄▄▀░█░░█░█░▀█░ ░░░░░▀▀▀▀░░░░░░░░░░░░░░░░░");
        chat(commandInfo->botMsg);
        sprintf(commandInfo->botMsg->text,"/ban %s",commandInfo->parsedCmd->next->arg);
        chat(commandInfo->botMsg);
    }
    else
    {
        strcpy(commandInfo->botMsg->text,"You forgot the ban name! FailFish");
        chat(commandInfo->botMsg);
    }
}

void updatePasta(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next == NULL)
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
    if(commandInfo->parsedCmd->next == NULL)
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
            while(raffleNames != NULL)//remove all entrys from list
            {
                current = raffleNames;//reset list
                raffleNames = raffleNames->next;
                free(current);
            }
            numEntrants = 0;
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
    if(commandInfo->parsedCmd->next == NULL)
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
    if(commandInfo->parsedCmd->next == NULL)
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
    if(commandInfo->parsedCmd->next == NULL)
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
    if(commandInfo->parsedCmd->next == NULL)
    {
        strcpy(commandInfo->botMsg->text,"You forgot the quote to remove! FailFish");
        chat(commandInfo->botMsg);
    }
    else
    {
        updateList(commandInfo->parsedCmd->next->arg,0,'d',commandInfo->botMsg);
    }
}

void updateSocial(cmdInfo_t *commandInfo)
{
    if(commandInfo->parsedCmd->next == NULL)
    {
        strcpy(commandInfo->botMsg->text,"You need to format the command !updateSocial {S|F|T|Y|M} name/link");
        chat(commandInfo->botMsg);
    }
    else
    {
    	int infoLoc = 0;
    	char info[128];
    	FILE *socialFile;
    	cmd_t *curArg;
    	curArg = commandInfo->parsedCmd->next;
    	if(curArg->next == NULL)
    	{
    	    strcpy(commandInfo->botMsg->text,"You need to format the command !updateSocial {S|F|T|Y|M} name/link");
    	    chat(commandInfo->botMsg);
    	}
    	else
    	{
    	    info[infoLoc] = curArg->arg[0];
    	    infoLoc++;
    	    while(curArg->next->arg[infoLoc - 1] != '\0')
    	    {
                info[infoLoc] = curArg->next->arg[infoLoc - 1];
                infoLoc++;
    	    }
    	    info[infoLoc] = '\0';
    	    socialFile = fopen("socialInfo","a");
    	    fputs(info,socialFile);
            fclose(socialFile);
        }
    }
}
//end of mod chat commands
