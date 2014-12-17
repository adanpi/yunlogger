
//Adan para Modbus
#include "modbus_tcp.h"
#include "pantalla.h"
//fin Adan

GN gn;
QM qm;
BDCONF BdConf;


// Adan para modbus
int table[MAX_DATA_LENGTH];
//struct termio tnew,tsaved;
int sfd,result,tabindex;
char aux[80];
// fin Adan



void EnvioModBus(){

	short i;
	unsigned short numTabla = 0;
	unsigned short numWord = 0;
	unsigned short numPts = 0;
	float abscisa = 0;
	float ordenada = 0;
	struct tm *newtime;
	unsigned char *vector;
	char *auxch,fechaqm[SIZE_MB_ANAQM_FECHA];
	float nivel=-1.0,caudal=-1.0;
			// Adan para Modbus
			// una vez leidos los canales analogicos se escriben en la BBDD modbus del axis
		  	// IP esclavo ModBus 

			memset((char *)&gn,0,sizeof(GN));
			if(i=ReadAxisGn(&gn)!=0){				// Leer Objeto B.D  BDCONF
				sprintf(aux,"ReadAxisGn:Error=%d",i);
				AxisLog(aux);
				printf("\n%s\n",aux);
				return;}

			if(i=ReadAxisBd(&BdConf)!=0){				// Leer Objeto B.D  BDCONF
				sprintf(aux,"ReadAxisBd:Error=%d",i);
				AxisLog(aux);
				printf("\n%s\n",aux);
				return;}

			memset((char *)&qm,0,sizeof(QM));
			if(  i=ReadAxisQm((gn.segjulhis),&qm)  != 0 ){			// Leemos QM
				sprintf(aux,"AxisLogos->ReadAxisQm:Error=%d gn1.segjulhis(%ld)",i,gn.segjulhis);
				AxisLog(aux);
				printf("\n%s\n",aux);
				return;
			}

			if(qm.JulQm < time(NULL) - (NUMHISTQM * SEGPQM)){
				sprintf(aux,"ReadAxisQm:Error de Fechas =%ld gn.segjulhis=%ld ",qm.JulQm,gn.segjulhis);
				AxisLog(aux);
				printf("\n%s\n",aux);
				return;
			}			// abrir conexion
  			sfd = set_up_tcp(NUMIPAXIS);				// Comprobar si open sfd 
			if(sfd == -1){
				sprintf(aux,"Error Envio QM Modbus Error conexion TCP Errno=%d",errno);
				AxisLog(aux);
				printf("\n%s\n",aux);
				return;
			}

			fflush(stdout);
			newtime=localtime(&qm.JulQm);
			//auxch=asctime(newtime);
			//sprintf(auxch,"%02d/%02d/%02d %02d:%02d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900,newtime->tm_hour,newtime->tm_min);
			sprintf(fechaqm,"%02d/%02d/%02d %02d:%02d",newtime->tm_mday,newtime->tm_mon+1,newtime->tm_year+1900,newtime->tm_hour,newtime->tm_min);

			printf("\n Fecha Inicio QM %s\n",fechaqm);

			// abrir conexion
  			sfd = set_up_tcp(NUMIPAXIS);				// Comprobar si open sfd 
			if(sfd == -1){
				sprintf(aux,"Error Envio QM Modbus Error conexion TCP Errno=%d",errno);
				AxisLog(aux);
				printf("\n%s\n",aux);
				return;
			}
			//envio modbus QM.Fecha
		  	for(tabindex=0;tabindex < SIZE_MB_ANAQM_FECHA;tabindex++)
				table[tabindex]=(int)fechaqm[tabindex];

			result = escribir_registros_modbus( 1, POS_MB_ANAQM_FECHA, SIZE_MB_ANAQM_FECHA, table, sfd );
			if(result >= 0){		// if no comms erros
				printf("\nEnvio Axis QM.fecha (%s)Modbus OK\n",fechaqm);
				for(tabindex=0;tabindex < NUMSENANA;tabindex++)
					printf("%d ",table[tabindex]);
				printf("\n");
			}else{
				sprintf(aux,"Error Envio QM.fecha Modbus");
				AxisLog(aux);
				printf("\n%s\n",aux);
			}

			//envio modbus QM Valor Ingenieria
			i=0;
		  	for(tabindex=0;tabindex < NUMSENANA;tabindex++){
				nivel = qm.ValorAna[tabindex]*BdConf.anaconf.fcm[tabindex]+BdConf.anaconf.fca[tabindex];
				*((unsigned char *)&table[i]) = (unsigned char)*((unsigned char *)&nivel   );
				*(1+(unsigned char *)&table[i++]) = (unsigned char)*((unsigned char *)&nivel +1);
				*((unsigned char *)&table[i]) = (unsigned char)*((unsigned char *)&nivel +2);
				*(1+(unsigned char *)&table[i++]) = (unsigned char)*((unsigned char *)&nivel +3);
				//printf("\nEnvio Axis QM.Ing %d valor:%f 1(%d) 2(%d)\n",tabindex,nivel,table[i-2],table[i-1]);				
			}
			result = escribir_registros_modbus( 1, POS_MB_ANAQM_ING_ANA1, 2*NUMSENANA, table, sfd );
			if(result >= 0){		// if no comms erros
				printf("Envio Axis QM.Ing 1 Modbus OK\n");
				for(tabindex=0;tabindex < NUMSENANA;tabindex++)
					printf("%d ",table[tabindex]);
				printf("\n");
			}else{
				sprintf(aux,"Error Envio QM.Ing 1 Modbus");
				AxisLog(aux);
				printf("\n%s\n",aux);
			}

			//ioctl(sfd,TCSETA,&tsaved);
			close(sfd);
			// fin Adan
}

// Invierte los bytes de un float
InvFloat(destino,origen)
float * origen;
float * destino;
{
*(3+(unsigned char *)destino) = (unsigned char)*((unsigned char *)origen   );
*(2+(unsigned char *)destino) = (unsigned char)*((unsigned char *)origen +1);
*(1+(unsigned char *)destino) = (unsigned char)*((unsigned char *)origen +2);
*(  (unsigned char *)destino) = (unsigned char)*((unsigned char *)origen +3);
}



