#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "hoarebot.h"

int main(int argc, char *argv[])
{
    struct Channel channelList[1024];
    //TODO read in PID list
    switch(daemon(0,0))
    {
        case -1:
            printf("Daemonizing has failed\n");
        case 0:
            if(argc < 2)
            {
                //this should list currect channels with PIDs
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
                strcpy(channelList[0].name,argv[1]);
                //TODO write new bot PID with channel to file
                //joinChannel(channelList[0]);
            }
        default:
            printf("You shouldn't have gotten here...\n");
    }
    while(1)
    {
        //TODO make bot
    }
    return 0;
}

int joinChannel(struct Channel chnl)
{
    int irc;
    irc = socket(AF_UNIX,SOCK_STREAM,0);
    if(irc == -1)
    {
        return -1;
    }
    return 0;
}
