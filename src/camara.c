// AXIS - MAIN
// Axis ine Axion
/*
*       Prometeo
*       FILE:           axis.c
*       AUTHOR:         M.Bibudis
*       DATE:           18-05-2005
*       REVISION:       1.0
*       PRODUCT:        Axis control de Procesos
*       O.S.:           LINUX - AXIS ine Axion
*       CUSTOMER:       SAIH
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include "logersaihbd.h"
#
int Pidlog();
#//////i
int main(int argc, char *argv[])
{
	short i,j,ipid;
	unsigned long segjulact,segjulqm;
	struct tm *newtime;
	char *auxch;
	char name[92],aux[92];

	strcpy(name,"camara.pid");				// PID del Proceso
	ipid=getpid();
	PidLog(name,ipid);

	segjulact=time(NULL);                                   // Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);

	sprintf(aux,"Proceso Camara Iniciado:Pid=%d %s",ipid,auxch);
	AxisLog(aux);						// Log
	// signal(-,SIG_IGN);

	for(;;){  

		strcpy(name,"/mnt/flash/webcam/imagen.sh > $SAIHBD/log/camara.log");
		system(name);

		TimeWait(3000);					// TimeOut
	}
	exit(0);
}
