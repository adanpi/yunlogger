#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

#eval `./proccgi.sh $*`
eval `/www/usr-cgi/proccgi.sh $*`
$SAIHBD/bin/leerlogerbd QM $FORM_dia/$FORM_mes/$FORM_anio $FORM_hora:$FORM_min

/www/usr-cgi/fin.sh /usr-cgi/supervision.sh
