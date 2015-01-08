#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

#eval `./proccgi.sh $*`
eval `/www/usr-cgi/proccgi.sh $*`
echo '<h3>Hist&oacute;rico Se&ntilde;al (24h)</h3>'
echo '<br>'
echo '<pre>'
$SAIHBD/bin/leeraxisbd HQM $FORM_sen $FORM_num
echo '</pre>'
/www/usr-cgi/fin.sh /usr-cgi/supervision.sh


