#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "hoarebot.h"

int main(int argc, char *argv[])
{
    struct Channel channelList[1024];
    //TODO read in PID list
    if(argc < 2)
    {
        printf("No channel name\n");
    }
    else if(argc > 2)
    {
        printf("Too many arguments\n");
    }
    else
    {
        strcpy(channelList[0].name,argv[1]);
        joinChannel(channelList[0]);
    }
    return 0;
}

void joinChannel(struct Channel chnl)
{
    switch(chnl.PID = fork())
    {
        case -1:
            printf("Forking failed.\n");
            break;
        case 0:
            //TODO put IRC joinging stuff here I think
	        printf("PID: %i\n",getpid());
	        break;
	    default:
	        printf("Process %i spawned for channel %s\n",chnl.PID,chnl.name);
	        break;
    }
}
