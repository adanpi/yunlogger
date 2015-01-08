#control de servo motor en pin 9
#http://yun/usr-cgi/luci/arduino/servo/9/180
echo "consulta MB"
valor=`python /radsys/pollmb/pollmb.py -h 127.0.0.1 -a 1054 -f 4 -q 2`
echo $valor
datos=`echo -n $valor | tail -c 8`
echo $datos
valor1=`python h2f.py $datos`
echo nuevo: $valor1 FIN
# si ha cambiado el valor se manda al servo
valor_anterior=`cat /tmp/servo.anterior`
echo anterior: $valor_anterior FIN
if [ "$valor1" != "$valor_anterior" ]
then 
	echo "wget http://localhost/usr-cgi/luci/arduino/servo/9/$valor1 -O - -q -T 1 > /tmp/servo" 
	wget http://localhost/usr-cgi/luci/arduino/servo/9/$valor1 -O - -q -T 1 > /tmp/servo
	echo $valor1 > /tmp/servo.anterior
else 
	echo "no cambio" 
fi
#wget http://localhost/usr-cgi/luci/arduino/servo/9/$valor1 -O - -q -T 1 > /tmp/servo
#echo $valor1 > /tmp/servo.anterior
