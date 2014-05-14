#!/bin/sh
echo Content-Type: text/html
echo Expires: Tue, 17 Jun 1979 02:20:00
echo Expires: 0
echo Pragma: no-cache
echo
echo '<html>'

echo '<body BGCOLOR="#FFFFFF">'
echo '<TABLE BORDER="0" WIDTH="900px" CELLSPACING="0" CELLPADDING="0">'
echo '<TR><TD ALIGN="center" VALIGN="center" HEIGHT="91" BGCOLOR="#ffd700">'
echo '<A HREF="http://www.chebro.es"><H2>CONFEDERACION HIDROGRAFICA DEL EBRO</H2></a></TD>'
echo '<TD VALIGN="CENTER" ALIGN="CENTER" WIDTH="50%" HEIGHT="91" BGCOLOR="#ffd700">'
echo '<A HREF="http://195.55.247.237/saihebro/"><H1>SAIH</H1></A></TD>'

echo '<TD VALIGN="MIDDLE" ALIGN="RIGHT" BGCOLOR="#d1d1d1"> </TD></TR>'
echo '</TABLE>' 
echo '<br><CENTER><hr>'
while read linea
do
        echo "<A href=/img/$linea> $linea </A><BR>"

done < /tmp/img.list

echo '</CENTER><br><hr>'
#echo '<br><br><br><CENTER>'
#echo '<img src="/pics/ute_ofi.jpg">'
#echo '</CENTER><br>'

echo '<form action=tomarimagencgi method=POST>'
echo '<input type=hidden value="reboot" name="apl">'
echo '<input type=submit value="Tomar nueva imagen">'
echo '</form>'

echo '<br><br><hr><a href="/index.html">Volver</a>'
echo '</body></html>'
