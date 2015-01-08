#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

eval `/www/usr-cgi/proccgi.sh $*`


#echo '<h2>Nueva Fecha: '$FORM_dia/$FORM_mes/$FORM_anio $FORM_hora:$FORM_minuto:$FORM_segundo'<h2>'
#hwtestrtc -s $FORM_anio-$FORM_mes-$FORM_dia $FORM_hora:$FORM_minuto:$FORM_segundo



echo '<table class="cent">'

echo '<tr class="dato"><td class="cabecera">Fecha antigua</td><td>'
date -u
echo '</td></tr>'
echo '<tr class="dato"><td class="cabecera">Fecha nueva</td><td class="alt">'
wget http://localhost/usr-cgi/luci/arduino/time/1,3,$FORM_anio,$FORM_mes,$FORM_dia,$FORM_hora,$FORM_minuto,$FORM_segundo -O - -q -T 1 > /tmp/yunlogger.setdate
date -s "$FORM_anio-$FORM_mes-$FORM_dia $FORM_hora:$FORM_minuto:$FORM_segundo"
date -u
echo '</td></tr>'

echo '</table>'

/www/usr-cgi/fin.sh /index_saih.html
