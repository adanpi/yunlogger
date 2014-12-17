#!/bin/sh
. /www/usr-cgi/inicioLigero.sh


echo '<h3>Log de la Aplicaci&oacute;n</h3>'
echo '<br>'

echo '<textarea>'
cat $SAIHBD/log/axis.log
echo '</textarea>'


/www/usr-cgi/fin.sh /logs.html
