/*
*	Prometeo
*	File:		logersaihbd.c
*	Autor:		M.Bibudis
*	Fecha:		01/02/2005
*	Revision:	1.0
*	Producto:	Axis ine Aksion
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

BDCONF BdConf;
DIGCONF DigConf;
ANACONF AnaConf;
QM qm;
IN in;
GN gn;

main()
{
	int i,q,b,n,g;
	char path[80];

	printf("\n\tINI CREASAIHBD...%ld \n",time(NULL));
	memset((char *)&BdConf,0,sizeof(BdConf));
	memset((char *)&qm,0,sizeof(QM));
	memset((char *)&in,0,sizeof(IN));

	if( (char *)getenv("SAIHBD") ==NULL){
		fprintf(stderr,"\n\tINISAIBD:Variable Entorno SAIHBD NO SET");
		return(-1);}
	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerBdConfig.dat"));		// Crea Fichero  B.D Estacion
	if((fh=fopen(path,"r+b"))==NULL){
		printf("\n\tNO OPEN: Path=%s",path);
		if((fh=fopen(path,"w+b"))==NULL){
			fprintf(stderr,"\nINISAIHBD:No se puede crear:%s ",path);
			return(-2);}}

	if(!fwrite(&BdConf,sizeof(BdConf),1,fh)){
		fprintf(stderr,"\nINISAIHBD:No se puede crear:%s ",path);
		return(-2);}
	if(fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerAnalogicas.dat"));		// Crea fichero Historico Analogicas
	if((fh=fopen(path,"r+b"))==NULL){
		if((fh=fopen(path,"w+b"))==NULL){
			fprintf(stderr,"\nINISAIHBD:No se puede abrir:%s",path);
			return(-2);}}
	for(i=0;i<NUMHISTQM;i++){
		if(!fwrite(&qm,sizeof(QM),1,fh)){
			fprintf(stderr,"\nINISIAHBD:No se puede crear:%s",path);
			return(-2);}
	}
	if(fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerDigitales.dat"));		// Crea Fichero Historico Incidencias
	if((fh=fopen(path,"r+b"))==NULL){
		if((fh=fopen(path,"w+b"))==NULL){
			fprintf(stderr,"\nINISAIHBD:No se puede abrir:%s",path);
			return(-2);}}
	if(!fwrite(&in,sizeof(IN),1,fh)){
		fprintf(stderr,"\nINISIAHBD:No se puede crear:%s",path);
		return(-2);}
	if(fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);

	strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));			// Crea Fichero Parametros Generales
	if((fh=fopen(path,"r+b"))==NULL){
		if((fh=fopen(path,"w+b"))==NULL){
			fprintf(stderr,"\nINISAIHBD:No se puede abrir:%s",path);
			return(-2);}}
	if(!fwrite(&gn,sizeof(GN),1,fh)){
		fprintf(stderr,"\nINISIAHBD:No se puede crear:%s",path);
		return(-2);}
	if(fclose(fh)!=0) printf("\n\tError en fclose:%d ",errno);

	q=sizeof(QM); b=sizeof(BDCONF); n=sizeof(IN); g=sizeof(GN);
	printf("\n\tFIN CREASAIHBD...%d BD=%d QM=%d %d IN=%d GN=%d Byte\n",time(NULL),b,q,q*2976,n,g);
}
