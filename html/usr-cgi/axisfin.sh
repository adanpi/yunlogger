#!/bin/sh
. /www/usr-cgi/inicioLigero.sh


echo '<h3>Parando los procesos del Axis</h3>'
echo '<br>'

echo '<textarea>'
$SAIHBD/bin/killpid axisloger 15
$SAIHBD/bin/killpid ipcserver 9
echo '</textarea>'


/www/usr-cgi/fin.sh /usr-cgi/status.sh
