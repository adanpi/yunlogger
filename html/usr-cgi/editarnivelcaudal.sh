#!/bin/sh
. ./inicioLigero.sh

#echo '<img src="/pics/aforo.gif" width="90%">'
$SAIHBD/bin/leeraxisbd E

./fin.sh /usr-cgi/nivelcaudal.sh
