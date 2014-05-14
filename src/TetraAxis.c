/*
*       S.A.C.
*       FILE:           TetraAxis.c
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
#include "TetraAxis.h"
#include "ipcserver.h"
#include "srvipc_shm.h"
int msg_datquin();
FILE *fh;
REMOTAS remotas;
main(argc, argv)
int argc;
char *argv[];
{
	int i,j,k,x,n;
	int flag,pid;
	unsigned long segjul,segjulact,segjulqm,qmactual;
	unsigned long julact,julqm,iqmactual;
	short ciclico,origen,ihw,ipid;
	struct tm *newtime;
	char ip[16];
	char *auxch;
	char name[5];
	char aux[64];

	strcpy(name,"TetraAxis.pid");				// PID del Proceso
	ipid=getpid();
	PidLog(name,ipid);

	segjulact=time(NULL);                                   // Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);

	sprintf(aux,"%s %s","Proceso TetraAxis Iniciado: ",auxch);
	AxisLog(aux);	
	if (argc > 1) exit(2);
	ciclico=50;
	origen=44;

	signal(SIGCLD,SIG_IGN);

	strcpy(remotas[0].ip,"192.168.1.174");		// Axis-A216
//	strcpy(remotas[0].ip,"10.8.12.148");		// Axis-A216
	strcpy(remotas[1].ip,"10.8.12.147");		// A089
	strcpy(remotas[2].ip,"10.8.12.136");		// E286
	strcpy(remotas[3].ip,"10.8.12.119");		// C040
	remotas[0].ihw=77;
	remotas[1].ihw=147;
	remotas[2].ihw=136;
	remotas[3].ihw=119;
	strcpy(remotas[0].name,"pr03");
	strcpy(remotas[1].name,"a089");
	strcpy(remotas[2].name,"e286");
	strcpy(remotas[3].name,"c040");
	for(;;){
		segjulact=time(NULL);                                   // Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		printf("\n\tTetraAxis:Proceso Padre:Pid=%d :FECHA ACTUAL = %ld  %s",getpid(),segjulact,auxch);
		sleep(5);

		segjulqm=segjulact;                                     // Fecha QM desde epoca=1970
		segjulqm=((segjulqm/SEGPQM)*SEGPQM);
		segjulqm=segjulqm - SEGJULCTE;
		qmactual=segjulqm - (8 * SEGPQM) - SEGPQM;				// QM
		iqmactual=qmactual;
		qmactual=htonl(qmactual);

		if( segjulact <= segjulqm +5) flag=1;			// Flag = 1 Para pedir QM Actual, Incidecnias
		flag=1;

		if( (segjulact > segjulqm +5 ) && (flag==1)){           // QM Actual, Incidencias
			flag=0;
			for(i=0;i<1;i++){			
				segjul=time(NULL);
	 			if((pid=fork())==-1){
					printf("\n\tTetraAxis:Error en Pid=%d",errno);}
				else if(pid==0){
					signal(SIGCLD,SIG_IGN);
					strcpy(name,remotas[i].name);
					ihw=remotas[i].ihw;
					strcpy(ip,remotas[i].ip);
					remotas[i].gn.NumComSac[0]++;		// NumComTotal
					for(n=0;n<10;n++){
						printf("\n\tTetraAxis:ProcesoHijo (%d):%d Remota=%hd FechaQm=%ld %s",i,getpid(),ihw,iqmactual,auxch);
						if( (i=msg_datquin(name,ciclico,origen,ihw,qmactual,ip,CNCL)) < 0){
							fprintf(stderr,"\n\tTetraAxis:Error_Conexion_Axis = %d Fecha=%ld",i,segjulact);}
						ciclico++;
						if(i > 0){
							remotas[i].gn.NumComSac[0]++;		// NumComBien
							break;}
                                		julact=time(NULL);
                                		julqm=((julact/SEGPQM)*SEGPQM);
                                		if(julact > julqm + 100) break;
						TimeWait(20);
						continue;
					}
					exit(0);
				}
				else if(pid){
					printf("\n\tTetraAxis:Creado Proceso (%d) :",i);
				}
			}
			printf("\n\tTetraAxis:Proceso Padre:Fin Crear Procesos:Fecha=%ld",time(NULL));
		}
		sleep(60);
	}
}
