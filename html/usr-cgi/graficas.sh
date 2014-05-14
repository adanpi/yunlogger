#!/bin/sh
SAIHBD=/radsys/
export SAIHBD
HTML_HOME=/www/
export HTML_HOME
echo Content-Type: text/html
echo Cache-Control: no-cache
echo Expires: -1
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

echo '<frameset rows="400,*">'
echo '<frame name="marcoGrafica" src="/usr-cgi/marcoGrafica.sh">'
echo '<frame name="marcoSeleccionGrafica" src="/usr-cgi/seleccionGrafica.sh">'
echo '<noframes>'
echo 'Se requiere un navegador con soporte para Frames'
echo '</noframes>'
echo '</frameset>'
echo '</html>'
