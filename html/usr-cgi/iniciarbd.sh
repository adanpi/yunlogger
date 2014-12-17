#!/bin/sh
. /www/usr-cgi/inicioLigero.sh


echo '<H3>Borrando BBDD</H3>'
echo '<br>'

echo '<textarea>'
echo '$SAIHBD/bin/killpid axissac'
$SAIHBD/bin/killpid axisloger 15
echo
echo 'rm $SAIHBD/*.dat'
rm $SAIHBD/LogerAnalogicas.dat
rm $SAIHBD/LogerDigitales.dat
rm $SAIHBD/LogerGen.dat
sleep 2
$SAIHBD/bin/axisloger > /dev/null &
sleep 2
$SAIHBD/bin/killpid axisloger 15
echo '$SAIHBD/bin/deftablas A'
$SAIHBD/bin/deftablas G 
sleep 10
echo '$SAIHBD/bin/axisloger > /dev/null &'
$SAIHBD/bin/axisloger > /dev/null &
echo '</textarea>'


/www/usr-cgi/fin.sh ./status.sh

