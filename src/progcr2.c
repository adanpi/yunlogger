/*
*       Prometeo
*       FILE:           AxisLogerUtil.c
*       AUTHOR:         Adan Pineiro@M.Bibudis
*       DATE:           24-11-05
*       REVISION:       1.0
*       PRODUCT:        AxisLoger 
*       SUBJECTS:       
*       O.S.:           LINUX ine Axion
*       CUSTOMER:       SAIH
*/
#
#
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <limits.h>
#
//#include "axisloger.h"	// include AxisSerialUtils 
//#include "ipcserver.h"	// include Mensajes DataLoger
#include "logersaihbd.h"
#




main(int argc, char *argv[])
{
int i,NumChar=0;	
unsigned char NomVar[20];
unsigned char auxch;

	//printf("\n\tAXIS_DATALOGER: Datos Instantaneos :");
	
	if(argc != 2){printf("\n\tuso: progcr2 NombreVariable\n\n\t\tej: progcr2 NumAna\n\n");exit(0);}

	printf("Valor variable: %s : ",argv[1]);
	for(i=0 ; i< 20 ; i++){
    	    auxch=(unsigned char)argv[1][i];
	    if (auxch != 0x00 ){
		NumChar++;
		NomVar[i]=auxch;
	    }else break;
	    //printf(" %02x ",argv[1][i]);	 
	}
/*	
NomVar[0]=0x4E;
NomVar[1]=0x75;
NomVar[2]=0x6D;
NomVar[3]=0x41;
NomVar[4]=0x6E;
NomVar[5]=0x61;
NumChar=6;
*/
	if( (i=DatosInstantaneos(NomVar,NumChar)) < 0 )
		printf("Error: %d", i);


}


