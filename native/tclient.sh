#!/bin/bash

./bin/ssl-key-client 71.96.94.69:8080 0.0.0.0:1935 /usr/bin/ffmpeg -re -i ../test/client/output.flv -t 00:00:20 -f flv -ar 11025 rtmps://127.0.0.1:1935/rtmp/test_vova?key=key.pem 


