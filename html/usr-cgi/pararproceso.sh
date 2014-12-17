#!/bin/sh
. ./inicio.sh


eval `./proccgi.sh $*`

echo '<h3>Parando proceso con PID '
echo $FORM_pid
echo '</h3>'
echo '<br>'

echo '<textarea>'

echo 'kill -9' $FORM_pid
kill -9 $FORM_pid
echo
echo '	-------- HECHO --------'
echo
ps x

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


./fin.sh /usr-cgi/procesos.sh
