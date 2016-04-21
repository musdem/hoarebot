#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "daemonize.h"
#include "hoarebot.h"

int main(int argc, char *argv[])
{
    struct Channel channelList[1024];
    //TODO read in PID list
    switch(daemonize(0))
    {
        case -1:
            printf("Daemonizing has failed\n");
        case 0:
            if(argc < 2)
            {
                //this should list currect channels with PIDs
                printf("No channel name\n");
            }
            else if(argc > 2)
            {
                printf("Too many arguments\n");
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

int daemonize(int flags)
{
    int fd, maxfd;
    switch(fork())//become background process
    {
        case -1:
            return -1;
        case 0:
	        break;
	    default:
	        exit(0);
    }
    if (setsid() == -1)//become leader of new session
    {
        return -1;
    }
    switch(fork())//ensure we are not session leader
    {
        case -1:
            return -1;
        case 0:
            break;
        default:
            exit(0);
    }
    if(!(flags & BD_NO_UMASK0))//clear file mode creation mask
    {
        umask(0);
    }
    if(!(flags & BD_NO_CHDIR))//change to root directory
    {
        chdir("/");//TODO figure out a good place to put all the json files and put the directory here shoudln't be a directory that isn't mounted as root
    }
    if(!(flags & BD_NO_CLOSE_FILES))//close all open files
    {
        maxfd = sysconf(_SC_OPEN_MAX);
        if(maxfd == -1)
        {
            maxfd = BD_MAX_CLOSE;
        }
        for(fd = 0;fd < maxfd;fd++)
        {
            close(fd);
        }
    }
    if(!(flags & BD_NO_REOPEN_STD_FDS))// reopen standard fd's to /dev/null
    {
        close(STDIN_FILENO);
        fd = open("/dev/null",O_RDWR);
        if(fd != STDIN_FILENO)//fd should be 0
        {
            return -1;
        }
        if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        {
            return -1;
        }
        if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        {
            return -1;
        }
    }
    return 0;
}
