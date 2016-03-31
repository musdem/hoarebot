struct Channel
{
    pid_t PID;
    char name[128];
};

void joinChannel(struct Channel chnl);
