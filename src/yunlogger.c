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
#define NUMCHARCOMMANDS 133

GN gn;
QM qm;
BDCONF BdConf;

int main(int argc, char *argv[])
{
	short i,j,n,ipid,muestreo,adquirirDato=0,guardarDato=0,numSen=0,numSenMB,indicePostScan=0;
	int sen,system_err;
	char name[15],path[92];
	char aux[192],ch[NUMCHARCOMMANDS],comando[NUMSEN][NUMCHARCOMMANDS];
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
		printf("\n\t yunlogger Version: %s",version);
		printf("\n\t\t + PRE y POST SCAN");
		printf("\n************************************\n");
		return(0);
	}

	strcpy(name,"yunlogger.pid");				// PID del Proceso
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
	}

	// leer parámetros de adquisición
	strcat(strcpy(path,(char *)getenv("SAIHBD")),("yunlogger.conf"));
	
	if((fhf=fopen(path,"r"))!=NULL){
		i=0;
		while(fgets(ch,NUMCHARCOMMANDS,fhf)){
			printf("\n\tyunlogger.conf[%d]=%s",i,ch);
			if(i==2){
				if(sscanf(ch,"%hd",&muestreo)!=1){
					sprintf(aux,"yunlogger.conf error en linea 3: Tiempo de Muestreo: %s",ch);
					AxisLog(aux);				// Log
					printf("\n\t  %s \n",aux);
					muestreo=10;
				}
			}
			if(i==6){
				if(sscanf(ch,"%hd",&numSen)!=1){
					sprintf(aux,"yunlogger.conf error en linea 7: Numero de señales: %s",ch);
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
			sprintf(aux,"yunlogger.conf error numSen=0 %hd || indicePostScan=0 %hd",numSen,indicePostScan);
			AxisLog(aux);				// Log
			printf("\n\t  %s \n",aux);
			exit(0);
		}
	}else{
		sprintf(aux,"yunlogger.conf no encontrado en %s",path);
		printf("\n\t  %s \n",aux);
		AxisLog(aux);
		exit(0);
	}

	segjulact=time(NULL);	
	// para comprobar cuando llega el 10 minutal exacto
	segjulqm=((segjulact/SEGPQM)*SEGPQM) + SEGPQM;


	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	sprintf(aux,"Proceso yunlogger Iniciado:%s",auxch);
	AxisLog(aux);

	for(;;){  
		segjulact=time(NULL);					// Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		sprintf(ch,"%02d/%02d/%02d %02d:%02d:%02d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900,newtime->tm_hour,newtime->tm_min,newtime->tm_sec);
		printf("\n\tYUN_DATALOGER:FECHA ACTUAL = %ld  %s ",segjulact,ch);

		// comprobar cuando llega el 10 minutal exacto para iniciar el muestreo si es el primero
		// o guardar el 10 minutal si no es el primero
		if(segjulact>=segjulqm && adquirirDato==0){
			adquirirDato=1;
		}else if(segjulact>=segjulqm && adquirirDato==1){
			guardarDato=1;			
		}

		if(adquirirDato==1){
			printf("\n\tYUN_DATALOGER:PRE-SCAN: %s ",comando[indicePostScan-1]);
			// comando PRE-SCAN
			system_err=system(comando[indicePostScan-1]);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",comando[indicePostScan-1]);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}
			for(sen=0;sen<numSen;sen++){
				printf("\n\tYUN_DATALOGER:adquisicion datos %d: %s ",sen,comando[sen]);

				// verificar si es un comando de adquisición múltiple modbus
				if(comando[sen][0]=='M'){
					// ver cuantas señales MB a muestrear
					if(sscanf(comando[sen]+1,"%hd",&numSenMB)!=1){
						sprintf(aux,"yunlogger.conf error en comando MB, numsenMB: %s",comando[sen]);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
						numSenMB=0;
					}
					strncpy(auxch,comando[sen] +3,NUMCHARCOMMANDS);
					printf("\n\tYUN_DATALOGER: esclavo modbus numSenMB:%d comando %s ",numSenMB,auxch);
					system_err=system(auxch);
					if(system_err==-1){
						sprintf(aux,"error system(%s)",auxch);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
					}
					// para cada señal ModBus el comando debe dar su valor ingeniería en /tmp/yunlogger.NUMSEN
					for(j=0;j<numSenMB;j++){
						strcpy(path,"/tmp/yunlogger");
						sprintf(aux,".%d",(sen+1));
						strcat(path,aux);
						if((fhf=fopen(path,"r"))!=NULL){
							while(fgets(name,14,fhf)){
							printf("\n\tYUN_DATALOGER: /tmp/yunlogger: %s ",name);
							}
						}
						fclose(fhf);
			
						if( sscanf(name,"%f",&valor) != 1 ){
							printf("\n\tyunlogger error convertir valor %s\n",name);
						}
			
						printf("\n\tYUN_DATALOGER: valor:%f num:%d acum:%f",valor, numMuestreos[sen], valoracum[sen]);
						valoracum[sen]+=valor;
						valoractual[sen]=valor;
						numMuestreos[sen]++;
		
					}
		
				}else{
					system_err=system(comando[sen]);
					if(system_err==-1){
						sprintf(aux,"error system(%s)",comando[sen]);
						AxisLog(aux);				// Log
						printf("\n\t  %s \n",aux);
					}		
					strcpy(path,"/tmp/yunlogger");
					sprintf(aux,".%d",(sen+1));
					strcat(path,aux);
					if((fhf=fopen(path,"r"))!=NULL){
						while(fgets(name,14,fhf)){
							printf("\n\tYUN_DATALOGER: /tmp/yunlogger: %s ",name);
						}
					}
					fclose(fhf);
	
					if( sscanf(name,"%f",&valor) != 1 ){
						printf("\n\tyunlogger error convertir valor %s\n",name);
					}
	
					printf("\n\tYUN_DATALOGER: valor:%f num:%d acum:%f",valor, numMuestreos[sen], valoracum[sen]);
					valoracum[sen]+=valor;
					valoractual[sen]=valor;
					numMuestreos[sen]++;
				}
			}
			printf("\n\tYUN_DATALOGER:POST-SCAN: %s ",comando[indicePostScan]);
			// comando POST-SCANConfTty			
			system_err=system(comando[indicePostScan]);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",comando[indicePostScan]);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}
			// guardar datos adquiridos en esclavo modbus local

			//envio modbus QM.Fecha

			sprintf(auxch,"/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 4026 -f 16 -q 20 -d ");
     			for(j=0;j<20;j++){
                		//printf(" %04x ",ch[j]);
				sprintf(caracter,"%04x",ch[j]);
				strcat(auxch,caracter);}
			
			printf("\n%s\n",auxch);
			fflush(stdout);			
			system_err=system(auxch);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",auxch);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}

			memset((unsigned char *)&table,0,sizeof(table));
			i=0;
			for(sen=0;sen<numSen;sen++){
				valor = valoractual[sen];
				table[i++] = (unsigned char)*((unsigned char *)&valor);
				table[i++] = (unsigned char)*((unsigned char *)&valor +1);
				table[i++] = (unsigned char)*((unsigned char *)&valor +2);
				table[i++] = (unsigned char)*((unsigned char *)&valor +3);
				//printf("\n valoractual[%d] %f \n",sen,valor);
        			//for(j=0;j<80;j++){
                		//	printf("%02x ",table[j]);}
			}
			sprintf(auxch,"/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 4054 -f 16 -q %d -d ",numSen*2);
        		for(j=0;j<numSen*4;j++){
                		printf(" %02x ",table[j]);
				sprintf(caracter,"%02x",table[j]);
				strcat(auxch,caracter);}
			printf("\n\tYUN_DATALOGER: modbus muestreos: %s \n",auxch);

			printf("\n%s\n",auxch);
			fflush(stdout);

			system_err=system(auxch);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",auxch);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}
			// fin modbus
		}

		if(guardarDato==1){

			memset((char *)&gn,0,sizeof(GN));
			// leer gn
	        	if( (i=ReadLogerGn(&gn)) !=0)
		              printf("\n\tReadLogerGn:Error=%d",i);

			gn.segjulhis=segjulqm-SEGPQM;		// Ultimo Juliano Recibido apl web

			// Se leen factores analogicas de la BdConf
			memset((char *)&BdConf,0,sizeof(BDCONF));

			if( (i=ReadLogerBd(&BdConf)) !=0){	/*Leer Objeto B.D  BDCONF */
				printf("\n\t YUN_DATALOGER Guardar Error lectura BdConfig:Error=%d",i);
			}

			memset((char *)&qm,0,sizeof(qm));

			newtime=localtime(&segjulqm);
			auxch=asctime(newtime);
			printf("\n\t \t YUN_DATALOGER Guardar :FECHA QM = %ld  %s ",segjulqm,auxch);
			memset((char *)&qm,0,sizeof(qm));				// Objeto QM
			for (sen=0;sen<NUMSENANA;sen++)	qm.Flag[sen]='N';		//inicializamos Flag SAICA como 'N' (NO VALIDO)
			qm.Status=512;

			qm.SegJul=segjulqm-SEGPQM;			//Seg Jul Unix QM recuperado (inicio del QM)
			qm.SegJulPer=segjulqm-SEGPQM;
			qm.NumAna=numSen;					
		


			for(sen=0;sen<qm.NumAna;sen++){
				if(numMuestreos[sen]>0){
					valor=valoracum[sen]/numMuestreos[sen];
					qm.Flag[sen]='V';
				}else
					valor=-9999.99;
				printf("\n\tYUN_DATALOGER: Guardar: valor Medio: %f ",valor);
				qm.FlValorAna[sen]=valor;
				//cambio a cuentas short protocolo SAC
				if (BdConf.anaconf.fcm[sen] ==  0) {
					sprintf(aux,"Factor multiplicativo = 0, senal [%d] %s \n",sen,BdConf.anaconf.tag[sen]);
//					AxisLog(aux);				// Log
					printf("\n\t%s",aux);
					valorint = 0;
					//qm.FlValorAna[sen]=-9999.99;
				}else
					valorint=(unsigned int)(valor/BdConf.anaconf.fcm[sen] - BdConf.anaconf.fca[sen]/BdConf.anaconf.fcm[sen]);
				if (valorint < 0 || valorint > 65535){
					sprintf(aux,"Valor Analogico fuera de rango Proto SAC Sen: %d Valor:%.2f\n",sen,valor);
//					AxisLog(aux);				// Log
					printf("\n\t%s\n",aux);
					valorint = 0;
					//qm.FlValorAna[sen]=-9999.99;
				}
				valors=(unsigned short)valorint;
			        qm.ValorAna[sen]=valors;
				numMuestreos[sen]=0;
				valoracum[sen]=0;
				valoractual[sen]=0;
			}

			qm.NumCont=NUMSENCONT;	// no hay contadores cincominutales
			qm.NumGray=NUMSENGRAY;
			qm.NumRs=NUMSENRS;
	
			if( (j=CrearBufferQm(&qm,BdConf.remconf.ihw))  !=0){
				printf("\n\tCrearBufferQm:Error=%d",j);
				}
			if( (j=WriteLogerQm(qm)) !=0){				// Escribimos el QM Modificado
				printf("\n\tWriteLogerQm:Error=%d",j);
				}
			if( (j=WriteLogerGn(gn)) !=0)
				printf("\n\tWriteLogerGn:Error=%d",j);


			// guardar QM en esclavo modbus local
			// primero crear string hexadecimal
			memset((unsigned char *)&table,0,sizeof(table));
			i=0;
			for(sen=0;sen<numSen;sen++){
				valor = qm.FlValorAna[sen];
				table[i++] = (unsigned char)*((unsigned char *)&valor);
				table[i++] = (unsigned char)*((unsigned char *)&valor +1);
				table[i++] = (unsigned char)*((unsigned char *)&valor +2);
				table[i++] = (unsigned char)*((unsigned char *)&valor +3);
				printf("\n valorqm[%d] %f \n",sen,valor);
        			for(j=0;j<80;j++){
                			printf("%02x ",table[j]);}
			}
			sprintf(auxch,"/radsys/pollmb/pollmb.py -h 127.0.0.1 -a 3054 -f 16 -q %d -d ",numSen*2);
        		for(j=0;j<numSen*4;j++){
                		printf(" %02x ",table[j]);
				sprintf(caracter,"%02x",table[j]);
				strcat(auxch,caracter);}
			printf("\n\tYUN_DATALOGER: modbus qm: %s \n",auxch);

			printf("\n%s\n",auxch);
			fflush(stdout);

			system_err=system(auxch);
			if(system_err==-1){
				sprintf(aux,"error system(%s)",auxch);
				AxisLog(aux);				// Log
				printf("\n\t  %s \n",aux);
			}
			// fin modbus
			
			guardarDato=0;
			segjulqm=segjulqm+SEGPQM;
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

