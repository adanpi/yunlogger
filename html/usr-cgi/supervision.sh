#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

echo '<table class="estr">'

echo '<tr>'
echo '<h3>Dato Quinceminutal</h3>'
echo '<br>'
echo '<form action=leerdato.sh method=POST>'
echo '<table class="cent">'
$SAIHBD/bin/leerlogerbd U
echo '<td><input class="boton" type=submit value="Consultar Datos"></td>'
echo '</table>'
echo '</form>'
echo '<br>'
echo '</tr>'

echo '<tr>'
echo '<h3>Datos Quinceminutales por Fecha y Se&ntilde;al </h3>'
echo '<br>'
echo '<form action=leersenialfechas.sh method=POST>'
echo '<table class="cent">'
echo '<tr><td>Se&ntilde;al</td><td>'
$SAIHBD/bin/leerlogerbd T
echo '<input type=hidden value="A" size=1 maxlength=2 name="sen"></tr>'
echo '<tr><td>Fecha Inicial</td>'
$SAIHBD/bin/leerlogerbd UG
echo '</tr>'
echo '<tr><td>Intervalo</td><td>'
echo '<select name="tipo">'
echo '<option value="4" selected="selected">4 Horas</option>'
echo '<option value="12">12 Horas</option>'
echo '<option value="24">24 Horas</option>'
echo '<option value="48">48 Horas</option>'
echo '<option value="96">96 Horas</option>'
echo '</select>'
echo '</td></tr>'
echo '<tr><td></td><td><input class="boton" type=submit value="Consultar Datos"></td></tr>'
echo '</table>'
echo '</form>'
echo '<br>'
echo '</tr>'

echo '</table>'

/www/usr-cgi/fin.sh ./inicioLigero.sh
