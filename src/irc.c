#include "irc.h"

size_t parseRaw(char *raw, struct getMsg *chatMsg)
{
    if(strstr(raw,"PRIVMSG") && !strstr(raw,"USERSTATE"))
    {
        int rawPos, textOffset;
        for(rawPos = 1;raw[rawPos] != '!';rawPos++)//write the username
        {
            chatMsg->username[rawPos - 1] = raw[rawPos];
        }
        chatMsg->username[rawPos - 1] = '\0';
        while(raw[rawPos] != ':') rawPos++;//put rawPos at actual message
        textOffset = rawPos + 1;
        for(rawPos += 1;raw[rawPos] != '\r';rawPos++)//write the actual message
        {
            chatMsg->text[rawPos - textOffset] = raw[rawPos];
        }
        chatMsg->text[rawPos - textOffset] = '\0';
        return (size_t) rawPos - textOffset - 1;//size of chatMsg.text
    }
    else return (size_t) 0;
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

int joinChannel(struct sendMsg *botMsg, char *botPass)//standard way to join IRC server
{
    int size;
    char buff[BUFSIZ];
    size = sprintf(buff,"PASS %s\r\n", botPass);
    if(write(botMsg->irc,buff,size) == -1)
    {
        printf("couldn't write PASS to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff,"USER %s\r\n", BOTNICK);
    if(write(botMsg->irc,buff,size) == -1)
    {
        printf("couldn't write USER to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff,"NICK %s\r\n", BOTNICK);
    if(write(botMsg->irc,buff,size) == -1)
    {
        printf("couldn't write NICK to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff,"JOIN %s\r\n", botMsg->channel);
    if(write(botMsg->irc,buff,size) == -1)
    {
        printf("couldn't write JOIN to server: %i\n",errno);
        return -1;
    }
    size = sprintf(buff,"CAP REQ :twitch.tv/commands\r\n");
    if(write(botMsg->irc,buff,size) == -1)
    {
        printf("couldn't write CAP REQ to server: %i\n",errno);
        return -1;
    }
    return 0;
}

int chat(struct sendMsg *botMsg)//standard way to send a message to IRC server
{
    int size;
    char buff[BUFSIZ];
    size = sprintf(buff,"PRIVMSG %s :%s\r\n",botMsg->channel,botMsg->text);
    if(write(botMsg->irc,buff,size) == -1)
    {
        printf("Couldn't write chat message to server: %i\n",errno);
        return -1;
    }
    return 0;
}
