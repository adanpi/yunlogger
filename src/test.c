/*       
*       FILE:           yunlogger.c
*       AUTHOR:         Adan
*       DATE:           04-06-14
*       REVISION:       1.0
*       PRODUCT:        Arduino Yun DataLoger
*       SUBJECTS:
*       O.S.:           LINUX
*       CUSTOMER:       
*
*              Modificaciones:
*                      04/06/2014      Inicio proyecto
*			16/12/2014	Incorporar escritura modbus de los datos adquiridos y de las medias
*
*		TODO: intervalo integración variable, hay que cambiar cabeceras .h y la lógica de todo el sistema.
*
*/
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


static char *version = "1.1.01 (16/12/2014: +modbus  23/06/2014 1.1.00: +conf. file)";

#define NUMSEN 10

GN gn;
QM qm;
BDCONF BdConf;

int main(int argc, char *argv[])
{
	short i,j,n,ipid,muestreo,adquirirDato=0,guardarDato=0,numSen;
	int sen;
	char name[15],path[92];
	char aux[64],ch[133],comando[NUMSEN][133];
	long numMuestreos[NUMSEN];
	unsigned long segjulact,segjulnew,espera,segjulqm;
	struct tm *newtime;
	char *auxch;
	FILE *fhf;
	float valor, valoracum[NUMSEN],valoractual[NUMSEN];
	unsigned short valors;
	unsigned int valorint;
	unsigned char table[80],caracter[2];

	if ( (argc==2) && (argv[1][1]=='v')){
		printf("\n************************************");
		printf("\n\t radsys.es");
		printf("\n\t test Version: %s",version);
		printf("\n\t\t + PRE y POST SCAN");
		printf("\n************************************\n");
		return(0);
	}


	if( (char *)getenv("SAIHBD") == NULL){
		printf("\n\t yunlogger:Variable Entorno SAIHBD NO SET");
		return(-1);}

	// inicializar 
	for(sen=0;sen<NUMSEN;sen++){
		numMuestreos[sen]=0;
		valoracum[sen]=0;
		valoractual[sen]=0;
	}

	// leer parámetros de adquisición
	strcat(strcpy(path,(char *)getenv("SAIHBD")),("yunlogger.conf"));
	
	if((fhf=fopen(path,"r"))!=NULL){
		i=0;
		while(fgets(ch,130,fhf)){
			printf("\n\tyunlogger.conf[%d]=%s",i,ch);
			if(i==2){
				if(sscanf(ch,"%hd",&muestreo)!=1){
					sprintf(aux,"yunlogger.conf error en linea 3: Tiempo de Muestreo: %s",ch);
	
					printf("\n\t  %s \n",aux);
					muestreo=10;
				}
			}
			if(i==6){
				if(sscanf(ch,"%hd",&numSen)!=1){
					sprintf(aux,"yunlogger.conf error en linea 7: Numero de señales: %s",ch);

					printf("\n\t  %s \n",aux);
					numSen=1;
				}
				
			}
			if(i>=8){
				strcpy(comando[i-8],ch);
			}
			i++;
		}
		fclose(fhf);
	}else{
		printf("\n\tyunlogger.conf no encontrado en %s",path);
	}

	segjulact=time(NULL);	
	// para comprobar cuando llega el 10 minutal exacto
	segjulqm=((segjulact/SEGPQM)*SEGPQM) + SEGPQM;


	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	sprintf(aux,"Test Iniciado:%s",auxch);


			// guardar datos adquiridos en esclavo modbus local
			// primero crear string hexadecimal
			memset((unsigned char *)&table,0,sizeof(table));
			i=0;
			for(sen=0;sen<numSen;sen++){
				valor = sen;
				table[i++] = (unsigned char)*((unsigned char *)&valor);
				table[i++] = (unsigned char)*((unsigned char *)&valor +1);
				table[i++] = (unsigned char)*((unsigned char *)&valor +2);
				table[i++] = (unsigned char)*((unsigned char *)&valor +3);
				printf("\n valoractual[%d] %f \n",sen,valor);
        			for(j=0;j<80;j++){
                			printf("%02x ",table[j]);}
			}
			sprintf(auxch,"/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 4054 -f 16 -q %d -d ",numSen*2);
        		for(j=0;j<numSen*4;j++){
                		printf(" %02x ",table[j]);
				sprintf(caracter,"%02x",table[j]);
				strcat(auxch,caracter);}
			printf("\n\tYUN_DATALOGER: modbus muestreos: %s \n",auxch);
			fflush(stdout);


			system(auxch);

	exit(0);
}

