#!/bin/sh
SAIHBD=/radsys/
export SAIHBD
HTML_HOME=/www/
export HTML_HOME
echo Content-Type: text/html
echo Expires: Tue, 17 Jun 1979 02:20:00
echo Expires: 0
echo Pragma: no-cache
echo

echo '<html>'
echo '<head>'
echo '<LINK REL="stylesheet" HREF="/estilos.css" TYPE="text/css">'
echo '</head>'

echo '<body>'
echo '<div id="contenido">'
echo '<h3>Generar gr&aacute;fica</h3>'
echo '<br>'

echo '<form action=actualizarMarcoGrafica.sh method=POST target=marcoGrafica>'
echo '<table class="cent">'

echo '<tr><td>Se&ntilde;al</td><td>'
$SAIHBD/bin/leerlogerbd T
echo '<input type=hidden value="A" size=1 maxlength=2 name="sen"></td></tr>'
echo '<tr><td>Valor</td><td>'
echo '<select name="ing">'
echo '<option value="1">Valor Ingenier&iacute;a</option>'
echo '<option value="0">Cuentas</option>'
echo '</select>'

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

echo '<tr><td></td><td><br><input class="boton" type=submit value="Actualizar Gr&aacute;fica"></td></tr>'

echo '</table>'
echo '</form>'
echo '</div></body></html>'