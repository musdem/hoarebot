struct Channel
{
    pid_t PID;
    char name[128];
};

int joinChannel(struct Channel chnl);
