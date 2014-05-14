/*      Prometeo
*       File:           saihbd.c
*       Autor:          M.Bibudis
*       Fecha:          01/03/2005
*       Revision:       1.0
*       Producto:       Axis ine Axion
*       Objetivo:       Kosmos...
*       Customer:       SAIH
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "logersaihbd.h"

main(argc,argv)
int argc;
char *argv[];
{
	short i;
 	struct tm *newtime;
        char *auxch;
	long segjul,segjulqm;
	char buf[5];
	int nano,ndia,nmes,h,m,s;

	printf("\n\t\tKalimera ...\n");

        if(argc<1 || argc>3){
                printf("\n\tEror Parametros seg || DD-MM-AA HH:MM:SS\n");
                exit(0);}

        switch(argc){
            case 1:
		segjul=time(NULL);
		newtime=localtime(&segjul);
		auxch=asctime(newtime);
		printf("\n\t\tFecha: %ld %s",segjul,auxch);

		segjulqm=(segjul/SEGPQM)*SEGPQM;
		memset(buf,0,5);
		newtime=localtime(&segjulqm);
		auxch=asctime(newtime);
		printf("\n\t\tFecha QM: %ld (LOGER=%ld) %s",segjulqm,segjulqm-SEGJULCTE,auxch);
		memcpy(buf,(char *)&segjulqm,4);
		printf("\n\t\tByte:");
		for(i=0;i<4;i++)
			printf("%02x ",buf[i]);
		printf("\n\n\n");
            break;
            case 2:
		if( sscanf(argv[1],"%ld",&segjul) !=1)
                	printf("\n\tEror Parametros Seg || DD-MM-AA HH:MM:SS\n");
		newtime=localtime(&segjul);
		auxch=asctime(newtime);
		printf("\n\t\tFecha: %ld %s",segjul,auxch);

		segjulqm=(segjul/SEGPQM)*SEGPQM;
		memset(buf,0,5);
		newtime=localtime(&segjulqm);
		auxch=asctime(newtime);
		printf("\n\t\tFecha QM: %ld (LOGER=%ld) %s",segjulqm,segjulqm-SEGJULCTE,auxch);
		memcpy(buf,(char *)&segjulqm,4);
		printf("\n\t\tByte:");
		for(i=0;i<4;i++)
			printf("%02x ",buf[i]);
		printf("\n\n\n");
            break;

            case 3:
                if( strlen(argv[1]) > 8 || strlen(argv[2]) >8 ){
                	printf("\n\tError_1 Parametros seg || DD-MM-AA HH:SS\n");
                        exit(1);}
		ndia=nmes=nano=0;
		if ((sscanf (argv[1],"%02d%*c%02d%*c%02d",&ndia,&nmes,&nano) > 5) || (ndia < 1) || (ndia >31)
			|| (nmes < 1) || (nmes > 12) || (nano < 0) || (nano > 39)){
                	printf("\n\tError_2 Parametros seg || DD-MM-AA HH:MM:SS\n");
			return(-1);}

		h=m=s=0;
		if ((sscanf(argv[2],"%2d:%2d:%2d",&h,&m,&s) > 3) || (h < 0) || (h > 23) ||
			(m < 0) || (m > 59) || (s < 0) || (s > 59)){
                	printf("\n\tError_3 Parametros seg || DD-MM-AA HH:MM:SS\n");
			return(-1);}

		segjul=time(NULL);
		newtime=localtime(&segjul);
		newtime->tm_year=100+nano; newtime->tm_mon=nmes-1;
		newtime->tm_mday=ndia; newtime->tm_hour=h; newtime->tm_min=m;newtime->tm_sec=s;
		segjul=mktime(newtime);
		memset(buf,0,5);
		auxch=asctime(newtime);
		printf("\n\t\tFecha : %ld (LOGER=%ld) %s",segjul,segjul-SEGJULCTE,auxch);
		memcpy(buf,(char *)&segjul,4);
		printf("\n\t\tByte:");
		for(i=0;i<4;i++)
			printf("%02x ",buf[i]);
		//printf("\n\n\n ");
		segjul=segjul-SEGJULCTE;
		memcpy(buf,(char *)&segjul,4);
		printf("\n\t\tByte Incid:");
		for(i=0;i<4;i++)
			printf("%02x ",buf[i]);
		printf("\n\n\n ");

            break;
            default:
                printf("\n\tError Parametros seg || DD-MM-AA HH:MM:SS\n");
                exit(1);
        }
}
