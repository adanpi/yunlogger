/*
*       Prometeo
*       FILE:           AxisLoger.c
*       AUTHOR:         M.Bibudis & Adan
*       DATE:           01-10-05
*       REVISION:       1.0
*       PRODUCT:        AxisDataLoger
*       SUBJECTS:
*       O.S.:           LINUX ine Axion
*       CUSTOMER:       SAIH
*
*              Modificaciones:
*                      02/06/2011      Irrecuperables marcar flag como N
*
*/

// Comunicaciones RS232 AXIS - DATALOGER

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <netdb.h>
#include "ipcserver.h"
#include "logersaihbd.h"
#include "axisloger.h"

//extern int h_errno;
void SigUsr1();
void SigUsr2();
void KillPro();
int Pidlog();
GN gn;
BDCONF BdConf;
FILE *fhgn;
short ihw;
static char *version = "1.0";

int main(int argc, char *argv[])
{
	short i,j,n,ipid;
	char name[15];
	char aux[64];
	long ifecha;
	unsigned long segjulact,segjulqm,julact,julqm;
	struct tm *newtime;
	char *auxch;
	unsigned long qmactual;
	//unsigned long IndActAna,IndActDig;
	long IndActAna,IndActDig;
	static short flag,FlagIrrec;
	char path[80];
	QM lqm;

	if ( (argc==2) && (argv[1][1]=='v')){
		printf("\n************************************");
		printf("\n\t radsys.es");
		printf("\n\t axisloger Version: %s",version);
		printf("\n************************************\n");
		return(0);
	}

	if ( (argc==2) && (argv[1][1]=='i')){
		printf("\n************************************");
		printf("\n\t radsys.es");
		printf("\n\t axisloger Version: %s",version);
		printf("\n\t iniciar datos analogicas");
		printf("\n************************************\n");
		IniDatosAna();
		return(0);
	}

	strcpy(name,"axisloger.pid");				// PID del Proceso
	ipid=getpid();
	PidLog(name,ipid);

	IniLogerBd();						// Bases De Datos

	segjulact=time(NULL);                                   // Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);

	sprintf(aux,"Proceso AxisLoger Iniciado:%s",auxch);
	AxisLog(aux);						// Log

	//printf("\nAXIS_DATALOGER:Errno=%d",errno);

	signal(SIGUSR1,SigUsr1);				// User Signal_1
	signal(SIGUSR2,SigUsr2);				// User Signal_2
	signal(SIGINT,KillPro);
	signal(SIGKILL,KillPro);
	signal(SIGTERM,KillPro);
	//signal(SIGPWR,KillPro);				// Power fallure

	flag=1;							// al inicio programa pedimos ultimo Dato
	FlagIrrec=0;
	segjulact=time(NULL);					// Hora Actual

	memset((char *)&gn,0,sizeof(GN));
/*	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerGN:Variable Entorno SAIHBD NO SET");
		return(-1);}
	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));      // Parametros Generales
	if((fhgn=fopen(path,"r+b"))==NULL){
		printf("\nReadLogerGN:No se puede abrir:%s errno=%d",path,errno);
		return(-1);}
	if(i=fread(&gn,sizeof(GN),1,fhgn)<=0){
		printf("\nReadLogerGN:Error_En_read:%s Errn=%d",path,errno);
		fclose(fhgn);
		return(-1);}
	fclose(fhgn);
*/

        if( (i=ReadLogerGn(&gn)) !=0)
              printf("\n\tReadLogerGn:Error=%d",i);

	if(gn.NumComLoger[0]==0) gn.NumComLoger[0]=1;
	if(gn.NumComLoger[1]==0) gn.NumComLoger[1]=1;

	if( (i=WriteLogerGn(gn)) !=0)
		printf("\n\tWriteLogerGn:Error=%d",i);

	for(;;){   
		if(gn.NumComLoger[0] > 1000){
			gn.NumComLoger[0]=5;
			gn.NumComLoger[1]=5;}

		segjulact=time(NULL);					// Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		printf("\n\tAXIS_DATALOGER:FECHA ACTUAL = %ld  %s IRREC:%d",segjulact,auxch,FlagIrrec);

		segjulqm=segjulact;					// Fecha QM desde epoca=1970
		segjulqm=((segjulqm/SEGPQM)*SEGPQM);
		qmactual=segjulqm;					// QM

		if( (segjulact <= segjulqm +5) && (segjulact >= segjulqm) ){				// Flag = 1 Para pedir QM Actual, Incidencias
			flag=1;
		}

		if( (segjulact > segjulqm +5 ) && (flag==1)){ 		// QM Actual, Incidencias
			flag=0;	
			IndActAna=0;					// Indice Actual = 0 (Peticion ultimo Registro)



        	if( (i=ReadLogerGn(&gn)) !=0)
	              printf("\n\tReadLogerGn:Error=%d",i);


			printf("\n\tAXIS_DATALOGER: Definicion Tablas QM (%04x) DIG (%04x)",gn.SigQm,gn.SigDig);
			for(n=0;n<50;n++){
				printf("\n\tQM ACTUAL:Numero Intentos=%d",n);
				gn.NumComLoger[0]++;					// NumComTotales
				if( (IndActAna=ActualizaQm(0)) >= 0){		// Ultimo QM
					printf("\n\tIndActAna Final=%lu SegJul:%lu",IndActAna,segjulqm);
					gn.IndActAna=IndActAna;					
					gn.segjulhis=segjulqm-SEGPQM;		// Ultimo Juliano Recibido apl web
					gn.NumComLoger[1]++;
					    if ( ((IndActAna - gn.IndHisAna) > NUMHISTQM) ){		// Al iniciar sincronizamos indice historico
						gn.IndHisAna = IndActAna - NUMHISTQM;
						if (gn.IndHisAna < 0) gn.IndHisAna=0;
						sprintf(aux,"\n\tAxisLoger :IndAct-IndHis>NUMHISTQM: IndActAna:%lu gn.IndHisAna:%lu",IndActAna,gn.IndHisAna);
    						AxisLog(aux);		// Log
					    }
					    if (gn.IndHisAna > IndActAna){	// En caso de que el indice historico sea mayor que el actual
						if ( IndActAna < NUMHISTQM ) gn.IndHisAna=0;
						else gn.IndHisAna = IndActAna - NUMHISTQM;
						//printf("\n\ndebug: gn.IndHisAna: %d = IndActAna: %lu - NUMHISTQM: %lu",gn.IndHisAna,IndActAna,NUMHISTQM);
						sprintf(aux,"\n\tAxisLoger :IndAct < IndHis: IndActAna:%lu gn.IndHisAna:%lu",IndActAna,gn.IndHisAna);
    						AxisLog(aux);		// Log			    
						printf("\n\t\t %s",aux);
					    }
					break;}		// NumComBien
				if(IndActAna==-3){
				 gn.NumComLoger[1]++;	// NumComBien pero fallo en recuperacion QM
 				 gn.IndActAna=0;
				 continue;}				

				if(IndActAna==-1) 	// No hay respuesta Loger
				 continue;
				
				julact=time(NULL);
				julqm=((julact/SEGPQM)*SEGPQM);
				if(julact > julqm + 200) break;
			}

			// Tres intentos sincronizar Dataloger
			for(n=0;n<3;n++){
				printf("\n\tAXIS_DATALOGER: TIME : Intento %d",n);
				gn.NumComLoger[0]++;				// NumComTotales
				if( (i=ActualizaTime()) >= 0 ){
					gn.NumComLoger[1]++;			// NumComBien
					break;
				}
				else
					printf("\n\t\t Fallo Actualiza Time: error %d intento:%d",i,n);
			}

			gn.NumComLoger[0]++;				// NumComTotales
			if( (IndActDig=ActualizaIn(gn.IndHisDig)) >= 0){
				gn.IndHisDig=IndActDig;
				printf("\n\tgn.IndHisDig Final=%d",gn.IndHisDig);
				gn.NumComLoger[1]++;}

			gn.NumComLoger[0]++;				// NumComTotales
			if( (IndActDig=ActualizaIn(gn.IndHisDig)) >= 0){
				gn.IndHisDig=IndActDig;
				printf("\n\tgn.IndHisDig Final=%d",gn.IndHisDig);
				gn.NumComLoger[1]++;}

			if( (i=WriteLogerGn(gn)) !=0)
				printf("\n\tWriteLogerGn:Error=%d",i);

			continue;
		}

		// Recupera QM Historicos
		//printf("\nsegjulact %d segjulqm %d Flag: %d gn.IndHisAna: %d gn.IndActAna: %d ",segjulact,segjulqm,flag,(gn.IndHisAna+1), gn.IndActAna);
		if( (segjulact > segjulqm +5 ) && (flag==0) && (segjulact < segjulqm +890) && ((gn.IndHisAna+1) < gn.IndActAna) ){
			printf("\n\tAXIS-DATALOGER:RECUPERA QUINCEMINUTALES:");
			memset((char *)&lqm,0,sizeof(QM));
			IndActAna=-3;

				gn.NumComLoger[0]++;					// NumComTotales
				if( (IndActAna=ActualizaQm(gn.IndHisAna+1)) >= 0){	// QM Recuperado
					printf("\n\tIndHisAna: %lu\tIndActAna:%d\n",gn.IndHisAna+1,IndActAna);
					gn.IndHisAna=IndActAna;
					gn.NumComLoger[1]++;}				// NumComBien
					
				//if(IndActAna == -3) gn.NumComLoger[1]++;		// NumComBien Status QM 0,1,256
				if( (IndActAna == -5) || (IndActAna == -3) ){
					gn.NumComLoger[1]++;
					gn.IndHisAna=gn.IndHisAna+1;
					printf("\n\tError Recuperacion IndHisAna: %lu\n",gn.IndHisAna);
				}
				if( (i=WriteLogerGn(gn)) != 0)
					printf("\n\tWriteLogerGn:Error=%d",i);

		}
/*		// Recupera IN Historicos
		if( (segjulact > segjulqm +180 ) && (flag==0) && (segjulact < segjulqm +240) ){
			printf("\n\tAXIS-DATALOGER:RECUPERA INCIDENCIAS:");
			gn.NumComLoger[0]++;				// NumComTotales
			if( (IndActDig=ActualizaIn(gn.IndHisDig)) >= 0){
				gn.IndHisDig=IndActDig;
				printf("\n\tgn.IndHisDig Final=%d",gn.IndHisDig);
				gn.NumComLoger[1]++;}
			if( (i=WriteLogerGn(gn)) != 0)
				printf("\n\tWriteLogerGn:Error=%d",i);
		}
		if( (segjulact > segjulqm +450 ) && (flag==0) && (segjulact < segjulqm +510) ){
			printf("\n\tAXIS-DATALOGER:RECUPERA INCIDENCIAS:");
			gn.NumComLoger[0]++;				// NumComTotales
			if( (IndActDig=ActualizaIn(gn.IndHisDig)) >= 0){
				gn.IndHisDig=IndActDig;
				printf("\n\tgn.IndHisDig Final=%d",gn.IndHisDig);
				gn.NumComLoger[1]++;}
			if( (i=WriteLogerGn(gn)) != 0)
				printf("\n\tWriteLogerGn:Error=%d",i);
		}
*/
		// Marcar Irrecuperables

		if( (FlagIrrec >= LOGER_INT_MARCAR_IRREC) && ((gn.IndHisAna+1) == gn.IndActAna) ){
		    FlagIrrec=0;
		    printf("\n\tAXIS-DATALOGER:MARCAR IRRECUPERABLES:");
		    if( (i=MarcarIrrecuperables()) < 0){
			printf("\n\n\t\tERROR MARCAR IRRECUPERABLES: %d \n",i);
		    }
		}

		FlagIrrec++;


		TimeWait(20);						// TimeOut
		fflush(stdout);
	}
	exit(0);
}

