#!/bin/sh
. ./inicioLigero.sh

eval `./proccgi.sh $*`

#echo $FORM_punto $FORM_nivel $FORM_caudal

$SAIHBD/bin/leeraxisbd E $FORM_punto $FORM_nivel $FORM_caudal
sleep 1
$SAIHBD/bin/leeraxisbd E

./fin.sh /usr-cgi/nivelcaudal.sh