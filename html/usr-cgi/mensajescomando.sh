#!/bin/sh

. ./inicio.sh

eval `./proccgi.sh $*`
./leermensajescgi $FORM_indiceMSG

./fin.sh /usr-cgi/mensajes.sh
