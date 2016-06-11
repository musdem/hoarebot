#include "hoarebot.h"

//bot command variables
char commands[NUM_CMD][32] = {"!commands","!slots","!pasta","!modspls","!raffle","!social","!healthy","!quote"};
char secretCommands[NUM_MOD_CMD][32] = {"!modcommands","!refreshmods","!ban","!updatepasta","!removepasta","!toggleraffle","!raffledraw","!updatehealthy","!removehealthy","!updatequote","!removequote"};
//slots variables
int force = 1;
char emotes[9][16] = {"Kappa","KappaPride","EleGiggle","BibleThump","PogChamp","TriHard","CoolCat","WutFace","Kreygasm"};
//list variables
list_t *lists[NUM_LISTS];
int numEntry[NUM_LISTS];
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
    int size, irc;
    irc = 0;
    char raw[BUFSIZ], botPass[37], channel[128];
    //chnlL_t channelList;
    struct getMsg chatMsg;
    struct sendMsg botMsg;
    FILE* passFile;
    passFile = fopen("pass","r");
    if(!passFile)
    {
        printf("IRC oauth file not found.\n");
        return -1;
    }
    fgets(botPass,37,passFile);
    fclose(passFile);
    //runningBots(&channelList);
    srand(time(NULL));//seed rng with current time
    //TODO read in PID list
    if(argc < 2)
    {
        //TODO list currect channels with PIDs
        printf("No channel name\n");
        return 0;
    }
    else if(argc > 2)
    {
        printf("Too many arguments\n");
        return 0;
    }
    else
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
                break;
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
    while(1)
    {
        size = read(irc,raw,BUFSIZ);
        if(strstr(raw,"PING :"))
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
    DIR* hoarebotPidDir;
    struct dirent *currentItem;
    hoarebotPidDir = opendir(PID_DIR);
    if(hoarebotPidDir)
    {
        for(currentItem = readdir(hoarebotPidDir);currentItem;currentItem = readdir(hoarebotPidDir))
        {
            //TODO read pids into linked list
            numRunning++;
        }
        closedir(hoarebotPidDir);
        return numRunning;
    }
    else//make hoarebot pid directory and rerun function
    {
        mkdir(PID_DIR,0700);
        return runningBots(CL);
    }
}

void populateLists()
{
    int curList = 0;
    char line[BUFSIZ], newItem;
    struct dirent *currentItem;
    list_t *list;
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
                list = lists[curList];
                while(fgets(line,BUFSIZ,listFile))//read until the end of the file these should have no empty lines
                {
                    if(newItem)
                    {
                        strcpy(list->type,currentItem->d_name);
                        newItem = 0;
                    }
                    else
                    {
                        strcpy(list->item,currentItem->d_name);
                        numEntry[curList]++;
                    }
                    list->next = malloc(sizeof(list_t));
                    list->next->next = NULL;
                    list = list->next;
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

void getMods()
{
    //TODO populate mod list use twitch api I guess
}

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
        //TODO make list function to pick random item from list
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[3]))//!modsPls
    {
        //TODO make list function to pick random item from list
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
        //TODO make list function to pick random item from list
        chat(botMsg);
    }
    else if(!strcmp(chatMsg->text,commands[7]))//!quote
    {
        //TODO make list function to pick random item from list
        chat(botMsg);
    }
    else
    {
        sprintf(botMsg->text,"%s is not a command; type !commands to get a list.",chatMsg->text);
        chat(botMsg);
    }
    botMsg->text[0] = '\0';//nullify the string for next message
}

void timeout(int seconds, char *username, struct sendMsg *botMsg)
{
    sprintf(botMsg->text,"/timeout %s %i",username,seconds);
    chat(botMsg);
}

void slots(char *username, struct sendMsg *botMsg)
{
    if((rand() % force) - 10 > WILL_FORCE_3)
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
            else
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
