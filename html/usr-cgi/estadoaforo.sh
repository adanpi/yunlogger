#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

#echo '<br><img src="/pics/aforo.gif" border="0" alt="">'

./leermodbus

/www/usr-cgi/fin.sh /usr-cgi/inicioLigero.sh
