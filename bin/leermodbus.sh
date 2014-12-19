while :; do
  # Todo lo que quieras repetir infinitas veces
    echo "consulta nivel"
	valor=`wget http://127.0.0.1/usr-cgi/luci/arduino/data/0,1,1,5,200,1,1 -O - -q`
	echo $valor
	valorHex=`python f2h.py $valor`
	/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 4054 -f 16 -q 2 -d $valorHex
    echo "consulta D 13"
	d13=`wget http://127.0.0.1/usr-cgi/luci/arduino/digital/13 -O - -q`
	echo $d13
	d13value=$(echo ${d13} | sed 's/.*\(..\)$/\1/')
	echo $d13value
	/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 0 -f 5 -q 1 -d $d13value
      sleep 1
      done
