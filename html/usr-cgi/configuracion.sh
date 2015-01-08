#!/bin/sh

. /www/usr-cgi/inicioLigero.sh

$SAIHBD/bin/leerlogerbdcgi

. /www/usr-cgi/fechahora.sh
. /www/usr-cgi/reiniciar.sh

/www/usr-cgi/fin.sh ./inicioLigero.sh
