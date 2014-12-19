echo "consulta MB"
valor=`python /radsys/pollmb/pollmb.py -h 127.0.0.1 -a 4054 -f 4 -q 6`
echo $valor
datos=`echo -n $valor | tail -c 24`
echo $datos
datos1=`echo ${datos:0:8}`
echo $datos1
valor1=`python h2f.py $datos1`
echo $valor1
echo $valor1 > /tmp/yunlogger.4
datos2=`echo ${datos:8:8}`
echo $datos2
valor2=`python h2f.py $datos2`
echo $valor2
echo $valor2 > /tmp/yunlogger.5
datos3=`echo ${datos:16:8}`
echo $datos3
valor3=`python h2f.py $datos3`
echo $valor3
echo $valor3 > /tmp/yunlogger.6
