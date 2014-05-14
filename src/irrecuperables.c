/*
*	Prometeo
*	File:		irrecuperables.c
*	Autor:		Adan
*	Fecha:		20/02/2007
*	Revision:	1.0
*	Producto:	Axis ine Axion
*	Objetivo:	Kosmos...
*	Customer:	SAIH
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
#include <netdb.h>
#include "logersaihbd.h"
//extern int h_errno;
FILE *fh;
FILE *fh2;

BDCONF BdConf;
QM qm;
IN in;
GN gn;
QM QmHis[NUMHISTQM];
int ReadLogerBd();

main(argc,argv)
char *argv[];
int argc;
{
    printf("\n\nFin: %d",MarcarIrrecuperables());
/*
	short i,j,ihw,sen,status;
	short numhistqm;
	unsigned short valor,valorMax=0,valorMin=0;
	long segjulqm,segjulqmIni,segjulqmFin;
	struct tm *newtime;
	char *auxch,fecha[15];
	int ndia,nmes,nano,h,m,s;
	char path[80];

	i=j=ihw=sen=status=segjulqm=0;
	if (argc!=1){
		printf("\n\tUso: irrecuperables\n");
		exit(0);
	}
	
	if(i=ReadLogerBd(&BdConf)!=0){			// Leer Objeto B.D  BDCONF 
		printf("\n\tReadLogerBd:Error=%d",i);
		exit(1);
	}

	memset((char *)&QmHis,0,sizeof(QmHis));
	if( i=ReadLogerQmHis(&QmHis) != 0 ){		// Leemos Historicos QM
		printf("\n\tReadLogerQm:Error=%d",i);
		exit(1);
	}
	
	segjulqmIni=time(NULL);
	segjulqmFin=((segjulqmIni/SEGPQM)*SEGPQM);

	// SegJulQmIni dia=1,hora=0,sec=0 del Mes del QM Solicitado
	newtime=localtime(&segjulqmIni);
	newtime->tm_mday=1; newtime->tm_hour=0; newtime->tm_min=0;newtime->tm_sec=0;
	segjulqmIni=mktime(newtime);
	auxch=asctime(newtime);
	//printf("\n\tFecha Ini Mes QM: %ld %s",segjulini,auxch);
	segjulqmIni=(segjulqmIni/SEGPQM)*SEGPQM - SEGPQM;

	// se revisa hasta la hora actual menos 1 hora
	numhistqm = ((segjulqmFin - segjulqmIni)/SEGPQM) - 4;
					    
	//for(i=0;i<NUMHISTQM;i++){
	for(i=0;i<numhistqm;i++){
		segjulqmIni = segjulqmIni + SEGPQM;
		if(QmHis[i].SegJul != segjulqmIni){
		    printf("\tQM IRRECUPERABLE: QM BD:%d \tQM Real:%d \n",QmHis[i].SegJul,segjulqmIni);
		    
		    continue;
		}

		//newtime=localtime(&QmHis[i].SegJul);
		newtime=localtime(&segjulqmIni);
		sprintf(fecha,"%02d/%02d/%02d %02d:%02d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year-100,
		newtime->tm_hour,newtime->tm_min);	
		printf("%d : %s Status= 0x%02x \t%d  %d\n",i,fecha,QmHis[i].Status,QmHis[i].SegJul,segjulqmIni);
	}
	
	printf("\n");
	
*/
}
