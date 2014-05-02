#!/bin/bash

#../enc_client.py 127.0.0.0.1:8080 && 
~/bin/ffmpeg -re -i ./crocodile_dundee.avi -t 00:10:00 -f flv -ar 11025 rtmps://127.0.0.1:8080/rtmp/test_vova?key=key.pem 


