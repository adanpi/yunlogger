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
echo '<title>'
hostname
echo '</title>'
#echo '<LINK REL="stylesheet" HREF="http://10.31.237.249/estilos.css" TYPE="text/css">'
echo '<LINK REL="stylesheet" HREF="/estilos.css" TYPE="text/css">'
#echo '<script type="text/javascript" language="JavaScript" src="http://10.31.237.249/switch.js"></script>'
#echo '<script type="text/javascript" language="JavaScript" src="/switch.js"></script>'
echo '<script type="text/javascript" language="JavaScript" src="/javascript.js"></script>'
echo '<!--[if gte IE 5.5]>'
#echo '<script language="JavaScript" src="http://10.31.237.249/dhtml.js" type="text/JavaScript"></script>'
echo '<script language="JavaScript" src="/dhtml.js" type="text/JavaScript"></script>'
echo '<![endif]-->'
echo '</head>'

echo '<body>'
echo '<div id="page">'
#echo ' <img src="http://10.31.237.249/cabecera.jpg">'
#echo ' <img src="/cabecera.jpg">'
#echo ' <img src="/LogotipoIDATA_s.jpg">'

echo '<BR> <h3>'
hostname
#echo '    <font size="-1">'
echo '&nbsp; &nbsp; &nbsp; &nbsp;'
date +"%d.%m.%Y %T"
#echo '</font>'
echo ' </h3>'

echo '<hr> <div id="contenido">'
echo '<a href="/usr-cgi/ultimodato.sh" target="content">Ultimo Dato</a>'
echo '  '
echo '<a href="/usr-cgi/ultimodia.sh" target="content">Ultimo Dia</a>'
echo '  '
echo '<a href="/usr-cgi/ultimos3dias.sh" target="content">Ultimos 3 Dias</a>'
echo '  '
echo '<a href="/usr-cgi/ultimos10dias.sh" target="content">Ultimos 10 Dias</a>'
echo '  '
echo '<a href="/usr-cgi/supervision.sh" target="content">Seleccionar-Señal-Fecha</a>'
echo '</div>'

echo '<hr> <div id="contenido">'
echo Prueba
echo '</div>'
