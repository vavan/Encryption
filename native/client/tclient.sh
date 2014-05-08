#!/bin/bash

#../enc_client.py 127.0.0.0.1:8080 && 
~/bin/ffmpeg -re -i ../../test/client/output.flv -t 00:00:20 -f flv -ar 11025 rtmps://127.0.0.1:1935/rtmp/test_vova?key=key.pem 


