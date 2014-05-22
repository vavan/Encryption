#!/bin/bash


TUNEL=ssl-tunel.py
PATHTO=../../python


case "$1" in
    start)
        $PATHTO/$TUNEL 0.0.0.0:2001 s127.0.0.1:2002 &
        $PATHTO/$TUNEL 0.0.0.0:2002 127.0.0.1:2003 encrypted.log &
        $PATHTO/$TUNEL s0.0.0.0:2003 127.0.0.1:2004 &
        ncat -l 2004 --keep-open --exec "/bin/cat"
        ;;

    stop)
        killall -9 $TUNEL
        ;;

    restart)
        $0 stop
        sleep 2
        $0 start
        ;;


    status)
        ps ax | grep $TUNEL | grep -v grep
        ;;

    *)
        echo "Usage: $NAME {start|stop|restart|status}" >&2
        exit 1
        ;;
esac

exit 0

