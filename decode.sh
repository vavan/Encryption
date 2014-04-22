rm -f output.flv
ffmpeg -re -f flv -i rtmps://71.96.94.69:8080/rtmp/test_vova?ca_file=cert.pem output.flv

