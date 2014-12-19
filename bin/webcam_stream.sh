mjpg_streamer -i "./input_uvc.so -y -n -d /dev/video0 -q 50 -r QVGA -f 6" -o "./output_http.so -p 8080 -n -w /www/webcam"
