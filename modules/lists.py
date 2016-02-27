class lists:
    def __init__(self,bot):
        #reading in the current copy pasta list
        pastaFile = open('/home/pi/hoarebot/pasta.json')
        self.pastalist = json.load(pastaFile)
        pastaFile.close()
        #reading in the current copy pasta list
        healthyFile = open('/home/pi/hoarebot/healthy.json')
        self.healthylist = json.load(healthyFile)
        healthyFile.close()
        self.bot = bot
        from bot import random

    #the following chat commands should be in the same order as their corresponding code commands
    #this returns a list of all chat commands for this module, g means general chat m means mods only
    def giveChatCMD(self):
        return ["g!pasta","g!healthy","m!updatepasta","m!removepasta","m!updatehealthy","m!removepasta"]

    #this returns a list of all code commands and how they should be set up
    def giveCMD(self):#this should be redone to fit with command method in hoarebot
        return [#!pasta
            """{}.sendItem('pasta')
            """,#!healthy
            """{}.sendItem('healthy')
            """,#!updatepasta
            """if len(cmd[1].split(' ')) > 1:
                {}.updateList(cmd[1].strip('!updatepasta '),'pasta','w')
            else:
                self.chat('You forgot the pasta ' + cmd[0] + ' FailFish')
            """,#!removepasta
            """if len(cmd[1].split(' ')) > 1:
                {}.updateList(cmd[1].strip('!removepasta '),'pasta','d')
            else:
                self.chat('You forgot the pasta ' + cmd[0] + ' FailFish')
            """,#!updatehealthy
            """if len(cmd[1].split(' ')) > 1:
                {}.updateList(cmd[1].strip('!updatehealthy '),'healthy','w')
            else:
                self.chat('You forgot the lewd ' + cmd[0] + ' FailFish')
            """,#!removehealthy
            """if len(cmd[1].split(' ')) > 1:
                {}.updateList(cmd[1].strip('!removehealthy '),'healthy','d')
            else:
                self.chat('You forgot the lewd ' + cmd[0] + ' FailFish')
            """]

    def updateList(item,listType,mode):
        if(mode == 'w'):
            if(listType == 'pasta'):
                if(not(item in self.pastalist)):
                    self.pastalist.append(item)
                    pastaFile = open('/home/pi/hoarebot/pasta.json','w')
                    json.dump(self.pastalist,pastaFile)
                    pastaFile.close()
                    self.bot.chat("{} added to pasta".format(item))
                else:
                    self.bot.chat('That is already in the list FailFish')
            elif(listType == 'healthy'):
                if(not(item in self.healthylist)):
                    self.healthylist.append(item)
                    healthyFile = open('/home/pi/hoarebot/healthy.json','w')
                    json.dump(self.healthylist,healthyFile)
                    healthyFile.close()
                    self.bot.chat("{} added to healthy".format(item))
                else:
                    self.bot.chat('That is already in the list FailFish')
            else:
                self.bot.chat("musdem fucked up Kappa no such list FailFish")#error message
        elif(mode == 'd'):
            if(listType == 'pasta'):
                if(item in self.pastalist):
                    self.pastalist.remove(item)
                    pastaFile = open('/home/pi/hoarebot/pasta.json','w')
                    json.dump(self.pastalist,pastaFile)
                    pastaFile.close()
                    self.bot.chat("{} removed from pasta".format(item))
                else:
                    self.bot.chat("That isn't in the list FailFish")
            elif(listType == 'healthy'):
                if(item in self.healthylist):
                    self.healthylist.remove(item)
                    healthyFile = open('/home/pi/hoarebot/healthy.json','w')
                    json.dump(self.healthylist,healthyFile)
                    healthyFile.close()
                    self.bot.chat("{} removed from healthy".format(item))
                else:
                    self.bot.chat("That isn't in the list FailFish")
            else:
                self.bot.chat("musdem fucked up Kappa no such list FailFish")#error message
        else:
            self.bot.chat('musdem fucked up Kappa no such mode FailFish')#error message

    def sendItem(listType):
        if(listType == 'healthy'):
            self.bot.chat(random.choice(self.healthylist))
        elif(listType == 'pasta'):
            self.bot.chat(random.choice(self.pastalist))
