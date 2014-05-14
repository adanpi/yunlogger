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

eval `/www/usr-cgi/proccgi.sh $*`

$SAIHBD/bin/datoscgi2 $FORM_sen $FORM_num $FORM_tipo $FORM_ing $FORM_anio $FORM_mes $FORM_dia $FORM_hora $FORM_min > $HTML_HOME/datos.json

echo '<html>'
echo '<head>'
echo '<LINK REL="stylesheet" HREF="/estilos.css" TYPE="text/css">'
echo '</head>'
echo '<body>'
echo '<script type="text/javascript" src="/swfobject.js"></script>'
echo '<script type="text/javascript">'
echo 'swfobject.embedSWF("/open-flash-chart.swf", "chart", "550", "350", "9.0.0", "expressInstall.swf",{"data-file":"/datos.json"});'
echo '</script>'
echo '<div id="contenido">'
echo '<BR><div id="chart">'
echo '</div></div></body></html>'