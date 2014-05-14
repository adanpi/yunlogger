#!/bin/sh
. ./inicioLigero.sh


echo '<h3>Estado PLC</h3>'
echo '<br>'

echo '<textarea>'
cat $SAIHBD/log/logos.conf
echo '</textarea>'


./fin.sh /usr-cgi/inicioLigero.sh
