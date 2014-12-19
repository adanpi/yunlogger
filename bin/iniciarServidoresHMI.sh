cd /radsys/mbasync
python mbasyncserver.py &
sleep 3
cd /radsys/hmiserver
python hmiserver/hmiservermbc.py -p 8082 -h 127.0.0.1 -r 502 -u 1 -t 30.0 &
