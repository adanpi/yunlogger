#!/bin/sh
eval `./proccgi.sh $*`

. ./inicio.sh

echo '<div id="flasharea">'

echo '<object classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="http://fpdownload.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=8,0,0,0" width="100%" height="100%" id="graph-2" align="middle">'
echo '<param name="allowScriptAccess" value="always" />'
#echo '<param name="movie" value="http://10.31.236.40/open-flash-chart.swf?data=/usr-cgi/datos.sh%3Fsen%3D'$FORM_sen'%26num%3D'$FORM_num'%26tipo%3D'$FORM_tipo'%26ing%3D'$FORM_ing'%26anio%3D'$FORM_anio'%26mes%3D'$FORM_mes'%26dia%3D'$FORM_dia'%26hora%3D'$FORM_hora'%26min%3D'$FORM_min'" />'
echo '<param name="movie" value="/open-flash-chart.swf?data=/usr-cgi/datos.sh%3Fsen%3D'$FORM_sen'%26num%3D'$FORM_num'%26tipo%3D'$FORM_tipo'%26ing%3D'$FORM_ing'%26anio%3D'$FORM_anio'%26mes%3D'$FORM_mes'%26dia%3D'$FORM_dia'%26hora%3D'$FORM_hora'%26min%3D'$FORM_min'" quality="high" bgcolor="#FFFFFF" width="100%" height="100%" name="open-flash-chart" align="middle" allowScriptAccess="always" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" />'

echo '<param name="quality" value="high" />'
echo '<param name="bgcolor" value="#FFFFFF" />'
#echo '<embed src="http://10.31.236.40/open-flash-chart.swf?data=/usr-cgi/datos.sh%3Fsen%3D'$FORM_sen'%26num%3D'$FORM_num'%26tipo%3D'$FORM_tipo'%26ing%3D'$FORM_ing'%26anio%3D'$FORM_anio'%26mes%3D'$FORM_mes'%26dia%3D'$FORM_dia'%26hora%3D'$FORM_hora'%26min%3D'$FORM_min'" quality="high" bgcolor="#FFFFFF" width="100%" height="100%" name="open-flash-chart" align="middle" allowScriptAccess="always" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" />'
echo '<embed src="/open-flash-chart.swf?data=/usr-cgi/datos.sh%3Fsen%3D'$FORM_sen'%26num%3D'$FORM_num'%26tipo%3D'$FORM_tipo'%26ing%3D'$FORM_ing'%26anio%3D'$FORM_anio'%26mes%3D'$FORM_mes'%26dia%3D'$FORM_dia'%26hora%3D'$FORM_hora'%26min%3D'$FORM_min'" quality="high" bgcolor="#FFFFFF" width="100%" height="100%" name="open-flash-chart" align="middle" allowScriptAccess="always" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" />'

echo '</object>'

echo '</div>'

./fin.sh /usr-cgi/graficas.sh

