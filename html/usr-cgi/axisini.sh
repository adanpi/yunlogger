#!/bin/sh
. /www/usr-cgi/inicioLigero.sh


echo '<h3>Reiniciando los procesos del AXPLC</h3>'
echo '<br>'

echo '<textarea>'
echo '$SAIHBD/bin/killpid axissac 15'
$SAIHBD/bin/killpid axisloger 15
echo
echo '$SAIHBD/bin/killpid ipcserver 9'
$SAIHBD/bin/killpid ipcserver 9
sleep 3
echo
echo '$SAIHBD/bin/axisloger > /dev/null &'
$SAIHBD/bin/axisloger > /dev/null &
echo
echo '$SAIHBD/bin/bin/ipcserver kosmos > /dev/null &'
$SAIHBD/bin/ipcserver kosmos > /dev/null &
echo '</textarea>'


/www/usr-cgi/fin.sh /usr-cgi/status.sh
