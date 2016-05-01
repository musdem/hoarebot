#include "hoarebot.h"

int main(int argc, char *argv[])
{
    int size, irc;
    irc = 0;
    char raw[BUFSIZ], botPass[37], channel[128];
    struct Channel channelList[1024];//maybe use malloc to assign size based on what is read in from what bot instances are running
    struct getMsg chatMsg;
    struct sendInfo msgInfo;
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
                msgInfo.irc = irc;
                strcpy(msgInfo.channel,channel);
            }
            break;
        default:
            printf("You shouldn't have gotten here...\n");
    }
    while(1)
    {
        size = read(irc,raw,BUFSIZ);
        printf("Raw message: %s\n",raw);
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
            printf("Username: %s\nMessage: %s\n",chatMsg.username,chatMsg.text);
            if(chatMsg.text[0] == '!')
            {
                //command(chatMsg);
                chat(&msgInfo, "Command issued");
            }
        }
    }
    return 0;
}

int parseRaw(char *raw, struct getMsg *chatMsg)
{
    if(strstr(raw,"PRIVMSG") && !strstr(raw,"USERSTATE"))
    {
        int rawPos, textOffset;
        for(rawPos = 1;raw[rawPos] != '!';rawPos++)
        {
            chatMsg->username[rawPos - 1] = raw[rawPos];
        }
        chatMsg->username[rawPos - 1] = '\0';
        while(raw[rawPos] != ':') rawPos++;
        textOffset = rawPos + 1;
        for(rawPos += 1;raw[rawPos] != '\r';rawPos++)
        {
            chatMsg->text[rawPos - textOffset] = raw[rawPos];
        }
        chatMsg->text[rawPos - textOffset] = '\0';
        return rawPos - textOffset - 1;//size of chatMsg.text
    }
    else return 0;
}

int twitchChatConnect()
{
    int irc;
    struct addrinfo hints, *result, *ircAddr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_NUMERICSERV;
    if(getaddrinfo(SERVER,PORT,&hints,&result) != 0)
    {
        printf("Couldn't get address info error: %s\n",gai_strerror(errno));
        return -1;
    }
    for(ircAddr = result;ircAddr != NULL;ircAddr = ircAddr->ai_next)
    {
        irc = socket(ircAddr->ai_family, ircAddr->ai_socktype, ircAddr->ai_protocol);
        if(irc == -1) continue;//try next address on error
        if (connect(irc, ircAddr->ai_addr, ircAddr->ai_addrlen) != -1) break;//if it can connect it will leave loop
        close(irc);
    }
    if(ircAddr == NULL)
    {
        printf("Couldn't connect to twitch chat through any socket.\n");
        return -1;
    }
    freeaddrinfo(result);
    return irc;
}

int joinChannel(int irc, char *channel, char *botPass)
{
    int size;
    char buff[BUFSIZ];
    size = sprintf(buff, "PASS %s\r\n", botPass);
    if(write(irc,buff,size) == -1)
    {
        printf("couldn't write PASS to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff, "USER %s\r\n", BOTNICK);
    if(write(irc,buff,size) == -1)
    {
        printf("couldn't write USER to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff, "NICK %s\r\n", BOTNICK);
    if(write(irc,buff,size) == -1)
    {
        printf("couldn't write NICK to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff, "JOIN %s\r\n", channel);
    if(write(irc,buff,size) == -1)
    {
        printf("couldn't write JOIN to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff, "CAP REQ :twitch.tv/commands\r\n");
    if(write(irc,buff,size) == -1)
    {
        printf("couldn't write CAP REQ to server: %i\n",errno);
        return -1;
    }
    printf("succsessful!\n");
    return 0;
}

int chat(struct sendInfo *msgInfo, char *message)
{
    int size;
    char buff[BUFSIZ];
    size = sprintf(buff, "PRIVMSG %s :%s\r\n",msgInfo->channel,message);
    if(write(msgInfo->irc,buff,size) == -1)
    {
        printf("Couldn't write chat message to server: %i\n",errno);
        return -1;
    }
    return 0;
}
