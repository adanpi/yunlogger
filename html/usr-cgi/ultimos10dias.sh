#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

$SAIHBD/bin/leerlogerbd D 10

/www/usr-cgi/fin.sh /usr-cgi/inicioLigero.sh
