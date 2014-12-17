
echo '<h3>Fecha Actual</h3>'

echo '<br>'

echo '<form action=cambiarfechahora.sh method=POST>'

echo '<table class=\"cent\">'
echo '<tr><td><input type=text value="'
date +%d
echo '" size=1 maxlength=2 name="dia">/'
echo '<input type=text value="'
date +%m
echo '" size=1 maxlength=2 name="mes">/'
echo '<input type=text value="'
date +%Y
echo '" size=3 maxlength=4 name="anio">  '
echo '<input type=text value="'
date +%H
echo '" size=1 maxlength=2 name="hora">:'
echo '<td><input type=text value="'
date +%M
echo '" size=2 maxlen=2 name="minuto">:</td>'
echo '<td><input type=text value="'
date +%S
echo '" size=2 maxlen=2 name="segundo"></td></tr>'

echo '<tr><td><br><input class="boton" type=submit value="Modificar hora"></td></tr>'

echo '</table>'
echo '</form>'

