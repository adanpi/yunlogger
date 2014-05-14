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
	long segjul,segjul80;
	char buf[5];
	int nano,ndia,nmes,h,m,s;

	printf("\n\t\tKalimera ...\n");

		h=m=s=0;
		ndia=1;
		segjul80=time(NULL);
		newtime=localtime(&segjul80);
		newtime->tm_year=80; 
		newtime->tm_mon=0,
		newtime->tm_mday=ndia;
		newtime->tm_hour=h; newtime->tm_min=m;newtime->tm_sec=s;
		segjul80=mktime(newtime);

		auxch=asctime(newtime);

		printf("\n\t\tFecha:1980 := %ld  %s",segjul80,auxch);
		printf("\n\n\n ");

}
