
// Comunicaciones RS232 AXIS - REMOTA SAC
// Axis ine Axion: Comunicacion Entre Procesos
/*
*       Prometeo
*       FILE:           SigPro.c
*       AUTHOR:         M.Bibudis
*       DATE:           01-11-2005
*       REVISION:       1.0
*       PRODUCT:        Axis Termina Remoto
*       O.S.:           LINUX - AXIS ine Axion
*       CUSTOMER:       SAIH
*/

// Signal To Process
// Comunicaciones RS232 AXIS - REMOTA SAC
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include "ipcserver.h"
//#include "logersaihbd.h"
//#include "axisloger.h"    // include AxisSerialUtils
#
int PidLog();
//GN gn;
//BDCONF BdConf;
FILE *fhgn;
short ihw;

// Control de Procesos

void KillPro()
{
        short i,sig;
        char path[80];
        char aux[64];
        long segjulact;
        struct tm *newtime;
        char *auxch;


        signal(SIGINT,SIG_IGN);
        signal(SIGTERM,SIG_DFL);
        signal(SIGHUP,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
        signal(SIGKILL,SIG_DFL);
        //signal(SIGPWR,SIG_IGN);                               // Power fallure


        printf("\n\tProceso Parado:PID=%d :\n",getpid());

        segjulact=time(NULL);                                   // Hora Actual
        newtime=localtime(&segjulact);
        auxch=asctime(newtime);


        if(i=kill(getpid(),SIGTERM) !=0){
                kill(getpid(),SIGKILL);
		sig=SIGKILL;
	}else
		sig=SIGTERM;

        sprintf(aux,"Pid:%d Proceso Terminado:%s (SIG %d)",getpid(),auxch,sig);
        AxisLog(aux);
        printf("\n");
        exit(0);
}

// Control de Procesos
void KillVTerm()
{
        short i;
        char path[80];
        char aux[64];
        long segjulact;
        struct tm *newtime;
        char *auxch;


        signal(SIGINT,SIG_IGN);
        signal(SIGTERM,SIG_IGN);
        signal(SIGHUP,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
        signal(SIGKILL,SIG_IGN);
        //signal(SIGPWR,SIG_IGN);                               // Power fallure


        printf("\n\tProceso SacVTerm Parado:PID=%d :\n",getpid());

        segjulact=time(NULL);                                   // Hora Actual
        newtime=localtime(&segjulact);
        auxch=asctime(newtime);

        sprintf(aux,"Proceso: SacVTerm Terminado:%s",auxch);
        AxisLog(aux);

        if(i=kill(getpid(),SIGTERM) !=0)
                kill(getpid(),SIGKILL);

        printf("\n");
        exit(0);
}

// Recibo de Signal.
void SigGen(numsig)
int numsig;
{
        short i;
        char path[80];
        long qmactual;
        struct tm *newtime;
        char *auxch;

	
        switch(numsig){
            case 1:
		printf("\n\n\tAxis Kalimera... Signal Recibido:%d Time=%ld",numsig,time(NULL));
		signal(numsig,SIG_IGN);
	    break;
            case 2:
		printf("\n\n\tAxis Kalimera... Signal Recibido:%d Time=%ld",numsig,time(NULL));
		signal(numsig,SIG_IGN);
	    break;

	    default:
		printf("\n\n\tAxis Kalimera Signal No Tratado:%d Time=%ld",numsig,time(NULL));
	    break;
	}
}

// Recibo de Signal SIGUSR1 ... QM Actual
void SigUsr1()
{
        short i;
        long ifecha,qmactual;
        struct tm *newtime;
        char *auxch;

        signal(SIGUSR1,SIG_IGN);
        printf("\n\n\tAxisSac Kalimera... Signal SIGUSR1 Recibido:Pid=%d",getpid());
        TimeWait(20);

}

// Recibo de Signal SIGUSR2 ... Ipnos.
void SigUsr2(timeout)
int timeout;
{
        short i;
        char path[80];
        long qmactual;
        struct tm *newtime;
        char *auxch;

        printf("\n\n\tAxis Kalimera... Signal SIGUSR2 Recibido:Time=%ld",time(NULL));
        signal(SIGUSR2,SIG_IGN);
        TimeWait(timeout);
        signal(SIGUSR2,SigUsr2);
        printf("\n\n\tAxis Kalimera... Signal SIGUSR2 Terminado:Time=%ld",time(NULL));
}

EnviarSignal(proceso,senal)
char proceso[22];
char senal[22];
{
        int i,ipid;
        char path[80];
        char ch[7];
        FILE *fhp;
        char name[20],aux[64],chaux[12];
        unsigned long segjulact,segjulqm;

        memset((char *)ch,0,7);
        ipid=99000;

        if( (char *)getenv("SAIHBD") ==NULL){
                printf("\n\tPidLog:Variable Entorno SAIHBD NO SET");
                exit(1);}
        strcat(strcpy(path,(char *)getenv("SAIHBD")),"log/");      // Parametros Generales
        strcat(path,proceso);
        strcat(path,".pid");

        if((fhp=fopen(path,"r"))==NULL){
                printf("\nPidLog:No se puede abrir:%s",path);
                exit(1);}
        while(fgets(ch,7,fhp)){
            if(sscanf(ch,"%d",&ipid)==1) break;
        }
        fclose(fhp);

        if(i=kill(ipid,senal) == 0){
                printf("\n\tProceso:PID=%d ENVIADO SENAL:%s\n",ipid,senal);
        }
        else{
                printf("\n\tProceso:PID=%d NO ENVIADO SENAL:%s\n",ipid,senal);
        }

        //TimeWait(20);

}

