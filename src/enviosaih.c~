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


static char *version = "1.0.01 (07/05/2015: inicial)";

#define NUMSEN 10
#define NUMCHARCOMMANDS 133
#define LONGTAGS 16

GN gn;
QM qm;
BDCONF BdConf;
IN in;
struct tm *newtime;
unsigned long segjulact,segjulnew,espera,segjulqm,ultimoEnvioFTP;
char aux[192],name[15],path[92],tags_saih[NUMSEN][LONGTAGS];
char *auxch,fecha[15];
short i,j,k,n,ipid,param_enviar[NUMSEN],numSen=0;
struct stat bufstat;
char ipnameFTP[NUMCHARCOMMANDS],directorio[NUMCHARCOMMANDS],usuario[LONGTAGS],contrasenia[LONGTAGS],nombre_fichero[LONGTAGS];

int main(int argc, char *argv[])
{
	short muestreo,adquirirDato=0,guardarDato=0,numSenMB,indicePostScan=0,numDig,numParam=-1;
	int sen,system_err,debug=0;	 
	char ch[NUMCHARCOMMANDS],comando[NUMSEN][NUMCHARCOMMANDS];
	long numMuestreos[NUMSEN];	
	char *auxch;
	FILE *fhf;
	float valor, valoracum[NUMSEN],valoractual[NUMSEN];
	unsigned short valors,estado_digitales[NUMSEN];
	unsigned int valorint;
	unsigned char table[80],caracter[2];

	if ( (argc==2) && (argv[1][1]=='v' || argv[1][0]=='v' || argv[1][1]=='V' || argv[1][0]=='V') ){
		printf("\n************************************");
		printf("\n\t radsys.es");
		printf("\n\t enviosaih Version: %s",version);
		printf("\n************************************\n");
		return(0);
	}

	if (argc==1)	debug=1;


	if( (char *)getenv("SAIHBD") == NULL){
		printf("\n\t enviosaih:Variable Entorno SAIHBD NO SET");
		return(-1);}

	// inicializar 
	for(sen=0;sen<NUMSEN;sen++){
		param_enviar[sen]=-1;
	}

	// leer parámetros de adquisición
	strcat(strcpy(path,(char *)getenv("SAIHBD")),("enviosaih.conf"));
	
	if((fhf=fopen(path,"r"))!=NULL){
		i=0;
		while(fgets(ch,NUMCHARCOMMANDS,fhf)){
			if(debug)
				printf("\n\tenviosaih.conf[%d]=%s",i,ch);
			if(i==2){
				if(sscanf(ch,"%hd",&muestreo)!=1){
					sprintf(aux,"enviosaih.conf error en linea 3: Tiempo de Muestreo: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
					muestreo=1000;
				}
			}
			if(i==6){
				if(sscanf(ch,"%hd",&numSen)!=1){
					sprintf(aux,"enviosaih.conf error en linea 7: Numero de comandos: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
				}
				
			}
//ipnameFTP,directorio,usuario,contrasenia,nombre_fichero;
			if(i==8){
				if(sscanf(ch,"%s",ipnameFTP)!=1){
					sprintf(aux,"enviosaih.conf error en linea 9, ipnameFTP: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
				}
				
			}
			if(i==10){
				if(sscanf(ch,"%s",usuario)!=1){
					sprintf(aux,"enviosaih.conf error en linea 11, usuario: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
				}
				
			}
			if(i==12){
				if(sscanf(ch,"%s",contrasenia)!=1){
					sprintf(aux,"enviosaih.conf error en linea 13, contrasenia: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
				}
				
			}
			if(i==14){
				if(sscanf(ch,"%s",directorio)!=1){
					sprintf(aux,"enviosaih.conf error en linea 15, directorio: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
				}
				
			}
			if(i==16){
				if(sscanf(ch,"%s",nombre_fichero)!=1){
					sprintf(aux,"enviosaih.conf error en linea 17, nombre_fichero: %s",ch);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
				}
				
			}
			if(i>=18){
				strcpy(comando[i-18],ch);
				if(sscanf(ch,"%hd %s",&numParam,tags_saih[i-18])!=2){
					sprintf(aux,"yuncontrol.conf error en param-tag: %s",comando[i-18]);
					AxisLog(aux);				// Log
					if(debug) printf("\n\t  %s \n",aux);
					numParam=-1;					
				}				
				param_enviar[i-18]=numParam-1;
				//strncpy(tags_saih[i-8],comando[i-8]+2,LONGTAGS);
				
			}
			i++;
		}
		indicePostScan=i-19;
		fclose(fhf);
		if(numSen==0 || indicePostScan==0){
			sprintf(aux,"enviosaih.conf error numSen=0 %hd || indicePostScan=0 %hd",numSen,indicePostScan);
			AxisLog(aux);				// Log
			if(debug) printf("\n\t  %s \n",aux);
			exit(0);
		}
	}else{
		sprintf(aux,"enviosaih.conf no encontrado en %s",path);
		printf("\n\t  %s \n",aux);
		AxisLog(aux);
		exit(0);
	}


	if (argc==2 && argv[1][0]=='F'){
		crearFichero(0);
		exit(0);
	}else if (argc==3 && argv[1][0]=='F' && argv[1][1]=='H'){
		if(sscanf(argv[2],"%ld",&segjulqm)!=1){
			printf("\n\tSAIHBD Parametros: enviosaih FH SegJulQm\n");
			exit(0);}
		if( segjulqm < time(NULL) - (NUMHISTQM*SEGPQM)){
			printf("\n\tSAIHBD Fecha fuera de historico\n");
			exit(0);}
		crearFichero(segjulqm);
		exit(0);
	}



	strcpy(name,"enviosaih.pid");				// PID del Proceso
	ipid=getpid();
	PidLog(name,ipid);
	segjulact=time(NULL);	
	// para comprobar cuando llega el 10 minutal exacto
	segjulqm=((segjulact/SEGPQM)*SEGPQM) + SEGPQM;


	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	sprintf(aux,"Proceso enviosaih Iniciado:%s",auxch);
	AxisLog(aux);

	for(;;){  
		segjulact=time(NULL);					// Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		sprintf(ch,"%02d/%02d/%02d %02d:%02d:%02d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900,newtime->tm_hour,newtime->tm_min,newtime->tm_sec);
		printf("\n\tenviosaih:FECHA ACTUAL = %ld  %s ",segjulact,ch);

		printf("\n\tenviosaih:PRE-SCAN: %s ",comando[indicePostScan-1]);
		// comando PRE-SCAN
		system_err=system(comando[indicePostScan-1]);
		if(system_err==-1){
			sprintf(aux,"error system(%s)",comando[indicePostScan-1]);
			AxisLog(aux);				// Log
			printf("\n\t  %s \n",aux);
		}


		// Envio de datos día actual e históricos si procede
		segjulqm=((segjulact/SEGPQM)*SEGPQM);
		EnvioFTP(segjulqm);
		EnvioFTPHist(segjulqm);	// Enviar Historicos si es necesario 

		printf("\n\tenviosaih:POST-SCAN: %s ",comando[indicePostScan]);
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
		if(espera < muestreo)
			TimeWait(10*(muestreo-espera));						// TimeOut

		fflush(stdout);
	}


	exit(0);
}


int GenerarFichero(unsigned long segjul){
	newtime=localtime(&segjul);
	strcpy(aux,"");
	strcpy(name,"");
	printf("\n Inicio Envio FTP: %04d-%02d-%02d %02d:%02d (Ultimo Envio: %ld JulianoActual: %ld [%ld])\n",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,newtime->tm_min,ultimoEnvioFTP,segjulact, segjulact-ultimoEnvioFTP);
	sprintf(aux,"%04d%02d%02d.dat",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday);
	strcpy(name,"/flash/idata/bin/enviosaih F > /flash/idata/saihdat/");
	strcat(name,aux);
	printf("\n %s \n",name);
	system(name);
	
	// procesamos fichero con sed para cambiar punto por coma
	strcpy(name,"");
	strcpy(name,"sed -i '2,$s/\\./,/g' /flash/idata/saihdat/");
	strcat(name,aux);
	printf("\n %s \n",name);
	system(name);

	strcpy(path,("/flash/idata/saihdat/"));				// PATH fichero datos
	strcat(path,aux);
	if(!stat(path,&bufstat)){						// si es menor que 50 bytes (caracteres ASCII) hay error
		if(bufstat.st_size < 50){
			auxch=asctime(newtime);
			sprintf(aux,"%s Error en GenerarFicheroSAIH FTP.",auxch);
			AxisLog(aux);
			AxisLog(name);
			return -1;
			}
	}

	// borramos ficheros dat antiguos
	strcpy(name,"/flash/idata/bin/borrarSaihDat.sh >> /flash/idata/log/axis.log");
	system(name);

	auxch=asctime(newtime);
	sprintf(aux,"GenerarFicheroSAIH OK %s",auxch);
	AxisLog(aux);

	return 0;
}

int GenerarFicheroHist(unsigned long segjul){
	newtime=localtime(&segjul);
	strcpy(aux,"");
	strcpy(name,"");
	printf("\n GenerarFicheroHist %04d-%02d-%02d %02d:%02d (Ultimo Envio: %ld JulianoActual: %ld [%ld])\n",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,newtime->tm_min,ultimoEnvioFTP,segjulact, segjulact-ultimoEnvioFTP);
	sprintf(aux,"%04d%02d%02d.dat",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday);
	sprintf(name,"/flash/idata/bin/enviosaih FH %ld ",segjul);
	strcat(name," > /flash/idata/saihdat/");
	strcat(name,aux);
	printf("\n %s \n",name);
	system(name);

	// procesamos fichero con sed para camiar punto por coma
	strcpy(name,"");
	strcpy(name,"sed -i '2,$s/\\./,/g' /flash/idata/saihdat/");
	strcat(name,aux);
	printf("\n %s \n",name);
	system(name);

	strcpy(path,("/flash/idata/saihdat/"));				// PATH fichero datos
	strcat(path,aux);
	if(!stat(path,&bufstat)){						// si es menor que 50 bytes (caracteres ASCII) hay error
		if(bufstat.st_size < 50){
			auxch=asctime(newtime);
			sprintf(aux,"%s Error en GenerarFicheroSAIH FTP.",auxch);
			AxisLog(aux);
			AxisLog(name);
			return -1;
			}
	}

	// borramos ficheros dat antiguos
	strcpy(name,"/flash/idata/bin/borrarSaihDat.sh >> /flash/idata/log/axis.log");
	system(name);

	auxch=asctime(newtime);
	sprintf(aux,"GenerarFicheroHistSAIH OK %s",auxch);
	AxisLog(aux);

	return 0;
}

int EnviarFichero(unsigned long segjul){
	newtime=localtime(&segjul);
	// Envio FTP comando ejemplo:
	//sftpclient -p 10.253.3.11 -c SIMAM/URDIAIN/ -l urdiain20091028.dat -u SCADA -w SCADA -k /flash/idata/dat/
	strcpy(aux,"");
	strcpy(name,"");
	sprintf(aux,"%04d%02d%02d",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday);
	// con hora:minuto
	//sprintf(name,"/flash/idata/bin/sftpclient -p %s -c SIMAM/%s/ -l %s.dat -u %s -w %s -k /flash/idata/dat/ -d %s_%s%02d%02d.dat",ipnameFTP,directorio,aux,usuario,contrasenia,nombre_fichero,aux,newtime->tm_hour,newtime->tm_min);
	sprintf(name,"/flash/idata/bin/sftpclient -p %s -c %s -l %s.dat -u %s -w %s -k /flash/idata/saihdat/ -d %s_%s.txt",ipnameFTP,directorio,aux,usuario,contrasenia,nombre_fichero,aux);
	sprintf(aux," -T %d -I %d %s ",TIMEOUTFTP,TIMELOGOUTFTP,PASIVO);
	strcat(name,aux);
	strcpy(aux,"2 > /flash/idata/log/envioFTPsaih.log");
	strcat(name,aux);
	printf("\n %s \n",name);
	//system(name);

	// curl -T /tmp/20150112.dat ftp://user:pass@radsys.es/SIMAM/yun/
	sprintf(aux,"%04d%02d%02d",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday);
	sprintf(name,"curl -T /flash/idata/saihdat/%s.dat ftp://%s:%s@%s%s --stderr /flash/idata/log/envioFTPsaih.log --fail --silent --show-error ",aux,usuario,contrasenia,ipnameFTP,directorio);
	printf("\n %s \n",name);
	system(name);

	strcpy(path,("/flash/idata/log/envioFTPsaih.log"));				// PATH envioFTP.log
	if(!stat(path,&bufstat)){						// si es mayor que cero hay error volvemos a intentarlo en 5 minutos
		if(bufstat.st_size > 0){
			auxch=asctime(newtime);
			sprintf(aux,"%s Error en envio FTP SAIH",auxch);
			AxisLog(aux);
			AxisLog(name);
			strcpy(name,"cat /flash/idata/log/envioFTPsaih.log >> /flash/idata/log/axis.log");
			system(name);
			return -1;
			}
	}

	auxch=asctime(newtime);
	sprintf(aux,"EnviarFicheroSAIH OK %s ",auxch);
	AxisLog(aux);

	return 0;
}


void EnvioFTP(unsigned long segjul)
{

	if(GenerarFichero(segjul)<0)	return;
	TimeWait(100);
	if(EnviarFichero(segjul)<0){		
		return;}

	// llegamos hasta aqui, todo correcto con envio actual entonces
	ultimoEnvioFTP=segjul;


	//printf("\n Fin Envio FTP: %04d-%02d-%02d %02d:%02d (Ultimo Envio: %ld JulianoActual: %ld)\n",newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,newtime->tm_min,ultimoEnvioFTP,segjulact);
}

// se llama a envio historico con la fecha actual.
// si lo almacenado en gn.UltEnvFtp es anterior al dia actual se envia el dia correspondiente
// en bloques de NUMDIASREC en NUMDIASREC dias.
// Se almacena gn.UltEnvFtp con la fecha del mediodia del dia correspondiente.
void EnvioFTPHist(unsigned long segjul){
	int numhistqm,dia,i;
	// Envio Historico hasta 30 dias
	newtime=localtime(&segjul);
	numhistqm=(newtime->tm_hour)*60*60 + (newtime->tm_min)*60 + (newtime->tm_sec);	// segundos desde hora 00:00 hasta segjul

	memset((char *)&gn,0,sizeof(GN));
        if( (i=ReadLogerGn(&gn)) !=0)
              printf("\n\tReadLogerGn:Error=%d",i);

	// si no hay valor inicial lo fijamos en el mediodia de NUMDIASREC dias atras
	if(gn.UltEnvFtp==0) gn.UltEnvFtp=segjul - numhistqm - NUMDIASREC*SEGPDIA + SEGPDIA/2;

	for (dia=0; dia < NUMDIASREC; dia++){
		if(gn.UltEnvFtp < (segjul - numhistqm + SEGPDIA/2) ){
			if(GenerarFicheroHist(gn.UltEnvFtp)<0)	return;
			TimeWait(100);
			if(EnviarFichero(gn.UltEnvFtp)<0)	return;
			gn.UltEnvFtp=gn.UltEnvFtp+SEGPDIA;
/*			if( (i=WriteLogerGn(gn)) !=0){
				printf("\n\tWriteLogerGn:Error=%d",i);
				return;
			}
*/		}else{
/****************************************************
Version 1.2 (10/02/2010) se fuerza el envio del ultimo dia completo en el primer envio 
después de medianoche.

			gn.UltEnvFtp=segjul - numhistqm + SEGPDIA/2;
			if( (i=WriteLogerGn(gn)) !=0){
				printf("\n\tWriteLogerGn:Error=%d",i);
				return;
			}
Fin
***************************************************/
			printf("\n\tNo hay ficheros historicos SAIH para enviar (gn.UltEnvFtp: %ld segjul:%ld)",gn.UltEnvFtp,segjul);
			break;
		}
	}
	return;
}



/*
Fichero de texto con lineas tipo:

A;EH03T02NRIO1;02/04/2015 00:00:00;0,38;BUENA
A;EH03T03NRIO2;02/04/2015 00:00:00;,00;MALA

*/
crearFichero(unsigned long segjulqm_aux){

short numhistqm,intervalo;
QM QmHis[NUMHISTQM];
FILE *fh;

			memset((char *)&gn,0,sizeof(GN));
			if( (char *)getenv("SAIHBD") ==NULL){
				fprintf(stderr,"\n\tReadAxisGN:Variable Entorno SAIHBD NO SET");
				return(-1);}
			strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));      // Parametros Generales
			if((fh=fopen(path,"r+b"))==NULL){
				fprintf(stderr,"\nReadAxisGN:No se puede abrir:%s",path);
				return(-1);}
			if(!fread(&gn,sizeof(GN),1,fh)){
				fprintf(stderr,"\nReadAxisGN:Error_En_read:%s Errn=%d",path,errno);
				fclose(fh);
				return(-1);}
			//printf("\n\t\t<b>Ultimo Juliano Recibido: %d</b>\n",gn.segjulhis);
			if(i=ReadLogerBd(&BdConf)!=0){				// Leer Objeto B.D  BDCONF
				printf("\n\tReadAxisBd:Error=%d",i);
				exit(1);}

			memset((char *)&QmHis,0,sizeof(QmHis));
			if( i=ReadLogerQmHis(&QmHis) != 0 ){				// Leemos Historicos QM
				printf("\n\tReadAxisQm:Error=%d",i);
				exit(1);}
			if ( segjulqm_aux==0 ){		// argumentos FH generar fichero historico
				segjulqm_aux=gn.segjulhis;
			}
			newtime=localtime(&segjulqm_aux);

			sprintf(fecha,"%02d/%02d/%04d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900);
			//printf("\nDEBUG: %ld  %s %02d:%02d\n",segjulqm,fecha,newtime->tm_hour,newtime->tm_min);
/*
SIN CABECERAS

			printf("'Nombre: %s Código: %s Fecha: %s Canales:%d IP: %ld.%ld.%ld.%ld",BdConf.remconf.desc,BdConf.remconf.name,fecha,NUMSENANASAICA,gn.iaux[0][0],gn.iaux[0][1],gn.iaux[0][2],gn.iaux[0][3]);
			printf("\n'TIPO;DD/MM/AAAA HH:MM:SS;VALOR;CALIDAD PARAMETROS:");
			for (i=0;i<NUMSENANASAICA;i++)
				printf(";%d,%s,%s",i+1,BdConf.anaconf.tag[i],BdConf.anaconf.uni[i]);
*/
			fflush(stdout);
			
			//printf("\n%s",fecha);
			numhistqm=(newtime->tm_hour)*60+(newtime->tm_min);
			for(i=0;i<NUMHISTQM;i++){
			//for(i=NUMHISTQM-1;i>0;i--){
				if( (QmHis[i].SegJul < (segjulqm_aux - (numhistqm*60))) || (QmHis[i].SegJul > (segjulqm_aux + (SEGPDIA-numhistqm*60) -SEGPQM)) ) continue;	//ultimos dias
				QmHis[i].SegJul+=SEGPQM;
				newtime=localtime(&QmHis[i].SegJul);
	                        sprintf(fecha,"%02d/%02d/%02d %02d:%02d:00",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900,
                                        newtime->tm_hour,newtime->tm_min);

				for(j=0;j<NUMSENANASAICA;j++){
					for(k=0;k<numSen;k++){
						if( j==param_enviar[k] ){
							if(QmHis[i].Flag[j]=='V')
								printf("A;%s;%s;%3.2f;BUENA\r\n",tags_saih[k],fecha,QmHis[i].FlValorAna[j]);
							else
								printf("A;%s;%s;%3.2f;MALA\r\n",tags_saih[k],fecha,QmHis[i].FlValorAna[j]);
						}
					}
				}
			
			}
			printf("\n");
			fflush(stdout);

}

