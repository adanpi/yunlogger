/*
*       S.A.C.
*       FILE:           ClntTetra.c
*       AUTHOR:         M.Bibudis@Enero-00
*       DATE:           01-02-05
*       REVISION:       1.0
*       PRODUCT:        IPC
*       SUBJECTS:       ipc services
*       O.S.:           LINUX-AXIS ine Axion
*       CUSTOMER:       SAIH
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include "ipcserver.h"
#include "srvipc_shm.h"
int msg_datquin();
FILE *fh;

main(argc, argv)
int argc;
char *argv[];
{
	long sjul,ltime;
	int i,j,k,x;
	long segjul;
	short ciclico,origen,ihw;
	struct tm *newtime;

	char *auxch;


	if (argc > 1) exit(2);
	ciclico=50;
	origen=44;
	ihw=77;
	for(;;){
		ltime=time(NULL);
		segjul=time(NULL);
		segjul=(segjul/SEGPQM)*SEGPQM;
		segjul=segjul-2*SEGPQM;
                newtime=localtime(&segjul);
                auxch=asctime(newtime);

		segjul=htonl(segjul);

		printf("\n\tClienteTetra:Connect:Remota=%hd FechaQM=%d %s",ihw,segjul,auxch);
		if((i=msg_datquin(ciclico,origen,ihw,segjul,"192.168.1.173",CNCL))<=0)
			fprintf(stderr,"\n\tClienteTetra:Error_Conexion_Axis = %d Fecha=%d",i,ltime);
		ciclico++;
		sleep(60);
	}
}
