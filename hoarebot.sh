#!/bin/bash
#
# chkconfig: 35 90 12
# description: twitch bot called hoarebot
#
# Start the service hoarebot
start()
{
    /usr/bin/python3 /home/pi/hoarebot/hoarebot.py "#lupuswarrior"&
    echo "Hoarebot started"
}
# Restart the service hoarebot
stop()
{
    pid=`ps -ef | grep 'python3 /home/pi/hoarebot/hoarebot.py' | awk '{ print $2 }'`
    echo $pid
    kill $pid
    sleep 2
    echo "Hoarebot killed."
}
### main logic ###
case "$1" in
    start)
	start
	;;
    stop)
	stop
	;;
    restart)
	stop
	start
	;;
    *)
	echo "Usage: /etc/init/hoarebot {start|stop|restart}"
	exit 1
esac
exit 0
