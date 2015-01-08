#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

eval `/www/usr-cgi/proccgi.sh $*`
echo dig: $FORM_sen estado: $FORM_activar
#$SAIHBD/bin/setval Public $FORM_sen $FORM_activar
wget http://127.0.0.1/usr-cgi/luci/arduino/digital/$FORM_sen/$FORM_activar -O - -q

/www/usr-cgi/fin.sh /usr-cgi/telemando.sh
