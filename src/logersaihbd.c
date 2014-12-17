/*
*	Prometeo
*	File:		saihbd.c
*	Autor:		M.Bibudis
*	Fecha:		01/02/2005
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
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <netdb.h>
#include <math.h>
#include "logersaihbd.h"

//extern int h_errno;
BDCONF BdConf;
QM qm;
IN in;
GN gn;
QM qmhis;
FILE *fhRBD;
FILE *fhWBD;
FILE *fhRQM;
FILE *fhWQM;
FILE *fhRIN;
FILE *fhWIN;
FILE *fhRGN;
FILE *fhWGN;

// Lee la Base de Datos Estacion. Se le llama con la direccion del objeto tipo BDCONF.
// Devuelve 0=corecto, -1=error.
ReadLogerBd(bdconf)
BDCONF *bdconf;
{
	short i;
	char path[80];
	char aux[64];
	static flaglog=0;

	memset((char *)&BdConf,0,sizeof(BdConf));

	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerBd:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerBdConfig.dat"));	//Config Remota,Sen Ana, Sen Dig	
	if((fhRBD=fopen(path,"rb"))==NULL){
		printf("\nReadLogerBD:No se puede abrir:%s errno=%d",path,errno);
		if(flaglog < 1){
			sprintf(aux,"\nReadLogerBd:fopen:Errno=%d  %ld ",errno,time(NULL));
			AxisLog(aux);
			flaglog++;}     
		return(-1);}
	if(i=fread(&BdConf,sizeof(BdConf),1,fhRBD) <=0){
		printf("\nReadLogerBD:Error_En_read:%s Errn=%d",path,errno);
		fclose(fhRBD);
		return(-1);}

	if(i=fclose(fhRBD) !=0){
                printf("\nReadLogerBd:fclose:Error=%d i=%d",errno,i);
		}

	bdconf=&BdConf;

	return(0);
}

// Escribe en la Base de Datos Estacion. Se le llama con un objeto tipo BDCONF.
// Devuelve 0=corecto, -1=error.
WriteLogerBd(BdConf)
BDCONF BdConf;
{
	short i;
	char path[80];
	char aux[64];
        static flaglog=0;


	if( sizeof(BdConf) > sizeof(BDCONF)){
		printf("\n\tSizof(BdConf)=%",sizeof(BdConf));
		return(-1);}

	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tREadLogerBd:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerBdConfig.dat"));	// Config Remota, Sen Ana, Sen Dig
	if((fhWBD=fopen(path,"r+b"))==NULL){
		printf("\nWriteLogerBD:No se puede abrir:%s errno=%d",path,errno);
		 if(flaglog < 1){
                        sprintf(aux,"\nWriteLogerBd:fopen:Errno=%d  %ld ",errno,time(NULL));
                        AxisLog(aux);
                        flaglog++;}
		return(-1);}

	if((fseek(fhWBD,0L,SEEK_SET))==-1){
                printf("\n\tWriteLogerBd:Error_En_fseek:%s errno=%d",path,errno);
                fclose(fhWBD);
                return(-1);}
        if(!fwrite(&BdConf,sizeof(BdConf),1,fhWBD)){
                printf("\nWriteLogerBd:No se puede crear:%s Errno=%d",path,errno);
		fclose(fhWBD);
                return(-1);}
        if(i=fclose(fhWBD) !=0){
                printf("\nWriteLogerBd:fclose:Error=%d i=%d",errno,i);
		}

	return(0);
}

// Lee un Quinceminutal. Se le llama con la direccion del  objeto tipo QM.
// Devuelve 0=corecto, -1=error.
ReadLogerQm(segjulqm,leerqm)
unsigned long segjulqm;
QM *leerqm;
{
	long byteqm;
	short i,numhistqm;
	short minuto,hora,dia,mes,anio;
	char path[80];
	unsigned long segjulini,segjulact,segjulloger;
	struct tm *newtime;
	char *auxch;
	char aux[64];
        static flaglog=0;



	if(segjulqm <=0)			// localtime
		return(-1);
	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerQm:Variable Entorno SAIHBD NO SET");
		return(-1);}
	memset((char *)&qm,0,sizeof(QM));

	segjulact=time(NULL);			// SegJulAct
	segjulloger=segtojul(NULL);
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	//printf("\n\tFecha Actual:%ld %ld %ld %s",segjulact,segjulloger,(segjulact-segjulloger),auxch);
	segjulact=(segjulact/SEGPQM)*SEGPQM;
	
	segjulqm=segjulqm;
	newtime=localtime(&segjulqm);		// SegJulQM Solicitado
	auxch=asctime(newtime);
	//printf("\n\tFecha QM: %ld %s",segjulqm,auxch);
	segjulqm=(segjulqm/SEGPQM)*SEGPQM;

	// SegJulIni dia=1,hora=0,sec=0 del Mes del QM Solicitado
	newtime->tm_mday=1; newtime->tm_hour=0; newtime->tm_min=0;newtime->tm_sec=0;
	segjulini=mktime(newtime);
	auxch=asctime(newtime);
	//printf("\n\tFecha Ini Mes QM: %ld %s",segjulini,auxch);
	segjulini=(segjulini/SEGPQM)*SEGPQM;

	if(segjulact < SEGJULCTE){
		printf("\n\tSAIHBD Error segjulact=%ld\n",segjulact); return(-2);}
	if(segjulqm < segjulini){
		printf("\n\tSAIHBD Error: (segjulqm<segjulini)=%ld\n",segjulqm); return(-2);}
	if(segjulqm > segjulact){
		printf("\n\tSAIHBD Error:(segjulqm > segjulact)=%ld\n",segjulqm); return(-2);}
	if(segjulqm < segjulact - (NUMHISTQM * SEGPQM)){
		printf("\n\treadLogerQm:Error (segjulqm < segjulact-NUMHISTQM*SEGPQM)=%ld\n",segjulqm);
		return(-2);}

	numhistqm=(segjulqm - segjulini)/SEGPQM;
	if( numhistqm <0 || numhistqm > NUMHISTQM ){
		printf("\n\tSAIHBD Error de Fechas:NumHistQM=%hd\n",numhistqm);
		return(-2);}


	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerAnalogicas.dat"));
	if((fhRQM=fopen(path,"rb"))==NULL){
		printf("\nReadLogerQm:No se puede abrir:%s errno=%d h_errno=%d",path,errno,h_errno);
		if(flaglog < 1){
                        sprintf(aux,"\nReadLogerQm:fopen:Errno=%d  %ld ",errno,time(NULL));
                        AxisLog(aux);
                        flaglog++;}
		return(-1);}

	byteqm=numhistqm * sizeof(QM); 				// Indice Byte QM
	if( byteqm < 0 || byteqm > ((NUMHISTQM-1) * sizeof(QM))){
		printf("\nReadLogerQm:byteqm=%d Errno=%d",byteqm,errno);
                fclose(fhRQM);
		return(-1);}
	if( (i=fseek(fhRQM,byteqm,SEEK_SET))==-1 ){
                printf("\n\tReadLogerBd:fseek:%s errno=%d byteqm=%d",path,errno,byteqm);
                fclose(fhRQM);
                return(-1);}

	if(i=feof(fhRQM)==EOF){
		printf("\nReadLogerQm:feof:Error=%d",errno);
		}
	if(i=ferror(fhRQM)!=0){
		printf("\nReadLogerQm:ferror:Error=%d i=%d",errno,i);
		}
	if(i=fread(&qm,sizeof(QM),1,fhRQM) <=0){
		printf("\nReadLogerQm:fread:Error=%d",errno);
		fclose(fhRQM);
		return(-1);}


	if(i=fclose(fhRQM) !=0){
                printf("\nReadLogerQm:fclose:Error=%d i=%d",errno,i);
		}

	leerqm=&qm;

	if(leerqm->SegJul !=segjulqm){
		printf("\n\tReadLogerQm: Error de Fechas (SegJul(%ld) != segjulqm(%ld))\n",qm.SegJul,segjulqm);
		return(-2);}

	return(0);
}


// Escribe un Quinceminutal. Se le llama con un objeto tipo QM.
// Devuelve 0=corecto, -1=error.
WriteLogerQm(writeqm)
QM writeqm;
{
	short i,j;
	char path[80];
	short numhistqm;
	long byteqm;
	unsigned long segjulqm,segjulini,segjulact,segjulloger;
	struct tm *newtime;
	char *auxch;
	char aux[64];
        static flaglog=0;



	if(writeqm.SegJul <=0 )				// localtime
		return(-1);
	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tWriteLogerQm:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerAnalogicas.dat"));
	if((fhWQM=fopen(path,"r+b"))==NULL){
		printf("\nWriteLogerQm:fopen:No se puede abrir:%s Errno=%d",path,errno);
		if(flaglog < 1){
                        sprintf(aux,"\nWriteQm:fopen:Errno=%d  %ld ",errno,time(NULL));
                        AxisLog(aux);
                        flaglog++;}
		return(-1);}

	segjulact=time(NULL);				// SegJulAct 
	segjulloger=segtojul(NULL);
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	printf("\n\tFecha Actual:%ld %ld %ld %s",segjulact,segjulloger,(segjulact-segjulloger),auxch);
	
	segjulqm=writeqm.SegJul;
	newtime=localtime(&segjulqm);			//SegJulQM Solicitado
	auxch=asctime(newtime);
	printf("\n\tFecha QM: %ld %s",segjulqm,auxch);
	segjulqm=(segjulqm/SEGPQM)*SEGPQM;

							// SegJulIni dia=1,hora=,sec=0 del Mes del QM Solicitado
	newtime->tm_mday=1; newtime->tm_hour=0; newtime->tm_min=0;newtime->tm_sec=0;
	segjulini=mktime(newtime);
	auxch=asctime(newtime);
	printf("\n\tFecha Ini Mes: %ld %s",segjulini,auxch);

	segjulini=(segjulini/SEGPQM)*SEGPQM;
	segjulact=(segjulact/SEGPQM)*SEGPQM;

	if(segjulact < SEGJULCTE){
		printf("\n\tWriteLogerQm: Error segjulact=%ld\n",segjulact);
		fclose(fhWQM);
		return(-2);}
	if(segjulqm < segjulini){
		printf("\n\tWriteLogerQm: Error (segjulqm<segjulini)=%ld\n",segjulqm);
		fclose(fhWQM);
		return(-2);}
	if(segjulqm > segjulact){
		printf("\n\tWriteLogerQm:Error (segjulqm > segjulact)=%ld\n",segjulqm);
		fclose(fhWQM);
		return(-2);}
	if(segjulqm < segjulact - (NUMHISTQM * SEGPQM)){
		printf("\n\tWriteLogerQm:Error (segjulqm < segjulact-NUMHISTQM*SEGPQM)=%ld\n",segjulqm);
		fclose(fhWQM);
		return(-2);}

	numhistqm=(segjulqm - segjulini)/SEGPQM;			// Calculamos Indice QM
	byteqm=numhistqm * sizeof(QM); 					// Indice Byte QM
	if( byteqm < 0 || byteqm > ((NUMHISTQM-1) * sizeof(QM))){
		printf("\nWriteLogerQm:Errno=%d",errno);
                fclose(fhWQM);
		return(-1);}
	if(i=(fseek(fhWQM,byteqm,SEEK_SET))==-1){
		printf("\n\tWriteLogerQm:fseek:%s Errno=%d byteqm=%d",path,errno,byteqm);
		fclose(fhWQM);
		return(-2);}

	 if(i=feof(fhWQM)==EOF){
                printf("\nWriteLogerQm:feof:Error=%d",errno);
		}
        if(i=ferror(fhWQM)!=0){
                printf("\nWriteLogerQm:ferror:Error=%d i=%d",errno,i);
		}


	if(i=fwrite(&writeqm,sizeof(QM),1,fhWQM) <=0){
		printf("\n\tWriteLogerQm:No se puede crear:%s Errno=%d",path,errno);
		fclose(fhWQM);
		return(-2);}


	if(i=fclose(fhWQM) !=0){
                printf("\nWriteLogerQm:fclose:Error=%d i=%d",errno,i);
		}


	return(0);
}

// Lee Todos los Quinceminutales. Se le llama con la direccion del  objeto tipo QmHis[NUMHISTQM].
// Devuelve 0=corecto, -1=error.
ReadLogerQmHis(LeerQmHis)
QM LeerQmHis[NUMHISTQM];
{
	short i;
	char path[80];
	int nbyte;
	FILE *fhhis;
	

	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerQmHis:Variable Entorno SAIHBD NO SET");
		return(-1);}
	memset((char *)&qm,0,sizeof(QM));

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerAnalogicas.dat"));
	if((fhhis=fopen(path,"rb"))==NULL){
		printf("\nReadLogerQmHis:No se puede abrir:%s Errno=%d",path,errno);
		return(-1);}

	nbyte=sizeof(QM);
	for(i=0;i<NUMHISTQM;i++){
		if(!fread(&qm,sizeof(QM),1,fhhis)){
			printf("\nReadLogerQmHis:Error=%d",errno);
			fclose(fhhis);
			return(-1);}
		memcpy((char *)&LeerQmHis[i],(char *)&qm,nbyte);
	}
	fclose(fhhis);

	return(0);
}

// Lee Incidencias. Se le llama con la direccion del  objeto tipo IN.
// Devuelve 0=corecto, -1=error.
ReadLogerIn(leerin)
IN *leerin;
{
	static flaglog=0;
	long byteqm;
	short i,numhistqm;
	short minuto,hora,dia,mes,anio;
	char path[80];
	unsigned long segjulini,segjulact,segjulloger;
	struct tm *newtime;
	struct timeval *tv;
	struct timezone *tz;
	char *auxch;
	char aux[64];


	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerIn:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerDigitales.dat"));
	if((fhRIN=fopen(path,"rb"))==NULL){
		printf("\nReadLogerIN:No se puede abrir:%s errno=%d",path,errno);
		if(flaglog < 1){
			sprintf(aux,"\nReadLogerIN:fopen:Errno=%d  %ld ",errno,time(NULL));
			AxisLog(aux);
			flaglog++;}     
		return(-1);}
	if(i=fread(&in,sizeof(IN),1,fhRIN) <=0){
		printf("\nReadLogerIn:Error=%d",errno);
		fclose(fhRIN);
		return(-1);}

	fclose(fhRIN);

	leerin=&in;

	return(0);
}
// Escribe Incidencias. Se le llama con la direccion del  objeto tipo IN.
// Devuelve 0=corecto, -1=error.
WriteLogerIn(writein)
IN writein;
{
	static flaglog=0;
	long byteqm;
	short i,numhistqm;
	short minuto,hora,dia,mes,anio;
	char path[80];
	unsigned long segjulini,segjulact,segjulloger;
	struct tm *newtime;
	char *auxch;
	char aux[64];


	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tWriteLogerIn:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerDigitales.dat"));
	if((fhWIN=fopen(path,"r+b"))==NULL){
		printf("\nReadLogerIN:No se puede abrir:%s errno=%d",path,errno);
		if(flaglog < 1){
			sprintf(aux,"\nReadLogerIN:fopen:Errno=%d  %ld ",errno,time(NULL));
			AxisLog(aux);
			flaglog++;}     
		return(-1);}
	if((fseek(fhWIN,0L,SEEK_SET))==-1){
                printf("\n\tWriteLogerIn:Error_En_fseek:%s errno=%d",path,errno);
                fclose(fhWIN);
                return(-1);}
	if(!fwrite(&writein,sizeof(IN),1,fhWIN)){
		printf("\nReadLogerIn:Error=%d",errno);
		fclose(fhWIN);
		return(-1);}
	fclose(fhWIN);
	return(0);
}
// Lee la Base de Datos GN
// Devuelve 0=corecto, -1=error.
ReadLogerGn(leergn)
GN *leergn;
{
	short i;
	char path[80];
	char aux[64];
	static flaglog=0;

	memset((char *)&gn,0,sizeof(gn));

	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerGn:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));	//Config Remota,Sen Ana, Sen Dig	
	if((fhRGN=fopen(path,"rb"))==NULL){
		printf("\nReadLogerGN:No se puede abrir:%s errno=%d",path,errno);
		if(flaglog < 1){
			sprintf(aux,"\nReadLogerGN:fopen:Errno=%d  %ld ",errno,time(NULL));
			AxisLog(aux);
			flaglog++;}     
		return(-1);}
	if(i=fread(&gn,sizeof(gn),1,fhRGN) <=0){
		printf("\nReadLogerGN:Error_En_read:%s Errn=%d",path,errno);
		fclose(fhRGN);
		return(-1);}

	if(i=fclose(fhRGN) !=0){
                printf("\nReadLogerGn:fclose:Error=%d i=%d",errno,i);
		}

	leergn=&gn;

	return(0);
}

// Escribe Parametros Generales
// Devuelve 0=corecto, -1=error.
WriteLogerGn(gn)
GN gn;
{
	short i;
	char path[80];
	char aux[64];
        static flaglog=0;


	if( sizeof(gn) > sizeof(GN)){
		printf("\n\tSizeof(Gn)=%",sizeof(gn));
		return(-1);}

	if( (char *)getenv("SAIHBD") ==NULL){
		printf("\n\tReadLogerGn:Variable Entorno SAIHBD NO SET");
		return(-1);}

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));	// Config Remota, Sen Ana, Sen Dig
	if((fhWGN=fopen(path,"r+b"))==NULL){
		printf("\nWriteLogerGN:No se puede abrir:%s errno=%d",path,errno);
		 if(flaglog < 1){
                        sprintf(aux,"\nWriteLogerGN:fopen:Errno=%d  %ld ",errno,time(NULL));
                        AxisLog(aux);
                        flaglog++;}
		return(-1);}

	if((fseek(fhWGN,0L,SEEK_SET))==-1){
                printf("\n\tWriteLogerGN:Error_En_fseek:%s errno=%d",path,errno);
                fclose(fhWGN);
                return(-1);}
        if(!fwrite(&gn,sizeof(gn),1,fhWGN)){
                printf("\nWriteLogerGn:No se puede crear:%s Errno=%d",path,errno);
		fclose(fhWGN);
                return(-1);}
        if(i=fclose(fhWGN) !=0){
                printf("\nWriteLogerGn:fclose:Error=%d i=%d",errno,i);
		}

	return(0);
}

IniDatosAna(){


QM qm;
BDCONF BdConf;
GN gn;
unsigned long segjulact,segjulqm,qmactual;
struct tm *newtime;
char *auxch;
char aux[128];
int i,j,sen;
double radianes;
float valor;
unsigned short valors;
unsigned int valorint;

	printf("\n\nIniDatosAna %ld \n",time(NULL));

	radianes=0.0;

	segjulact=time(NULL);					// Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	printf("\n\tIniDatosAna:FECHA ACTUAL = %ld  %s ",segjulact,auxch);

	segjulqm=segjulact;					// Fecha QM desde epoca=1970
	segjulqm=((segjulqm/SEGPQM)*SEGPQM);
	qmactual=segjulqm-SEGPQM;					// QM

	// Se leen factores analogicas de la BdConf
	memset((char *)&BdConf,0,sizeof(BDCONF));

	if(j=ReadLogerBd(&BdConf)!=0){
		printf("\n\t IniDatosAna Error lectura BdConfig:Error=%d",j);
		return(-3);}

       	if( (j=ReadLogerGn(&gn)) !=0)
              printf("\n\tReadLogerGn:Error=%d",j);

	gn.segjulhis=segjulqm-SEGPQM;		// Ultimo Juliano Recibido apl web
	if( (j=WriteLogerGn(gn)) !=0)
		printf("\n\tWriteLogerGn:Error=%d",j);

	// rellenamos todos los datos históricos
	for(i=0;i<NUMHISTQM-400;i++){
		printf("\n\t \tIniDatosAna:FECHA QM = %ld  %s ",segjulact,auxch);
		memset((char *)&qm,0,sizeof(qm));				// Objeto QM
		for (sen=0;sen<NUMSENANA;sen++)	qm.Flag[sen]='V';		//inicializamos Flag SAICA como 'N' (NO VALIDO)
		qm.Status=512;

		qm.SegJul=segjulqm-SEGPQM;			//Seg Jul Unix QM recuperado (inicio del QM)
		qm.SegJulPer=segjulqm-SEGPQM;
		qm.NumAna=8;					// rellenamos solo 8 señales
		
		for(sen=0;sen<qm.NumAna;sen++){
			qm.FlValorAna[sen]=sen+3+sin(radianes);
			//cambio a cuentas short protocolo SAC
/*			if (BdConf.anaconf.fcm[sen] ==  0) {
				sprintf(aux,"Factor multiplicativo = 0, senal: %d \n",sen);
				AxisLog(aux);				// Log
				printf("\n\t%s",aux);
				valorint = 0;
				qm.FlValorAna[sen]=-9999.99;
			}else
*/				valorint=(unsigned int)(valor/BdConf.anaconf.fcm[sen] - BdConf.anaconf.fca[sen]/BdConf.anaconf.fcm[sen]);
			if (valorint < 0 || valorint > 65535){
				sprintf(aux,"Valor Analogico fuera de rango Proto SAC Sen: %d Valor:%.2f\n",sen,valor);
//				AxisLog(aux);				// Log
				printf("\n\t%s\n",aux);
				valorint = 0;
				qm.FlValorAna[sen]=-9999.99;
			}
			valors=(unsigned short)valorint;
		        qm.ValorAna[sen]=valors;
		}

		qm.NumCont=NUMSENCONT;	// no hay contadores cincominutales
		qm.NumGray=NUMSENGRAY;
		qm.NumRs=NUMSENRS;

		if( (j=CrearBufferQm(&qm,BdConf.remconf.ihw))  !=0){
			printf("\n\tCrearBufferQm:Error=%d",j);
			return(-3);}
		if( (j=WriteLogerQm(qm)) !=0){				// Escribimos el QM Modificado
			printf("\n\tWriteLogerQm:Error=%d",j);
			return(-3);}

		// anterior QM
		segjulact=segjulqm-SEGPQM;			// Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		segjulqm=segjulact;	
		radianes+=0.73;
	}
}

