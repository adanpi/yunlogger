source /radsys/bin/saihbd.sh
# esperar a que se ajuste al hora por NTP
# sleep 60
# se ha colocado RTC, no hace falta esperar
/radsys/bin/AxionMain > /dev/null &
touch /radsys/started.AxionMain
