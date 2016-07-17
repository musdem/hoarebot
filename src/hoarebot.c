#include "hoarebot.h"

//bot command variables
char commands[NUM_CMD][32] = {"!commands","!slots","!pasta","!modspls","!raffle","!social","!healthy","!quote"};
char secretCommands[NUM_MOD_CMD][32] = {"!modcommands","!refreshmods","!ban","!updatepasta","!removepasta","!toggleraffle","!raffledraw","!updatehealthy","!removehealthy","!updatequote","!removequote"};
//slots variables
int force = 1;
char emotes[9][16] = {"Kappa","KappaPride","EleGiggle","BibleThump","PogChamp","TriHard","CoolCat","WutFace","Kreygasm"};
//raffle variables
int numEntrants = 0;
re_t raffleNames;
//social variables
char socialSetVar = 0;
char *streamerName;
char *facebook;
char *twitter;
char *youtube;
char *MAL;

int main(int argc, char *argv[])
{
    int size, irc, numRunning;
    irc = 0;
    char raw[BUFSIZ], botPass[37], channel[128], running;
    chnlL_t channelList, *current;
    struct getMsg chatMsg;
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
            while(current->next != NULL)
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
        /*if(!mkdir(channel,0700))//make directory for bot using channel and make it currect working directory
        {
            chdir(channel);
        }
        else
        {
            printf("Could not create %s directory\nIs there a bot already running on that channel?\n",channel);
            return -1;
        }*/
        switch(daemon(0,0))
        {
            case -1:
                printf("Daemonizing has failed\n");
                return -1;
            case 0:
                strcpy(channel,argv[1]);
                //TODO write new bot PID with channel to file
                irc = twitchChatConnect();//the irc socket file descriptor
                if(irc == -1)
                {
                    printf("Couldn't connect to twitch servers.\n");
                }
                if(joinChannel(irc, channel, botPass) == -1)
                {
                    printf("Couldn't join the channel\n");
                }
                botMsg.irc = irc;
                strcpy(botMsg.channel,channel);
                break;
            default:
                printf("You shouldn't have gotten here...\n");
                return -1;
        }
    }
    running = 1;
    while(running)
    {
        size = read(irc,raw,BUFSIZ);
        if(strstr(raw,"PING :"))//make sure the bot stays connected
        {
            size = sprintf(raw,"PONG :tmi.twitch.tv\r\n");
            if(write(irc,raw,size) == -1)
            {
                printf("couldn't write PING back server: %i\n",errno);
                return -1;
            }
        }
        else if(parseRaw(raw,&chatMsg))
        {
            if(chatMsg.text[0] == '!')
            {
                command(&chatMsg, &botMsg);
            }
        }
    }
    return 0;
}

int runningBots(chnlL_t *CL)
{
    int numRunning = 0;
    char pidStr[64];
    chnlL_t *current;
    DIR *hoarebotPidDir;
    FILE *pidFile;
    struct dirent *currentItem;
    hoarebotPidDir = opendir(PID_DIR);
    if(hoarebotPidDir)//dir exists so read in running channels
    {
        chdir(PID_DIR);
        current = CL;
        for(currentItem = readdir(hoarebotPidDir);currentItem;currentItem = readdir(hoarebotPidDir))//go over every item in run dir
        {
            if(strcmp("..",currentItem->d_name) != 0 && strcmp(".",currentItem->d_name) != 0)//make sure that dir up and cur dir aren't considered a pid
            {
                pidFile = fopen(currentItem->d_name,"r");
                fgets(pidStr,64,pidFile);
                current->PID = (pid_t) atoi(pidStr);
                strcpy(current->name,currentItem->d_name);
                current->next = malloc(sizeof(chnlL_t));
                current->next->next = NULL;
                current = current->next;
                numRunning++;
            }
        }
        closedir(hoarebotPidDir);
        if(current->next != NULL)//make sure this doesn't try to free something that isn't there
        {
            free(current);
            current = NULL;
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
    //TODO populate mod list using /mods and lists.c
    //get raw and jump to mod lists then add mods to lists using lists.c and never save
    //the mod list will always be the last item in the list_t array (NUM_LISTS - 1)
}

//maybe I should make a command type with the command and the rest of the text
void command(struct getMsg *chatMsg, struct sendMsg *botMsg)
{
    if(!strcmp(chatMsg->text,commands[0]))
    {
        int i;
        for(i = 0;i < NUM_CMD - 1;i++)//!commands
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
        getRandomItem(0);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[3]))//!modsPls
    {
        getRandomItem(3);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[4]))//!raffle
    {
        raffle(chatMsg->username, botMsg);
    }
    else if(!strcmp(chatMsg->text, commands[5]))//!social
    {
        social(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[6]))//!healthy
    {
        getRandomItem(1);
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[7]))//!quote
    {
        getRandomItem(2);
        chat(botMsg);
    }
    else
    {
        sprintf(botMsg->text,"%s is not a command; type !commands to get a list.",chatMsg->text);
        chat(botMsg);
    }
    botMsg->text[0] = '\0';//nullify the string for next message
}

int argPos(char *cmd, int argNum)
{
    int cmdArgPos = 0;
    while(argNum != 0)
    {
        cmdArgPos++;
        if(cmd[cmdArgPos] == ' ') argNum--;//if an argument is found decrement
        else if(cmd[cmdArgPos] == '\0') return -1;//if the end of cmd is reached and there wasn't the correct number of requested arguments
    }
    return cmdArgPos + 1;//the start of the selected command argument
}

void timeout(int seconds, char *username, struct sendMsg *botMsg)
{
    sprintf(botMsg->text,"/timeout %s %i",username,seconds);
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
    //TODO check if user has entered already
    sprintf(botMsg->text,"%s has entered the draw.",username);
    chat(botMsg);
    //TODO add item to raffleNames
    numEntrants++;
}

void social(struct sendMsg *botMsg)
{
    char message[256];
    if(socialSetVar & FACEBOOK_SET)
    {
        sprintf(message,"Like %s on facebook at %s ",streamerName,facebook);
    }
    strcat(botMsg->text,message);
    if(socialSetVar & TWITTER_SET)
    {
        sprintf(message,"Follow %s on twitter at %s ",streamerName,twitter);
    }
    strcat(botMsg->text,message);
    if(socialSetVar & YOUTUBE_SET)
    {
        sprintf(message,"Subscribe to %s on youtube at %s ",streamerName,youtube);
    }
    strcat(botMsg->text,message);
    if(socialSetVar & MAL_SET)
    {
        sprintf(message,"Friend %s on MAL at %s ",streamerName,MAL);
    }
    strcat(botMsg->text,message);
    chat(botMsg);
}
