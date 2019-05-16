#include "hoarebot.h"

int running = 1;
char ident[256];

void removePound(char *channel, char *fixedChannel)
{
	int i;
	for(i = 1;channel[i] != '\0';i++)//remove the pound symbol from the channel
	{
		fixedChannel[i-1] = channel[i];
	}
	fixedChannel[i-1] = '\0';
}

void returnPound(char *channel, char *fixedChannel)
{
	int i;
	fixedChannel[0] = '#';
	for(i = 0;channel[i] != '\0';i++)//return the pound symbol from the channel
	{
		fixedChannel[i+1] = channel[i];
	}
	fixedChannel[i+1] = '\0';
}

void createPID(struct sendMsg *botMsg)
{
	char fileData[64];
	removePound(botMsg->channel,fileData);
	//create PID file for new bot
	chdir(PID_DIR);
	FILE *newPID;
	newPID = fopen(fileData,"a");
	sprintf(fileData,"%i",(int) getpid());
	if(fputs(fileData,newPID) == -1)
	{
		printf("Failed to make a PID file.\n");
	}
	fclose(newPID);
	chdir("..");//leave PID_DIR
}

int runningBots(chnlL_t *CL)
{
	int numRunning = 0;
	char pidStr[64];
	chnlL_t *current, *previous;
	DIR *hoarebotPidDir;
	FILE *pidFile;
	struct dirent *currentItem;
	hoarebotPidDir = opendir(PID_DIR);
	if(hoarebotPidDir)//dir exists so read in running channels
	{
		chdir(PID_DIR);
		previous = NULL;
		current = CL;
		for(currentItem = readdir(hoarebotPidDir);currentItem;currentItem = readdir(hoarebotPidDir))//go over every item in run dir
		{
			if(strcmp("..",currentItem->d_name) != 0 && strcmp(".",currentItem->d_name) != 0)//make sure that dir up and cur dir aren't considered a pid
			{
				pidFile = fopen(currentItem->d_name,"r");
				fgets(pidStr,64,pidFile);
				fclose(pidFile);
				current->PID = (pid_t) atoi(pidStr);
				strcpy(current->name,currentItem->d_name);
				current->next = malloc(sizeof(chnlL_t));
				current->next->next = NULL;
				previous = current;
				current = current->next;
				numRunning++;
			}
		}
		closedir(hoarebotPidDir);
		if(previous == NULL)//if there are no bots running set the head to null
		{
			CL = NULL;
		}
		else//free the last node
		{
			free(previous->next);
			previous->next = NULL;
		}
		chdir("..");
		return numRunning;
	}
	else//make hoarebot pid directory and rerun function
	{
		mkdir(PID_DIR,0755);
		return runningBots(CL);
	}
}

int isRunning(char *channel, chnlL_t *running)
{
	char check[64];
	chnlL_t *current;
	removePound(channel,check);
	current = running;
	while(current != NULL)
	{
		if(!strcmp(current->name,check)) return 1;
		current = current->next;
	}
	return 0;
}

static void *checkRunning(void *channel)
{
	char *c = (char*) channel;
	sem_t *stopBot;
	c[0] = '/';
	sem_unlink(c);
	stopBot = sem_open(c,(O_CREAT | O_EXCL),0600,0);
	if(stopBot == NULL)
	{
		printf("couldn't create semaphore: %i\n",errno);
		running = 0;
		pthread_detach(pthread_self());
	}
	c[0] = '#';//put pound sign back to let bot connect to IRC
	sem_wait(stopBot);
	sem_close(stopBot);
	running = 0;
	pthread_detach(pthread_self());
	return NULL;
}

void killBot(char *channel)
{
	sem_t *stopBot;
	channel[0] = '/';
	stopBot = sem_open(channel,0);
	if(stopBot == NULL)
	{
		printf("couldn't create semaphore: %i\n",errno);
	}
	sem_post(stopBot);
	channel[0] = ' ';
	printf("killing%s\n",channel);
	sem_close(stopBot);
}

