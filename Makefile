#
#CROSSDIR=/datos/axotec/crossdev
#include $(CROSSDIR)/Rules.mak

CFLAGS_SHARED	= -g -O3 -fPIC
#LDFLAGS+= -L/datos/arduino/openwrt/trunk/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/lib/

all: yunlogger

yunlogger: info libidata.so axisloger ipcserver leerlogerbd cgi killpid deftablas sftpclient AxionMain leermodbus axismbus 

nfsaxplc: limpiar axplc
	cp -a bin /axoteclinux/nfs2target/
	cp libidata.so /axoteclinux/nfs2target/

nfsAxionMain: eclipse AxionMain
	cp -a bin/AxionMain /axoteclinux/nfs2target/bin/

nfsipcserver: eclipse ipcserver
	cp -a bin/ipcserver /axoteclinux/nfs2target/bin/

nfsaxisloger: axisloger
	cp -a bin/axisloger /axoteclinux/nfs2target/bin/

nfsleerlogerbd: leerlogerbd
	cp -a bin/leerlogerbd /mnt/SAICA/idata/bin/

eclipse:
	cp /home/adan/workspace/saica/src/* src/
#	cp /home/adan/workspace/saica/src/ipcserver.c src/

info:
	echo $(CFLAGS)
	echo $(CFLAGS_SHARED)
	echo $(LDFLAGS)

limpiar:
	rm -f *.o *.a *.so bin/* 

axplc: libidata.so axisloger ipcserver leerlogerbd cgi killpid deftablas sftpclient AxionMain

#enviaraxplc: axplc enviarlibidata enviarweb enviaraxislogos enviaripcserver enviaraxismbus

libidata.so: ./src/logersaihbd.c ./src/juliano.c ./src/IpcFun.c ./src/SigPro.c ./src/cgl.c ./src/UtilTty.c ./src/AxisLogerUtil.c ./src/commun.c ./src/mbs.c ./src/modbus_tcp_maestro.c 
	$(CC) $(CFLAGS_SHARED) -c ./src/logersaihbd.c
	$(CC) $(CFLAGS_SHARED) -c ./src/juliano.c
	$(CC) $(CFLAGS_SHARED) -c ./src/IpcFun.c
	$(CC) $(CFLAGS_SHARED) -c ./src/SigPro.c
	$(CC) $(CFLAGS_SHARED) -c ./src/cgl.c
	$(CC) $(CFLAGS_SHARED) -c ./src/UtilTty.c
	$(CC) $(CFLAGS_SHARED) -c ./src/AxisLogerUtil.c
	$(CC) $(CFLAGS_SHARED) -c ./src/commun.c
	$(CC) $(CFLAGS_SHARED) -c ./src/mbs.c
	$(CC) $(CFLAGS_SHARED) -c ./src/modbus_tcp_maestro.c
	$(CC) -o libidata.so logersaihbd.o juliano.o IpcFun.o SigPro.o cgl.o UtilTty.o AxisLogerUtil.o  mbs.o commun.o modbus_tcp_maestro.o -shared -lpthread -lm

axismbus: axismodbus.o libidata.so
	$(CC) axismodbus.o -o bin/axismbus -L. -lidata -lpthread

axismodbus.o: src/axismodbus.c src/pantalla.h
	$(CC) -c src/axismodbus.c 

leermodbus.o: src/leermodbus.c
	$(CC) $(CFLAGS) -c src/leermodbus.c

leermodbus: leermodbus.o libidata.so
	$(CC) $(CFLAGS) leermodbus.o -o bin/leermodbus -L. -lidata 

timejul.o: src/timejul.c
	$(CC) $(CLAGS) -c src/timejul.c

timejul: timejul.o
	$(CC) -o bin/$@ $@.o

sftpclient.o: src/sftpclient.c
	$(CC) $(CLAGS) -c src/sftpclient.c

sftpclient: sftpclient.o
	$(CC) -o bin/$@ $@.o

deftablas.o: src/deftablas.c
	$(CC) -c src/deftablas.c

deftablas: deftablas.o libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

AxionMain.o: src/AxionMain.c
	$(CC) -c src/AxionMain.c

AxionMain: AxionMain.o libidata.so
	$(CC) -o yunlogger $@.o -L. -lidata

axisloger.o: src/axisloger.c
	$(CC) -c src/axisloger.c

axisloger: axisloger.o libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

ipcserver.o: src/ipcserver.c
	$(CC) $(CFLAGS) -c src/ipcserver.c

ipcserver: libidata.so ipcserver.o 
	$(CC) -o bin/$@ $@.o -L. -lidata

killpid.o: src/killpid.c
	$(CC) $(CFLAGS) -c src/killpid.c

killpid: libidata.so killpid.o 
	$(CC) -o bin/$@ $@.o -L. -lidata

leerlogerbd.o: src/leerlogerbd.c
	$(CC) $(CFLAGS) -c src/leerlogerbd.c

leerlogerbd: leerlogerbd.o libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

leerlogerbdcgi.o: src/leerlogerbdcgi.c
	$(CC) $(CFLAGS) -c src/leerlogerbdcgi.c

leerlogerbdcgi: leerlogerbdcgi.o libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

datoscgi2.o: ./src/datoscgi2.c 
	$(CC) $(CFLAGS) -c ./src/datoscgi2.c

datoscgi2: datoscgi2.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

incidenciascgi.o: ./src/incidenciascgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/incidenciascgi.c

incidenciascgi: incidenciascgi.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

segjulhistcgi.o: ./src/segjulhistcgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/segjulhistcgi.c

escribirlogerbdcgi.o: ./src/escribirlogerbdcgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/escribirlogerbdcgi.c

escribirlogerbdcgi: escribirlogerbdcgi.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

analogicascgi.o: ./src/analogicascgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/analogicascgi.c

analogicascgi: analogicascgi.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

digitalescgi.o: ./src/digitalescgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/digitalescgi.c

digitalescgi: digitalescgi.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

escribiranalogicascgi.o: ./src/escribiranalogicascgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/escribiranalogicascgi.c

escribiranalogicascgi: escribiranalogicascgi.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

escribirdigitalescgi.o: ./src/escribirdigitalescgi.c ./src/cgi.h
	$(CC) $(CFLAGS) -c ./src/escribirdigitalescgi.c

escribirdigitalescgi: escribirdigitalescgi.o  libidata.so
	$(CC) -o bin/$@ $@.o -L. -lidata

cgi: leerlogerbdcgi escribirlogerbdcgi analogicascgi escribiranalogicascgi digitalescgi escribirdigitalescgi incidenciascgi datoscgi2




