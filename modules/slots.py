class slots:
    def __init__(self,bot):
        self.emotes = ['Kappa','KappaPride','EleGiggle','BibleThump','PogChamp','TriHard','CoolCat','WutFace','Kreygasm']
        self.willForce3 = 0
        self.bot = bot
        from bot import random

    #the following chat commands should be in the same order as their corresponding code commands
    #this returns a list of all chat commands for this module, g means general chat m means mods only
    def giveChatCMD(self):
        return ["g!slots"]

    #this returns a list of all code commands and how they should be set up
    def giveCMD(self):
        return ["{}.slots(cmd[0])"]

    def slots(self,username):
        if(self.willForce3 > random.randint(10,20)):
            emote = random.choice(self.emotes)
            outcome =  '{} | {} | {}'.format(emote,emote,emote)
            self.willForce3 = 0
        else:
            outcome =  '{} | {} | {}'.format(random.choice(self.emotes),random.choice(self.emotes),random.choice(self.emotes))
            self.willForce3 += 1
        self.bot.chat(outcome)
        if(outcome == 'Kappa | Kappa | Kappa'):
            self.bot.chat('Goodbye {} Kappa'.format(username))
            self.bot.timeout(username,1)
        elif(outcome == 'TriHard | TriHard | TriHard'):
            uNameHype = 'TriHard '
            for i in range(0,len(username)):
                uNameHype += username.upper()[i] + ' TriHard '
                uNameHype += 'H TriHard Y TriHard P TriHard E TriHard'
                self.bot.chat(uNameHype)
        elif(outcome == 'Kreygasm | Kreygasm | Kreygasm'):
            self.bot.chat('Kreygasm For later tonight {} https://www.youtube.com/watch?v=P-Vvm7M4Lig Kreygasm'.format(username))