int initialize(char *botPass, struct sendMsg *botMsg)
{
	char botDir[64];
	pthread_t runThread;
	sprintf(ident, "[channel %s]", botMsg->channel);
	openlog(ident, LOG_NDELAY, LOG_DAEMON);
	if(pthread_create(&runThread,NULL,checkRunning,botMsg->channel) != 0)
	{
		printf("run thread failed: %i\n",errno);
		return -1;
	}
	syslog(LOG_INFO, "created exit check thread");
	botMsg->irc = twitchChatConnect();//the irc socket file descriptor
	if(botMsg->irc == -1)
	{
		printf("Couldn't connect to twitch servers.\n");
		return -1;
	}
	syslog(LOG_INFO, "connected to twitch IRC server");
	if(joinChannel(botMsg, botPass) == -1)
	{
		printf("Couldn't join the channel\n");
		return -1;
	}
	syslog(LOG_INFO, "joined channel %s", botMsg->channel);
	createPID(botMsg);
	removePound(botMsg->channel,botDir);
	if(chdir(botDir) != 0)
	{
		if(mkdir(botDir,0755) != 0)
		{
			printf("error making bot directory: %i\n",errno);
			return -1;
		}
		if(chdir(botDir) != 0)
		{
			printf("error changing to bot directory (%s): %i\n",botDir,errno);
			return -1;
		}
	}
	getMods(botMsg);
	syslog(LOG_INFO, "added mods to mod list");
	getSocial();
	syslog(LOG_INFO, "generated social message");
	populateLists();
	syslog(LOG_INFO, "generated lists");
	return 0;
}

int run(struct sendMsg *botMsg)
{
	int size, readStatus;
	char raw[BUFSIZ];
	struct getMsg chatMsg;
	while(running)
	{
		readStatus = read(botMsg->irc,raw,BUFSIZ);
		if(readStatus == -1)
		{
			printf("couldn't read from the server: %i\n",errno);
			syslog(LOG_ERR, "couldn't read from the server");
			return -1;
		}
		else if(readStatus == 0)
		{
			syslog(LOG_ERR, "read returned a 0");
			return -1;
		}
		else
		{
			if(strstr(raw,"PING :"))//make sure the bot stays connected
			{
				size = sprintf(raw,"PONG :tmi.twitch.tv\r\n");
				if(write(botMsg->irc,raw,size) == -1)
				{
					printf("couldn't write PING back server: %i\n",errno);
					syslog(LOG_ERR, "couldn't write PING back to server");
					return -1;
				}
			}
			else if(parseRaw(raw,&chatMsg))
			{
				if(chatMsg.text[0] == '!')
				{
					command(&chatMsg, botMsg);
					syslog(LOG_INFO, "ran command from user: %s using message %s", chatMsg.username, chatMsg.text);
				}
			}
		}
	}
	strcpy(botMsg->text,"/me ~rosebud~");
	chat(botMsg);
	syslog(LOG_INFO, "sent exit message");
	return 0;
}

int cleanup(struct sendMsg *botMsg)
{
	char fileData[64];
	sprintf(fileData,"../%s",PID_DIR);
	chdir(fileData);
	botMsg->channel[0] = '/';
	sem_unlink(botMsg->channel);
	syslog(LOG_INFO, "unlinked semaphore");
	botMsg->channel[0] = '#';
	removePound(botMsg->channel,fileData);
	if(remove(fileData) != 0)
	{
		printf("couldn't remove PID file: %i\n",errno);
		return -1;
	}
	syslog(LOG_INFO, "removed PID file");
	if(disconnect(botMsg) == -1)
	{
		printf("couldn't part from the server gracefully\n");
	}
	syslog(LOG_INFO, "parted from server gracefully");
	closelog();
	return 0;
}
