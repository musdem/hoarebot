import random
import socket
import time
import sys
import os
import json

class HoareBot:
    def __init__(self,channel):
        #reading in the oauth token for irc chat
        passwordFile = open('/home/pi/hoarebot/oauthIRC.json')
        self.password = json.load(passwordFile)[0]
        passwordFile.close()
        #initializing all of the global variables and twitch information
        self.server = 'irc.twitch.tv'
        self.port = 6667
        self.channel = channel
        self.botnick = 'hoarebot'
        self.maxRepeat = 20
        self.willForce3 = 0
        self.count = 0
        self.modslist = []
        self.moduleCount = 0
        self.installedModules = []
        self.commands = ['!commands','!modspls','!social']
        self.secretcommands = ['!modcommands','!refreshmods','!ban']
        self.modulecommands = []#use json to load previously installed modules
        #creating the socket and connecting to twitch irc server
        self.irc = socket.socket()
        self.irc.connect((self.server, self.port))
        #loging in and joining the channel that was passed into initialization
        self.irc.send('PASS {}\r\n'.format(self.password).encode('utf-8'))
        self.irc.send('USER {} {} {}\r\n'.format(self.botnick, self.botnick, self.botnick).encode('utf-8'))
        self.irc.send('NICK {}\r\n'.format(self.botnick).encode('utf-8'))
        self.irc.send('JOIN {}\r\n'.format(self.channel).encode('utf-8'))
        self.irc.send('CAP REQ :twitch.tv/commands\r\n'.encode('utf-8'))
        #setting the seed for slots rng
        random.seed(channel)

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

    def refreshModules(self):
        self.installedModules = os.listdir('./modules')
        for i in range(0,len(self.installedModules)):
            self.installedModules = self.installedModules[i].replace('.py','')

    def activateModule(self,module):
        if(module in self.installedModules):
            exec('import modules.{}'.format(module))
            name = 'cmd{}'.format(module)
            exec('{} = modules.{}.{}(self)'.format(name,module,module))
            self.modulecommands.append(name)
            self.modulecommands.append(exec('{}.giveChatCMD()'.format(name)))
            self.modulecommands.append(exec('{}.giveCodeCMD()'.format(name)))
            self.chat('Activated module {}'.format(module))
        else:
            self.chat("{} isn't installed DansGame".format(module))

    def deactivateModule(self,module):
        pos = 0
        while(pos <= len(self.modulecommands) - 2):
            if('module' in self.modulecommands[pos]):
                for i in range(0,3):
                    exec('{} = None'.format(module))
                    del self.modulecommands[pos]
                    print('Removed module {}'.format(module))
                    return(None)
            pos += 3
        print("That module isn't activated DansGame")

    def command(self,cmd):
        if(cmd[1].lower() not in self.commands and cmd[1].split(' ')[0].lower() not in self.secretcommands):
            pos = 1
            while(pos <= len(self.modulecommands) - 1):#module command
                if(cmd[1].lower() in self.modulecommands[pos]):
                    for i in range(0,len(self.modulecommands[pos])):
                        if(cmd[1].split(' ')[0].lower() in self.modulecommands[pos][i]):
                            if(self.modulecommands[pos][i][1] == 'g'):
                                exec(self.modulecommands[pos+1][i].format(self.modulecommands[pos-1]))
                            elif(self.modulecommands[pos][i][1] == 'm' and cmd[0] in self.modslist):
                                exec(self.modulecommands[pos+1][i].format(self.modulecommands[pos-1]))
                            else:
                                self.chat("What do you think you are doing {}? You aren't a mod. DansGame".format(cmd[0]))
                    break
                pos += 3
        elif(cmd[1].lower() in self.commands):
            if(cmd[1].lower() == self.commands[0]):#lists commands
                cmd[1] = ''
                for i in range(0,len(self.commands) - 1):
                    cmd[1] += self.commands[i][1:] + ', '
                self.chat('The commands are: ' + cmd[1] + self.commands[len(self.commands) - 1][1:])
            elif(cmd[1].lower() == self.commands[1]):#!modspls
                self.chat("( ͡° ͜ʖ ͡°)╯╲___卐卐卐卐 Don't mind me just taking the mods for a walk!")
            elif(cmd[1].lower() == self.commands[2]):#!social
                self.chat("Follow Reid on twitter at {} See his shitty anime taste on his MAL: {}".format('https://twitter.com/the__hoare','http://myanimelist.net/profile/lupuswarrior'))
        elif(cmd[1].split(' ')[0].lower() in self.secretcommands):#mod commands
            if(cmd[0] in self.modslist):
                if(cmd[1].lower() == self.secretcommands[0]):#lists mod commands
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
            else:
                self.chat("What do you think you are doing {}? You aren't a mod. DansGame".format(cmd[0]))
        else:
            self.chat('{} is not a command; type !commands to get a list.'.format(cmd[1][1:]))

    def run(self):
        self.setMods()
        while 1:
            raw = self.irc.recv(1024).decode('utf-8')
            #if twitch pings the server we must ping back in order to keep the connection alive
            if raw.find('PING :') != -1:
                self.irc.send('PONG :tim.twitch.tv\r\n'.encode('utf-8'))
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

if __name__ == '__main__':
    bot = HoareBot(sys.argv[1])
    bot.run()
