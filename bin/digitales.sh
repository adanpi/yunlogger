echo "consulta Digitales 10 - 12"
d13=`wget http://127.0.0.1/usr-cgi/luci/arduino/digital/-1 -O - -q`
echo $d13
/radsys/pollmb/pollmb.py -a 0 -f 15 -d $d13 -q 3
