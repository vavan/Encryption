#!/bin/bash

#../enc_client.py 127.0.0.0.1:8080 && 
~/bin/ffmpeg -re -i ./output.flv -t 00:00:03 -f flv -ar 11025 rtmps://127.0.0.1:8080/rtmp/test_vova?key=key.pem 


