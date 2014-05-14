#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

echo '<h3>LOG del Sistema</h3>'
echo '<textarea>'
#cat /var/log/messages
dmesg
echo '</textarea>'

/www/usr-cgi/fin.sh /logs.html
