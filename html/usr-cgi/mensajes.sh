#!/bin/sh
. ./inicio.sh


echo '<h2>Tipo Mensaje</h2>'

echo '<h5>Consulta del &uacute;ltimo mensaje recibido de cada categor&iacute;a</h5>'

echo '<br>'

echo '<form action=mensajescomando.sh method=POST>'

echo '<ul class="simple">'

echo '<li class="simple"><input type=radio CHECKED name="indiceMSG" value="0">SAC SENV: Mensaje de dimensi&oacute;n Se&ntilde;ales Anal&oacute;gicas de la Base de Datos</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="1">SAC CARPA: Mensaje de carga de FCM y FCA de las Se&ntilde;ales Anal&oacute;gicas</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="2">SAC CARDES: Mensaje de carga Descripci&oacute;n y Unidades de todas las Se&ntilde;ales Anal&oacute;gicas</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="3">SAC MASCAR: Mensaje de carga de M&aacute;scara de Anulaci&oacute;n de las Se&ntilde;ales Digitales</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="4">SAC CONCAR: Mensaje de carga de FCM y FCA de los 4 Contadores-Pluv</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="5">SAC DIGPET: Mensaje de petici&oacute;n del Estado Actual de las Se&ntilde;ales Digitales</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="6">SAC CARFOR: Mensaje de carga de las F&oacute;rmulas de las Se&ntilde;ales Calculadas</li>'
echo '<li class="simple"><input type=radio name="indiceMSG" value="7">SAC CARTAB: Mensaje de carga de las Tablas de las Se&ntilde;ales</li>'

echo '</ul>'

echo '<br>'

echo '<input class="botonc" type=submit value="Consultar Mensaje">'
echo '</form>'

./fin.sh /index_saih.html
