#!/bin/sh

# FALTAN mensajescomando.sh

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

echo ' <h1>'
hostname
echo ' </h1>'
echo ' <hr>'

echo '<ul id="navmenu-h">'
echo '<li><a href="#">Administraci&oacute;n Sistema</a>'
echo '<ul>'
echo '<li><a href="/admin-bin/editcgi.cgi?file=/">Explorar sistema de archivos</a></li>'
echo '<li><a href="/usr-cgi/fechahora.sh">Fecha-Hora</a></li>'
echo '<li><a href="/admin-bin/editcgi.cgi?file=/etc/conf.d/net.eth0">IP  Local</a></li>'
echo '<li><a href="/admin-bin/editcgi.cgi?file=/etc/conf.d/hostname">Nombre Axis</a></li>'
echo '<li><a href="/comp_ref.txt">Componentes</a></li>'
echo '<li><a href="/usr-cgi/kernellog.sh">System log</a></li>'
echo '<li><a href="/usr-cgi/tetralog.sh">TETRA log</a></li>'
echo '<li><a href="/usr-cgi/reiniciar.sh">Reiniciar Axis</a></li>'
echo '</ul>'
echo '</li>'
echo '<li><a href="#">Administraci&oacute;n Remota</a>'
echo '<ul>'
echo '<li><a href="/usr-cgi/supervision.sh">Supervisi&oacute;n QuinceMinutales</a></li>'
echo '<li><a href="/usr-cgi/incidencias.sh">Supervisi&oacute;n Incidencias</a></li>'
echo '<li><a href="/usr-cgi/mensajes.sh">Supervisi&oacute;n Mensajes</a></li>'
echo '<li><a href="/usr-cgi/configuracion.sh">Configuraci&oacute;n</a></li>'
echo '<li><a href="/usr-cgi/aplicaciones.sh">Aplicaciones</a></li>'
echo '<li><a href="/usr-cgi/axislog.sh">Log Aplicaci&oacute;n</a></li>'
echo '<li><a href="/usr-cgi/reiniciarbd.sh">Iniciar Bases Datos</a></li>'
echo '<li><a href="/usr-cgi/imagenes.sh">Im&aacute;genes</a></li>'
echo '<li><a href="/usr-cgi/estadistica.sh">Estad&iacute;stica Com. Axis-Remota</a></li>'
echo '<li><a href="/usr-cgi/graficas.sh">Gr&aacute;ficas</a></li>'
echo '</ul>'
echo '</li>'
echo '</ul>'

echo ' <hr>'

echo ' <div id="contenido">'

