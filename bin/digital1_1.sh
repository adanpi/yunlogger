# se ejecuta al activar D 1
valor=180
echo $valor
valorHex=`python f2h.py $valor`
/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 1054 -f 16 -q 2 -d $valorHex
