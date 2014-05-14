#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

echo '<h3>Archivo LOG conexion GPRS</h3>'
echo '<textarea>'
cat /tmp/gprs.log
echo '</textarea>'

/www/usr-cgi/fin.sh /logs.html
