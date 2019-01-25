#include "hoarebot.h"

int main(int argc, char *argv[])
{
    int numRunning, opt, verbose, kill, debug;
    char botPass[37];
    chnlL_t channelList, *current;
    struct sendMsg botMsg;
    FILE *passFile;
    verbose = 0;
    kill = 0;
    debug = 0;
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
    while((opt = getopt(argc,argv,"kvd")) != -1)
    {
        switch(opt)
        {
            case 'v':
                verbose = 1;
                break;
            case 'k':
                kill = 1;
                break;
            case 'd':
                debug = 1;
                break;
            default:
                printf("Command usage: hoarebot [-k kill selected bot | -v enable verbose mode | -d enable debug mode] [channel]\n");
                return -1;
        }
    }
    if(argc == 1)//no arguments were added to the command so list running bots
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
    else if(kill)
    {
        if(numRunning)
        {
            strcpy(botMsg.channel,argv[optind]);
            if(!strcmp("all",botMsg.channel))
            {
                printf("Killing all bots\n");
                current = &channelList;
                while(current != NULL)
                {
                    returnPound(current->name,botMsg.channel);
                    killBot(botMsg.channel);
                    current = current->next;
                }
            }
            else
            {
                if(isRunning(botMsg.channel,&channelList))
                {
                    killBot(botMsg.channel);
                }
                else printf("%s isn't running on this machine.\n",botMsg.channel);
            }
        }
        else
        {
            printf("There are no bots running.\n");
        }
        return 0;
    }
    else//argument provided this should be a channel to join
    {
        if(numRunning && isRunning(argv[optind],&channelList))
        {
            printf("%s is already running on this machine!\n",argv[optind]);
            return 0;
        }
        strcpy(botMsg.channel,argv[optind]);
        if(debug)
        {
            printf("running in debug mode, this will disable daemonizing\nrun this in gdb to access debug info\n");
        }
        else
        {
            switch(daemon(1,verbose))
            {
                case -1:
                    printf("Daemonizing has failed\n");
                    return -1;
                case 0:
                    break;
                default:
                    printf("You shouldn't have gotten here...\n");
                    return -1;
            }
        }
    }
    if(initialize(botPass, &botMsg) != 0)
    {
        printf("failed to initialize the bot\n");
        syslog(LOG_ERR, "failed to intialize the bot");
        return -1;
    }
    if(run(&botMsg) != 0)
    {
        printf("bot run error\n");
        syslog(LOG_ERR, "bot run error");
        syslog(LOG_ERR, "attempting to clean up...");
    }
    if(cleanup(&botMsg) != 0)
    {
        printf("failed to clean up bot\nyou need to manually clean up\n");
        syslog(LOG_ERR, "failed to clean up bot");
        syslog(LOG_ERR, "you need to manually clean up");
        return -1;
    }
    return 0;
}
