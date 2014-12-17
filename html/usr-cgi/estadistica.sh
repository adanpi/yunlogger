#!/bin/sh
. ./inicio.sh


echo '<h3>Estad&iacute;stica de comunicaciones</h3>'
echo '<br>'

$SAIHBD/bin/leeraxisbd C

echo '<br>'

echo '<form action=iniciarestadistica.sh method=POST>'
echo '<input type=hidden value="1" size=1 maxlen=1 name="numero">'
echo '<input class="botonc" type=submit value="Reiniciar Contadores">'
echo '</form>'


./fin.sh /index_saih.html
