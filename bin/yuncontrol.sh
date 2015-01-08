source /radsys/bin/saihbd.sh
# aguardar a que arranque el sistema y se ajuste la hora por NTP o RTC
sleep 2
/radsys/bin/yuncontrol > /dev/null &
