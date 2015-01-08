/*       
*       FILE:           yuncontrol.c
*       AUTHOR:         Adan
*       DATE:           29-12-14
*       REVISION:       1.0
*       PRODUCT:        Arduino Yun DataLoger
*       SUBJECTS:
*       O.S.:           LINUX
*       CUSTOMER:       
*
*              Modificaciones:
*                      29/12/2014      Inicio proyecto
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


static char *version = "1.0.01 (29/12/2014: inicial)";

#define NUMSEN 10
#define NUMCHARCOMMANDS 133

GN gn;
QM qm;
BDCONF BdConf;
IN in;
struct tm *newtime;
unsigned long segjulact,segjulnew,espera,segjulqm;


int main(int argc, char *argv[])
{
	short i,j,n,ipid,muestreo,adquirirDato=0,guardarDato=0,numSen=0,numSenMB,indicePostScan=0,numDig;
	int sen,system_err;
	char name[15],path[92];
	char aux[192],ch[NUMCHARCOMMANDS],comando[NUMSEN][NUMCHARCOMMANDS];
	long numMuestreos[NUMSEN];	
	char *auxch;
	FILE *fhf;
	float valor, valoracum[NUMSEN],valoractual[NUMSEN];
	unsigned short valors,estado_digitales[NUMSEN];
	unsigned int valorint;
	unsigned char table[80],caracter[2];

	if ( (argc==2) && (argv[1][1]=='v')){
		printf("\n************************************");
		printf("\n\t radsys.es");
		printf("\n\t yuncontrol Version: %s",version);
		printf("\n************************************\n");
		return(0);
	}

	strcpy(name,"yuncontrol.pid");				// PID del Proceso
	ipid=getpid();
	PidLog(name,ipid);

	if( (char *)getenv("SAIHBD") == NULL){
		printf("\n\t yunlogger:Variable Entorno SAIHBD NO SET");
		return(-1);}

	// inicializar 
	for(sen=0;sen<NUMSEN;sen++){
		numMuestreos[sen]=0;
		valoracum[sen]=0;
		valoractual[sen]=0;
		estado_digitales[sen]=0;
	}

	// leer parámetros de adquisición
	strcat(strcpy(path,(char *)getenv("SAIHBD")),("yuncontrol.conf"));
	
	if((fhf=fopen(path,"r"))!=NULL){
		i=0;
		while(fgets(ch,NUMCHARCOMMANDS,fhf)){
			printf("\n\tyuncontrol.conf[%d]=%s",i,ch);
			if(i==2){
				if(sscanf(ch,"%hd",&muestreo)!=1){
					sprintf(aux,"yuncontrol.conf error en linea 3: Tiempo de Muestreo: %s",ch);
					AxisLog(aux);				// Log
					printf("\n\t  %s \n",aux);
					muestreo=10;
				}
			}
			if(i==6){
				if(sscanf(ch,"%hd",&numSen)!=1){
					sprintf(aux,"yuncontrol.conf error en linea 7: Numero de comandos: %s",ch);
					AxisLog(aux);				// Log
					printf("\n\t  %s \n",aux);
				}
				
			}
			if(i>=8){
				strcpy(comando[i-8],ch);
			}
			i++;
		}
		indicePostScan=i-9;
		fclose(fhf);
		if(numSen==0 || indicePostScan==0){
			sprintf(aux,"yuncontrol.conf error numSen=0 %hd || indicePostScan=0 %hd",numSen,indicePostScan);
			AxisLog(aux);				// Log
			printf("\n\t  %s \n",aux);
			exit(0);
		}
	}else{
		sprintf(aux,"yuncontrol.conf no encontrado en %s",path);
		printf("\n\t  %s \n",aux);
		AxisLog(aux);
		exit(0);
	}

	segjulact=time(NULL);	
	// para comprobar cuando llega el 10 minutal exacto
	segjulqm=((segjulact/SEGPQM)*SEGPQM) + SEGPQM;


	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	sprintf(aux,"Proceso yuncontrol Iniciado:%s",auxch);
	AxisLog(aux);

	for(;;){  
		segjulact=time(NULL);					// Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		sprintf(ch,"%02d/%02d/%02d %02d:%02d:%02d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900,newtime->tm_hour,newtime->tm_min,newtime->tm_sec);
		printf("\n\tYUN_CONTROL:FECHA ACTUAL = %ld  %s ",segjulact,ch);

			printf("\n\tYUN_CONTROL:PRE-SCAN: %s ",comando[indicePostScan-1]);
			// comando PRE-SCAN
			system_err=system(comando[indicePostScan-1]);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",comando[indicePostScan-1]);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}
			for(sen=0;sen<numSen;sen++){
				printf("\n\tYUN_CONTROL:adquisicion datos %d: %s ",sen,comando[sen]);
				valor=0;
				// verificar si es el comando de adquisición digital
				if(sen==0 && comando[sen][0]=='D'){
					// ver cuantas señales MB a muestrear
					if(sscanf(comando[sen]+1,"%hd",&numSenMB)!=1){
						sprintf(aux,"yuncontrol.conf error en comando D, numDig: %s",comando[sen]);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
						numSenMB=0;
						continue;
					}
					strncpy(auxch,comando[sen] +3,NUMCHARCOMMANDS);
					printf("\n\tYUN_CONTROL: digitales:%d comando %s ",numSenMB,auxch);
					system_err=system(auxch);
					if(system_err==-1){
						sprintf(aux,"error system(%s)",auxch);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
					}
					strcpy(path,"/tmp/digitales");
					if((fhf=fopen(path,"r"))!=NULL){
						while(fgets(name,14,fhf)){
							printf("\n\tYUN_CONTROL: /tmp/digitales: %s ",name);
						}
					}
					fclose(fhf);
					// leer digitales:
					for (i=0;i<numSenMB;i++){
						if( name[i]=='1' )
							valors=1;							
						else
							valors=0;

						printf("\n\tYUN_CONTROL: %hd",valors);
						// comprobar cambios
						if(estado_digitales[i]!=valors){
							printf("\n\tYUN_CONTROL: cambio estado digital %d -> %hd",i+1,valors);
							// guardar incidencia (i+1-> primera digital 1, no 0)
							guardarIncid(i+1,valors);
							//actuarCambioDigital
							for(j=1;j<numSen;j++){
								if(comando[j][0]=='D'){
									// ver señal digital
									if(sscanf(comando[j]+1,"%hd",&numDig)!=1){
									sprintf(aux,"yuncontrol.conf error en actuador D, numDig: %d, %s",i,comando[j]);
									//AxisLog(aux);	// no Log
									printf("\n\t  %s \n",aux);
									continue;
									}
									// ver cambio señal digital si es la señal que nos interesa
									if( numDig==i && sscanf(comando[j]+3,"%hd",&numDig)==1){
										// si es el cambio de estado se aplica el comando
										if(numDig==valors){
											strncpy(auxch,comando[j] +5,NUMCHARCOMMANDS);
											printf("\n\tYUN_CONTROL: digitales:%d comando %s ",i,auxch);
											system_err=system(auxch);
											if(system_err==-1){
												sprintf(aux,"error system(%s)",auxch);
												AxisLog(aux);				// Log
												printf("\n\t  %s \n",aux);
											}
										}

									}
								}
							}
						}
						//almacenar cambio
						estado_digitales[i]=valors;
					}
					//envio ModBus
					// /radsys/pollmb/pollmb.py -a 0 -f 15 -d $d13 -q 3
					sprintf(auxch,"/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 0 -f 15 -q %d -d ",numSenMB);
        				for(j=0;j<8;j++){                				
						sprintf(caracter,"%01d",estado_digitales[j]);
						strcat(auxch,caracter);}
					printf("\n%s\n",auxch);
					fflush(stdout);
		
					system_err=system(auxch);
					if(system_err==-1){
						sprintf(aux,"error system(%s)",auxch);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
					}
					// fin modbus
				// comando servo
				}else if(comando[sen][0]=='S'){
					strncpy(auxch,comando[sen] +1,NUMCHARCOMMANDS);
					printf("\n\tYUN_CONTROL: servo comando %s ",auxch);
					system_err=system(auxch);
					if(system_err==-1){
						sprintf(aux,"error system(%s)",auxch);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
					}
				}else{

				}
			}
			printf("\n\tYUN_CONTROL:POST-SCAN: %s ",comando[indicePostScan]);
			// comando POST-SCANConfTty			
			system_err=system(comando[indicePostScan]);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",comando[indicePostScan]);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}



		segjulnew=time(NULL);	

		espera=segjulnew-segjulact;

		// como no sabemos el tiempo de la adquisición de la señal
		// si ha pasado menos tiempo que el muestreo ajustamos la espera
		// en caso contrario seguimos con el siguiente scan.
		if(espera*10 < muestreo)
			TimeWait(muestreo-10*espera);						// TimeOut

		fflush(stdout);
	}


	exit(0);
}


void guardarIncid(int i, short estado){
	unsigned char auxchar;
	unsigned short valors;
	int j;
	char * auxch;

	printf("\n\tYUN_CONTROL: guardar incid %d -> %hd",i,estado);
	if( (j=ReadLogerIn(&in)) != 0){				//Axis Incidencias
		printf("\n\tReadLogerIn:No Read:Error=%d",j);
		sprintf(auxch,"ReadLogerIn:No Read:Error=%d",j);
		AxisLog(auxch);                                           // Log
		return;
	}

	in.IndUltIn=in.IndUltIn+1;
	segjulact=time(NULL);					// Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);

	in.SegJulIn[in.IndUltIn]=segjulact;
	in.NumSen[in.IndUltIn]=i;
	in.Estado[in.IndUltIn]=estado;
	//crear BufferIn, primero se copia la fecha
	segjulact=htonl(in.SegJulIn[in.IndUltIn]-SEGJULCTESAC);
	memcpy(in.BufferIn[in.IndUltIn],(char *)&segjulact,4);
	//el bit alto primer byte (numero señal -1 (¿¿FE??) estado de la señal
	valors=in.NumSen[in.IndUltIn] - 1;
	memcpy((char *)&auxchar,(char *)&valors,1);
	if ( in.Estado[in.IndUltIn] == 1){
		auxchar |= 0x80;
		printf("\n\tCrearBufferIn Estado 1\t %02x",auxchar);
	}else{ 
		printf("\n\tCrearBufferIn Estado 0\t %02x",auxchar);
	}
	//se asigna el primer byte modificado numero se señal y estado
	memcpy((char *)in.BufferIn[in.IndUltIn],(char *)&auxchar,1);

	printf("\n\tCrearBufferIn \t");
		for(j=0;j<4;j++)
		printf("%02x ",in.BufferIn[in.IndUltIn][j]);
	
	printf("\n\t\tFecha Incidencia(%d): %s \t Señal: %d \t Estado: %d \n",i,auxch,in.NumSen[in.IndUltIn],in.Estado[in.IndUltIn]);
	
	in.IndAct=in.IndUltIn;

	if(in.IndAct > VALMAXIND)		
		in.IndAct = in.IndAct - VALMAXIND -1;

	if(in.IndAct < 0) in.IndAct = in.IndAct + VALMAXIND + 1;		// + IndAct NO Negativo

	in.IndUltIn=in.IndAct;

	in.NumInAlm=in.IndAct;

	in.ValMaxInd=NUMINALM;			// (NUMINALM o VALMAXIN) ???

	if(i=WriteLogerIn(in) !=0)
		printf("\n\tWriteLogerIn:No Write");
	printf("\n\tAXIS:IndAct=%hd (%0x) IndUltIn=%hd (%0x) NumInAlm=%hd (%0x)",
	in.IndAct,in.IndAct,in.IndUltIn,in.IndUltIn,in.NumInAlm,in.NumInAlm);
	printf("\n\t------------------------------------------------------------------------------");

}

