#!/bin/sh
. ./inicioLigero.sh

echo '<img src="/pics/aforo.gif" width="90%">'
$SAIHBD/bin/leeraxisbd N

./fin.sh /usr-cgi/inicioLigero.sh
