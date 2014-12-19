source /radsys/bin/saihbd.sh
# aguardar a que arranque el sistema y se ajuste la hora por NTP o RTC
sleep 20
/radsys/bin/yunlogger > /dev/null &
