#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

eval `./proccgi.sh $*`
$SAIHBD/bin/incidenciascgi $FORM_num $FORM_sen

/www/usr-cgi/fin.sh /usr-cgi/inicioLigero.sh
