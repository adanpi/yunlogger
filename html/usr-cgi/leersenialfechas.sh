#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

#eval `./proccgi.sh $*`
eval `/www/usr-cgi/proccgi.sh $*`
echo '<pre>'
$SAIHBD/bin/leerlogerbd I $FORM_dia/$FORM_mes/$FORM_anio $FORM_hora:$FORM_min $FORM_tipo $FORM_sen $FORM_num
echo '</pre>'
/www/usr-cgi/fin.sh /usr-cgi/supervision.sh