//Inicializa Ficheros *.dat
IniLogerBd()
{
        int i,q,b,n,g;
        char path[80],aux[12];
	FILE *fh;
	BDCONF BdConf;
	DIGCONF DigConf;
	ANACONF AnaConf;
	QM qm;
	IN in;
	GN gn;

        printf("\n\tINI CREASAIHBD...%ld \n",time(NULL));
        memset((char *)&BdConf,0,sizeof(BdConf));
        memset((char *)&qm,0,sizeof(QM));
        memset((char *)&in,0,sizeof(IN));
        memset((char *)&gn,0,sizeof(GN));
	q=b=n=g=0;

	for (i=0;i<NUMSENANA;i++){
		sprintf(aux,"%s%d","A",i+1);
		strcpy(BdConf.anaconf.tag[i],aux);
		sprintf(aux,"%s%d","Analogica entrada ",i+1);
		strcpy(BdConf.anaconf.desc[i],aux);
		BdConf.anaconf.fcm[i]=0.1;
		BdConf.anaconf.fca[i]=-2000.0;
		printf("\n\t IniLogerBd:sen=%d %s fcm=%f fca=%f ",i,aux,BdConf.anaconf.fcm[i],BdConf.anaconf.fca[i]);
	}

        if( (char *)getenv("SAIHBD") ==NULL){
                fprintf(stderr,"\n\tINISAIBD:Variable Entorno SAIHBD NO SET");
                return(-1);}
        strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerBdConfig.dat"));              // Crea Fichero  B.D Estacion
        if((fh=fopen(path,"r+b"))==NULL){
                printf("\n\tNO OPEN: Path=%s",path);
                if((fh=fopen(path,"w+b"))==NULL){
                        fprintf(stderr,"\nINISAIHBD:No se puede crear:%s ",path);
                        return(-2);}
        	if(!fwrite(&BdConf,sizeof(BdConf),1,fh)){
                	fprintf(stderr,"\nINISAIHBD:No se puede crear:%s ",path);
                	return(-2);}
        	b=sizeof(BDCONF);
	}
	if(i=fclose(fh) !=0)
        	printf("\n\tError en fclose:%d ",errno);

	for (i=0;i<NUMSENANA;i++)	qm.Flag[i]='N';		//inicializamos Flag SAICA como 'N' (NO VALIDO)

        strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerAnalogicas.dat"));            // Crea fichero Historico Analogicas
        if((fh=fopen(path,"r+b"))==NULL){
                if((fh=fopen(path,"w+b"))==NULL){
                        fprintf(stderr,"\nINISAIHBD:No se puede abrir:%s",path);
                        return(-2);}
        	for(i=0;i<NUMHISTQM;i++){
                	if(!fwrite(&qm,sizeof(QM),1,fh)){
                        	fprintf(stderr,"\nINISIAHBD:No se puede crear:%s",path);
                        	return(-2);}
        	}
        	q=sizeof(QM);
	}
        if(i=fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);


        strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerDigitales.dat"));             // Crea Fichero Historico Incidencias
        if((fh=fopen(path,"r+b"))==NULL){
                if((fh=fopen(path,"w+b"))==NULL){
                        fprintf(stderr,"\nINISAIHBD:No se puede abrir:%s",path);
                        return(-2);}
        	if(!fwrite(&in,sizeof(IN),1,fh)){
                	fprintf(stderr,"\nINISIAHBD:No se puede crear:%s",path);
                	return(-2);}
        	n=sizeof(IN);
	}
	if(i=fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);


        strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));                   // Crea Fichero Parametros Generales
        if((fh=fopen(path,"r+b"))==NULL){
                if((fh=fopen(path,"w+b"))==NULL){
                        fprintf(stderr,"\nINISAIHBD:No se puede abrir:%s",path);
                        return(-2);}
	//inicializamos indices historicos
	gn.IndHisAna=0;
	gn.IndHisCon=0;
	gn.IndHisDig=0;	
        	if(!fwrite(&gn,sizeof(GN),1,fh)){
                	fprintf(stderr,"\nINISIAHBD:No se puede crear:%s",path);
                	return(-2);}
        	g=sizeof(GN);
	}
        if(i=fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);

	if(b==0)
        	printf("\n\tFILE LogerBdConfig.dat CREADO");
	else
        	printf("\n\tSE CREA FILE LogerBdConfig.dat: BD=%d ",b);

	if(q==0)
        	printf("\n\tFILE LogerAnalogicas.dat CREADO:");
	else
        	printf("\n\tSE CREA FILE LogerAnalogicas.dat: QM=%d %d Byte\n",q,q*2976);

	if(g==0)
        	printf("\n\tFILE LogerGen.dat CREADO");
	else
        	printf("\n\tSE CREA FILE LogerGen.dat: GN=%d Byte",g);

	if(n==0)
        	printf("\n\tFILE LogerDigitales.dat CREADO");
	else
        	printf("\n\tSE CREA FILE LogerDigitales.dat: IN=%d Byte",n);
	printf("\n\tIniLogerbd Fin...");

}
// Aplicacion Axis Log
AxisLog(buffer)
char buffer[132];
{
        //short i;
        char path[80];
        FILE *fhl;
        struct stat bufstat;


        if( (char *)getenv("SAIHBD") ==NULL){
                printf("\n\tAxisLog:Variable Entorno SAIHBD NO SET");
                return;}
        strcat(strcpy(path,(char *)getenv("SAIHBD")),("log/axis.log"));      // Parametros Generales

	if(!stat(path,&bufstat)){						/*borrar si tamano fichero mayor */
                if(bufstat.st_size > (10000))
			unlink(path);
	}

        if((fhl=fopen(path,"a"))==NULL){
                printf("\nAxisLog:No se puede abrir:%s",path);
                return(-2);}

        fprintf(fhl,"\n\t%s",buffer);

        fclose(fhl);

        return(0);
}
// Escribir el PID del Proceso
PidLog(name,ipid)
char name[25];
short ipid;
{
        short i;
        char path[80];
        FILE *fhp;

        if( (char *)getenv("SAIHBD") ==NULL){
                printf("\n\tPidLog:Variable Entorno SAIHBD NO SET");
                return;}

        strcat(strcpy(path,(char *)getenv("SAIHBD")),("log/"));      // Parametros Generales
        strcat(path,name);
        if((fhp=fopen(path,"w"))==NULL){
                printf("\nPidLog:No se puede abrir:%s",path);
                return(-2);}

        fprintf(fhp,"%hd",ipid);

        fclose(fhp);

        return(0);
}
KillPid(name)
char name[20];
{
        short i;
	int ipid;
        char path[80];
	char ch[7];
        FILE *fhp;

	memset((char *)ch,0,7);
	ipid=99000;
        if( (char *)getenv("SAIHBD") ==NULL){
                printf("\n\tPidLog:Variable Entorno SAIHBD NO SET");
                return;}

        strcat(strcpy(path,(char *)getenv("SAIHBD")),("log/"));      // Parametros Generales
        strcat(path,name);
        if((fhp=fopen(path,"r"))==NULL){
                printf("\nPidLog:No se puede abrir:%s",path);
                return(-1);}
	while(fgets(ch,7,fhp)){
            if(sscanf(ch,"%d",&ipid)==1) break;
	}
        fclose(fhp);

	if(ipid < 200)
		return(-1);
	printf("\n\tch=%s PID=%d",ch,ipid);
	if(i=kill(ipid,SIGKILL) ==0)
        	return(0);
	else return(-1);
}

