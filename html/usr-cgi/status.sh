#!/bin/sh
. /www/usr-cgi/inicioLigero.sh

$SAIHBD/bin/leerlogerbd S 
echo '<form action=resetultimoenvioftp.sh method=POST>'
echo '<input class="boton" type=submit value="Resetear Ultimo Envio FTP"></form>'

. /www/usr-cgi/aplicaciones.sh

. /www/usr-cgi/reiniciarbd.sh

/www/usr-cgi/fin.sh /usr-cgi/inicioLigero.sh
