#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

eval `/www/usr-cgi/proccgi.sh $*`
echo servo: $FORM_sen valor: $FORM_activar
#$SAIHBD/bin/setval Public $FORM_sen $FORM_activar
wget http://127.0.0.1/usr-cgi/luci/arduino/servo/$FORM_sen/$FORM_activar -O - -q
valorHex=`python /radsys/bin/f2h.py $FORM_activar`
/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 1054 -f 16 -q 2 -d $valorHex
                
/www/usr-cgi/fin.sh /usr-cgi/telemando.sh
