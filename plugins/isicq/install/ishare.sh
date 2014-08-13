#!/bin/sh
# IShare Daemmon initialization script
#
# chkconfig: 5 96 4
# description: IShare daemon start/stop
#

# Calls the basic init.d functions
. /etc/rc.d/init.d/functions

cd /etc

case $1 in
        start)
                action "Starting I-Share" /usr/bin/ishare --start --user=nobody --log=/tmp/ishare.log

        ;;
        stop)
                action "Stopping I-Share" killall ishare
        ;;
        restart)
                $0 stop
                $0 start

        ;;
        status)
                eds=`ps -aux | grep ishare | awk '{ print $2 }'`
                echo $eds
        ;;
        *)
        echo "*** Usage: mqm {start|stop|restart|status}"
        exit 1

esac
exit 0
