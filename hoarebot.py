import random
import socket
import time
import sys
import json
import time

class HoareBot:
    def __init__(self,channel):
        #reading in the oauth token for irc chat
        passwordFile = open('/home/pi/hoarebot/oauthIRC.json')
        self.password = json.load(passwordFile)[0]
        passwordFile.close()
        #reading in the current copy pasta list
        pastaFile = open('/home/pi/hoarebot/pasta.json')
        self.pastalist = json.load(pastaFile)
        pastaFile.close()
        #reading in the current copy pasta list
        healthyFile = open('/home/pi/hoarebot/healthy.json')
        self.healthylist = json.load(healthyFile)
        healthyFile.close()
        #reading in modspls messages
        modsplsFile = open('/home/pi/hoarebot/modspls.json')
        self.modsplsList = json.load(modsplsFile)
        modsplsFile.close()
        #initializing all of the global variables and twitch information
        self.server = 'irc.twitch.tv'
        self.port = 6667
        self.channel = channel
        self.botnick = 'hoarebot'
        self.maxRepeat = 20
        self.willForce3 = 0
        self.count = 0
        self.drawActive = False
        self.drawlist = []
        self.modslist = []
        #setting the list of commands and emotes for slots
        self.emotes = ['Kappa','KappaPride','EleGiggle','BibleThump','PogChamp','TriHard','CoolCat','WutFace','Kreygasm']
        self.commands = ['!commands','!slots','!pasta','!modspls','!raffle','!social','!healthy']
        self.secretcommands = ['!modcommands','!refreshmods','!ban','!updatepasta','!toggleraffle','!raffledraw','!updatehealthy','!removepasta','!removehealthy']
        #creating the socket and connecting to twitch irc server
        self.irc = socket.socket()
        self.connect()
        #setting the seed for slots rng
        random.seed(channel)

    def connect(self):
        self.irc.connect((self.server, self.port))
        #loging in and joining the channel that was passed into initialization
        self.irc.send('PASS {}\r\n'.format(self.password).encode('utf-8'))
        self.irc.send('USER {} {} {}\r\n'.format(self.botnick, self.botnick, self.botnick).encode('utf-8'))
        self.irc.send('NICK {}\r\n'.format(self.botnick).encode('utf-8'))
        self.irc.send('JOIN {}\r\n'.format(self.channel).encode('utf-8'))
        self.irc.send('CAP REQ :twitch.tv/commands\r\n'.encode('utf-8'))

    def timeout(self,username,length):
        self.chat('/timeout {} {}'.format(username, length))

    def ban(self,username):
        self.chat('░░░░░░░░░░░░ ▄████▄░░░░░░░░░░░░░░░░░░░░ ██████▄░░░░░░▄▄▄░░░░░░░░░░ ░███▀▀▀▄▄▄▀▀▀░░░░░░░░░░░░░ ░░░▄▀▀▀▄░░░█▀▀▄░▄▀▀▄░█▄░█░ ░░░▄▄████░░█▀▀▄░█▄▄█░█▀▄█░ ░░░░██████░█▄▄▀░█░░█░█░▀█░ ░░░░░▀▀▀▀░░░░░░░░░░░░░░░░░')
        self.chat('/ban {}'.format(username))

    def chat(self,msg,override=False):
        self.irc.sendall('PRIVMSG {} :{}\r\n'.format(self.channel, msg).encode('utf-8'))

    def isSpam(self,msg):
        if(msg[1].count(msg[1].split(' ')[0]) > self.maxRepeat):
            self.chat('What the FUCK do you think you are doing {}?!'.format(msg[0]))
            self.timeout(msg[0],60)

    def setMods(self):
        self.chat('/mods')
        for i in range(0,100):
            raw = self.irc.recv(1024).decode('utf-8')
            if(':tmi.twitch.tv NOTICE {} :The moderators of this room are: '.format(self.channel) in raw):
                self.chat('Mods set')
                self.modslist = raw.strip(':tmi.twitch.tv NOTICE {} :The moderators of this room are: '.format(self.channel)).split(', ')
                break
            elif(i == 99):
                self.chat('Mod set failed')
        self.modslist[len(self.modslist) - 1] = self.modslist[len(self.modslist) - 1].strip('\r\n')

    def slots(self,username):
        if(self.willForce3 > random.randint(10,20)):
            emote = random.choice(self.emotes)
            outcome =  '{} | {} | {}'.format(emote,emote,emote)
            self.willForce3 = 0
        else:
            outcome =  '{} | {} | {}'.format(random.choice(self.emotes),random.choice(self.emotes),random.choice(self.emotes))
            self.willForce3 += 1
        self.chat(outcome)
        if(outcome == 'Kappa | Kappa | Kappa'):
            self.chat('Goodbye {} Kappa'.format(username))
            self.timeout(username,1)
        elif(outcome == 'TriHard | TriHard | TriHard'):
            uNameHype = 'TriHard '
            for i in range(0,len(username)):
                uNameHype += username.upper()[i] + ' TriHard '
            uNameHype += 'H TriHard Y TriHard P TriHard E TriHard'
            self.chat(uNameHype)
        elif(outcome == 'Kreygasm | Kreygasm | Kreygasm'):
            self.chat('Kreygasm For later tonight {} https://www.youtube.com/watch?v=P-Vvm7M4Lig Kreygasm'.format(username))

    def updateList(self,item,listType,mode):
        if(mode == 'w'):
            if(listType == 'pasta'):
                if(not(item in self.pastalist)):
                    self.pastalist.append(item)
                    pastaFile = open('/home/pi/hoarebot/pasta.json','w')
                    json.dump(self.pastalist,pastaFile)
                    pastaFile.close()
                    self.chat("{} added to pasta".format(item))
                else:
                    self.chat('That is already in the list FailFish')
            elif(listType == 'healthy'):
                if(not(item in self.healthylist)):
                    self.healthylist.append(item)
                    healthyFile = open('/home/pi/hoarebot/healthy.json','w')
                    json.dump(self.healthylist,healthyFile)
                    healthyFile.close()
                    self.chat("{} added to healthy".format(item))
                else:
                    self.chat('That is already in the list FailFish')
            else:
                self.chat("musdem fucked up Kappa no such list FailFish")#error message
        elif(mode == 'd'):
            if(listType == 'pasta'):
                if(item in self.pastalist):
                    self.pastalist.remove(item)
                    pastaFile = open('/home/pi/hoarebot/pasta.json','w')
                    json.dump(self.pastalist,pastaFile)
                    pastaFile.close()
                    self.chat("{} removed from pasta".format(item))
                else:
                    self.chat("That isn't in the list FailFish")
            elif(listType == 'healthy'):
                if(item in self.healthylist):
                    self.healthylist.remove(item)
                    healthyFile = open('/home/pi/hoarebot/healthy.json','w')
                    json.dump(self.healthylist,healthyFile)
                    healthyFile.close()
                    self.chat("{} removed from healthy".format(item))
                else:
                    self.chat("That isn't in the list FailFish")
            else:
                self.chat("musdem fucked up Kappa no such list FailFish")#error message
        else:
            self.chat('musdem fucked up Kappa no such mode FailFish')#error message

    def enterDraw(self,username):
        if(self.drawActive):
            if(username in self.drawlist):
                self.chat('You are already in the draw {}'.format(username))
            else:
                self.drawlist.append(username)
                self.chat('Entered {} into the draw.'.format(username))
        else:
            self.chat('There is no draw dumbass FailFish')
    def draw(self):
        if(len(self.drawlist) > 0):
            self.chat('TriHard {} HAS WON THE DRAW! TriHard'.format(random.choice(self.drawlist).upper()))
            self.drawlist = []
        else:
            self.chat('BibleThump No one entered the draw. BibleThump')

    def command(self,cmd):
        if(cmd[1].lower() == self.commands[0]):#lists commands
            cmd[1] = ''
            for i in range(0,len(self.commands) - 1):
                cmd[1] += self.commands[i][1:] + ', '
            self.chat('The commands are: ' + cmd[1] + self.commands[len(self.commands) - 1][1:])
        elif(cmd[1].lower() == self.commands[1]):#!slots
            self.slots(cmd[0])
        elif(cmd[1].lower() == self.commands[2]):#!pasta
            self.chat(random.choice(self.pastalist))
        elif(cmd[1].lower() == self.commands[3]):#!modspls
            self.chat(random.choice(self.modsplsList))
        elif(cmd[1].lower() == self.commands[4]):#!raffle
            self.enterDraw(cmd[0])
        elif(cmd[1].lower() == self.commands[5]):#!social
            self.chat("Follow Reid on twitter at {} See his shitty anime taste on his MAL: {}".format('https://twitter.com/the__hoare','http://myanimelist.net/profile/lupuswarrior'))
        elif(cmd[1].lower() == self.commands[6]):#!healthy
            self.chat(random.choice(self.healthylist))
        elif(cmd[1] == '!'):#are you fucking happy trevor??
            self.chat('Are you fucking happy Trevor?')
        elif(cmd[1].split(' ')[0].lower() in self.secretcommands):#mod commands
            if(cmd[0] in self.modslist):
                if(cmd[1].lower() == self.secretcommands[0]):#lsits mod commands
                    cmd[1] = ''
                    for i in range(0,len(self.secretcommands) - 1):
                        cmd[1] += self.secretcommands[i][1:] + ', '
                    self.chat('The mod commands are: ' + cmd[1] + self.secretcommands[len(self.secretcommands) - 1][1:])
                elif(cmd[1].lower() == self.secretcommands[1]):#!refreshmods
                    self.setMods()
                elif(cmd[1].split(' ')[0].lower() == self.secretcommands[2]):#!ban
                    if(len(cmd[1].split(' ')) > 1):
                        self.ban(cmd[1].split(' ')[1])
                    else:
                        self.chat('You forgot the username {} FailFish'.format(cmd[0]))
                elif(cmd[1].split(' ')[0].lower() == self.secretcommands[3]):#!updatepasta
                    if(len(cmd[1].split(' ')) > 1):
                        self.updateList(cmd[1].strip('!updatepasta '),'pasta','w')
                    else:
                        self.chat('You forgot the pasta {} FailFish'.format(cmd[0]))
                elif(cmd[1].lower() == self.secretcommands[4]):#!toggleraffle
                    self.drawActive ^= True
                    if(self.drawActive):
                        self.chat('Draw started, type !raffle to enter the draw.')
                    else:
                        self.chat('Draw stopped.')
                elif(cmd[1].lower() == self.secretcommands[5]):#!raffleDraw
                    self.draw()
                elif(cmd[1].split(' ')[0].lower() == self.secretcommands[6]):#!updatehealthy
                    if(len(cmd[1].split(' ')) > 1):
                        self.updateList(cmd[1].strip('!updatehealthy '),'healthy','w')
                    else:
                        self.chat('You forgot the lewd {} FailFish'.format(cmd[0]))
                elif(cmd[1].split(' ')[0].lower() == self.secretcommands[7]):#!removepasta
                    if(len(cmd[1].split(' ')) > 1):
                        self.updateList(cmd[1].strip('!removepasta '),'pasta','d')
                    else:
                        self.chat('You forgot the pasta {} FailFish'.format(cmd[0]))
                elif(cmd[1].split(' ')[0].lower() == self.secretcommands[8]):#!removehealthy
                    if(len(cmd[1].split(' ')) > 1):
                        self.updateList(cmd[1].strip('!removehealthy '),'healthy','d')
                    else:
                        self.chat('You forgot the lewd {} FailFish'.format(cmd[0]))
            else:
                self.chat('What do you think you are doing {} DansGame'.format(cmd[0]))
        else:
            self.chat('{} is not a command; type !commands to get a list.'.format(cmd[1][1:]))

    def run(self):
        self.setMods()
        lastPing = time.time()
        pingThreshold = 5 * 60 #5 mintues
        while 1:
            raw = self.irc.recv(1024).decode('utf-8')
            #if twitch pings the server we must ping back in order to keep the connection alive
            if raw.find('PING :') != -1:
                self.irc.send('PONG :tim.twitch.tv\r\n'.encode('utf-8'))
                lastPing = time.time()
            else:
                #this splits the message into a list where index 0 is the username and index 1 is the actual message
                message = [raw.split(":")[1].split("!")[0],raw.split(self.channel + ' :')]
                if(len(message[1]) > 1):
                    message[1] = message[1][1].strip('\r\n')
                    self.isSpam(message)
                    if(message[1][0] == '!'):
                        self.command(message)
                    elif('fuck you hoarebot' in message[1].lower()):
                        self.timeout(message[0],1)
                        self.chat('EleGiggle Fuck you too {} SoBayed'.format(message[0]))
            if (time.time() - lastPing) > pingThreshold:#reconnects if too much time has passed since last ping
                self.connect()

if __name__ == '__main__':
    bot = HoareBot(sys.argv[1])
    bot.run()
