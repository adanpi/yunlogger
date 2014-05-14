#!/bin/sh
. ./inicioLigero.sh

echo '<h3>Generar gráfica</h3>'
echo '<br>'

echo '<form action=graficaSeleccionada.sh method=POST>'
echo '<table class=\"cent\">'

echo '<tr><td>Señal</td><td>'
echo '<select name="sen">'
echo '<option value="G">Gray</option>'
echo '<option value="A">Analógica</option>'
echo '<option value="R">RS232</option>'
echo '<option value="C">Contador</option>'
echo '</select>'

echo '<input type=text value="1" size=1 maxlength=2 name="num"></td></tr>'

echo '<tr><td>Valor</td><td>'
echo '<select name="ing">'
echo '<option value="0">Cuentas</option>'
echo '<option value="1">Valor Ingeniería</option>'
echo '</select>'

echo '<tr><td>Fecha Inicial</td><td><input type=text value="'
date +%d
echo '" size=1 maxlength=2 name="dia">/'
echo '<input type=text value="'
date +%m
echo '" size=1 maxlength=2 name="mes">/'
echo '<input type=text value="'
date +%Y
echo '" size=3 maxlength=4 name="anio">  '
echo '<input type=text value="'
#(date +%H)-2
echo '00'
echo '" size=1 maxlength=2 name="hora">:'
echo '<input type=text value="00" size=2 maxlength=2 name="min"></td></tr>'

echo '<tr><td>Intervalo</td><td>'
echo '<select name="tipo">'
echo '<option value="4">4 Horas</option>'
echo '<option value="12">12 Horas</option>'
echo '<option value="24" selected="selected">24 Horas</option>'
echo '<option value="48">48 Horas</option>'
echo '<option value="96">96 Horas</option>'
echo '</select>'

echo '<tr><td></td><td><br><input class="boton" type=submit value="Generar Gráfica"></td></tr>'

echo '</table>'
echo '</form>'

./fin.sh /index_saih.html
