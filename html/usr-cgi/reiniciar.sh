
echo '<h3>Reiniciar AXPLC</h3>'

echo '<h5>Atenci&oacute;n: si se reinicia el AXPLC, se perder&aacute; la conexi&oacute;n. Deber&aacute; esperar unos minutos para conectar de nuevo</h5>'
echo '<br>'

echo '<form action=lanzarproceso.sh method=POST>'
echo '<input type=hidden value="reboot" name="apl">'
echo '<input class="botond" type=submit value="Reiniciar">'
echo '</form>'

