/*
*       Prometeo
*       File:           KillPid.c
*       Autor:          M.Bibudis
*       Fecha:          01/04/2005
*       Revision:       1.0
*       Producto:       Axis ine Axion
*       Objetivo:       Kosmos...
*       Customer:       SAIH
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include "logersaihbd.h"
//Kill PID de un Proceso
main(argc,argv)
char *argv[];
int argc;
{
        short i;
        int ipid,numsig;
        char path[80];
        char ch[7];
        FILE *fhp;
	char name[20],aux[64],chaux[12];
	unsigned long segjulact,segjulqm;
        struct tm *newtime;
        char *auxch;

	if ( argc <2 || argc >3){
		printf("\n\tParametros:killpid name numsig\n");
		exit(0);}

	segjulact=time(NULL);                                   // Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	sprintf(aux,"Proceso Terminado:%s  %s",argv[1],auxch);

	if(argc==3){
		if(sscanf(argv[2],"%02d",&numsig)!=1){
			printf("\n\tParametros:killpid name numsig\n");
			exit(0);}
	}
	else
		numsig=15;

        memset((char *)ch,0,7);
        ipid=99000;

	strcpy(chaux,argv[1]);
	if(!strncmp(chaux,"pppd",4)){
		strcpy(path,"/var/run/ppp0.pid");
	}
        else{
		if( (char *)getenv("SAIHBD") ==NULL){
                	printf("\n\tPidLog:Variable Entorno SAIHBD NO SET");
                	exit(1);}
        	strcat(strcpy(path,(char *)getenv("SAIHBD")),"log/");      // Parametros Generales
        	strcat(path,argv[1]);
        	strcat(path,".pid");
	}

        if((fhp=fopen(path,"r"))==NULL){
                printf("\nPidLog:No se puede abrir:%s",path);
                exit(1);}
        while(fgets(ch,7,fhp)){
            if(sscanf(ch,"%d",&ipid)==1) break;
        }
        fclose(fhp);

        if(ipid < 50 || ipid >99999){
		printf("\n\tPID No Permitido\n");
                exit(1);}

        //printf("\n\tpath=%s ch=%s PID=%d",path,ch,ipid);

	if(numsig==9){         
		if(i=kill(ipid,SIGKILL) == 0){
			printf("\n\tPID=%d Terminado\n",ipid);
			AxisLog(aux);
			exit(0);}
		else
			printf("\n\tPID=%d No Terminado\n",ipid);
	}
	else if(numsig==15){         
		if(i=kill(ipid,SIGTERM) == 0){
			printf("\n\tPID=%d Terminado\n",ipid);
			exit(0);}
		else
			printf("\n\tPID=%d No Terminado\n",ipid);
	}
	else if(numsig==10){         
		if(i=kill(ipid,SIGUSR1) == 0){
			printf("\n\tPID=%d SIGUSR1 ENVIADO\n",ipid);
			exit(0);}
		else
			printf("\n\tPID=%d SIGUSR1 NO ENVIADO\n",ipid);
	}
	else if(numsig==12){         
		if(i=kill(ipid,SIGUSR2) == 0){
			printf("\n\tPID=%d SIGUSR2 ENVIADO\n",ipid);
			exit(0);}
		else
			printf("\n\tPID=%d SIGUSR2 NO ENVIADO\n",ipid);
	}
	else{
		printf("\n\tSignal No Permitido\n");

	}
	exit(0);
}
