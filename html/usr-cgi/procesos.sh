#!/bin/sh
. /www/usr-cgi/inicio.sh


echo '<h3>Procesos activos</h3>'
echo '<br>'

echo '<textarea>'
ps 
echo '</textarea>'

echo ' <br><br>'

echo '<table class="cent">'
echo '<tr><form action=pararproceso.sh method=POST>'
echo '<td>Detener proceso identificado por PID</td>'
echo '<td><input type=text value="" size=10 maxlen=10 name="pid"></td>'
echo '<td><input class="botond" type=submit value="Detener"></td>'
echo '</form></tr>'

echo '<tr><form action=lanzarproceso.sh method=POST>'
echo '<td>Ejecutar proceso especificado por comando</td>'
echo '<td><input type=text value="" size=25 maxlen=25 name="apl"></td>'
echo '<td><input class="botond" type=submit value="Ejecutar"></td>'
echo '</form></tr>'
echo '</table>'


/www/usr-cgi/fin.sh /usr-cgi/aplicaciones.sh
