class raffle:
    def __init__(self,bot):
        self.drawActive = False
        self.drawlist = []
        self.bot = bot
        from bot import random

    #the following chat commands should be in the same order as their corresponding code commands
    #this returns a list of all chat commands for this module, g means general chat m means mods only
    def giveChatCMD(self):
        return ["g!raffle","m!toggleraffle","m!raffledraw"]

    #this returns a list of all code commands and how they should be set up
    def giveCodeCMD(self):
        return ["{}.enterDraw(cmd[0])","{}.draw()","{}.toggleRaffle()"]

    def enterDraw(username):
        if(self.drawActive):
            if(username in self.drawlist):
                self.bot.chat('You are already in the draw {}'.format(username))
            else:
                self.drawlist.append(username)
                self.bot.chat('Entered {} into the draw.'.format(username))
        else:
            self.bot.chat('There is no draw dumbass FailFish')

    def draw():
        if(len(self.drawlist) > 0):
            self.bot.chat('TriHard {} HAS WON THE DRAW! TriHard'.format(random.choice(self.drawlist).upper()))
            self.drawlist = []
        else:
            self.bot.chat('BibleThump No one entered the draw. BibleThump')

    def toggleRaffle():
        self.drawActive ^= True
        if(self.drawActive):
            self.bot.chat('Draw started, type !raffle to enter the draw.')
        else:
            self.bot.chat('Draw stopped.')
