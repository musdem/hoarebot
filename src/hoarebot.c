#include "hoarebot.h"

char commands[NUM_CMD][128] = {"!commands","!slots","!pasta","!modspls","!raffle","!social","!healthy","!quote","!braveid"};
char secretCommands[NUM_MOD_CMD][128] = {"!modcommands","!refreshmods","!ban","!updatepasta","!toggleraffle","!raffledraw","!updatehealthy","!removepasta","!removehealthy","!updatequote","!removequote"};

int main(int argc, char *argv[])
{
    int size, irc;
    irc = 0;
    char raw[BUFSIZ], botPass[37], channel[128];
    struct Channel channelList[1024];//maybe use malloc to assign size based on what is read in from what bot instances are running
    struct getMsg chatMsg;
    struct sendMsg botMsg;
    FILE* passFile;
    passFile = fopen("pass","r");
    fgets(botPass,37,passFile);
    fclose(passFile);
    //TODO read in PID list
    switch(daemon(0,1))//this is 0,1 for testing should let this be set at runtime
    {
        case -1:
            printf("Daemonizing has failed\n");
            break;
        case 0:
            if(argc < 2)
            {
                //TODO list currect channels with PIDs
                printf("No channel name\n");
                exit(0);
            }
            else if(argc > 2)
            {
                printf("Too many arguments\n");
                exit(0);
            }
            else
            {
                strcpy(channel,argv[1]);
                strcpy(channelList[0].name,argv[1]);
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
            }
            break;
        default:
            printf("You shouldn't have gotten here...\n");
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

int command(struct getMsg *chatMsg, struct sendMsg *botMsg)
{
    if(strcmp(chatMsg->text,commands[0]) == 0)
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
    else
    {
        sprintf(botMsg->text,"%s is not a command; type !commands to get a list.",chatMsg->text);
        chat(botMsg);
    }
    botMsg->text[0] = '\0';//nullify the string for next message
    return 0;
}
