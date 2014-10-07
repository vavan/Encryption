#!/bin/bash

ulimit -c unlimited
#rm ../../native/core

rm -r *.log
rm -r gmon.out

../../python/ssl-tunel.py 127.0.0.1:2000 s127.0.0.1:4000 &
#../../native/bin/tunnel 127.0.0.1:4000 127.0.0.1:6000 &


sleep 1

#ps ax | egrep "ssl-tunel.py|tunnel"

./test_proxy.py $1


killall -9 ssl-tunel.py
#killall -TERM tunnel


sleep 1
gprof ../../native/bin/tunnel > prof.report 
