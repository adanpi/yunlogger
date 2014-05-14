#!/bin/sh
. /www/usr-cgi/inicioLigero.sh
PATH=/sbin:/usr/sbin:$PATH:$SAIHBD/bin
export PATH

eval `/www/usr-cgi/proccgi.sh $*`
echo '<h3>'
echo 'Lanzando aplicacion '
echo $FORM_apl
echo '</h3>'
echo '<br>'

echo '<textarea>'
echo $FORM_apl
$FORM_apl
echo $PATH
echo '	-------- HECHO --------'
echo
echo '</textarea>'


/www/usr-cgi/fin.sh /usr-cgi/procesos.sh
