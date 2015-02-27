/*
 ============================================================================
 Name        : axismodbus.c
 Author      : Adan
 Version     :
 Copyright   : IData Sistemas
 Description : Axis ModBusTCP Esclavo
 PENDIENTE	 : Crear fichero con pid y actuar ante señales
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "modbus.h"
#include "modbus_tcp.h"
#include "logersaihbd.h"
#include "pantalla.h"

char aux[64];
unsigned long segjulact;
struct tm *newtime;
char *auxch;
BDCONF BdConf;

int main(int argc, char *argv[])
{
	int device=-1;
	int socket=-1;
	int ComTcp=0,ComSerie=0;
	short ipid;
	char name[20];


    segjulact=time(NULL);                                   // Hora Actual
    newtime=localtime(&segjulact);
    auxch=asctime(newtime);
    sleep(10);

	Mbs_data=(unsigned short *)malloc(BBDDMODBUSLON*sizeof(unsigned short));	/* allocate the modbus database */
	printf("\nIdata Axis ModBus RTU + TCP Esclavo Servidor Version %s\n",VERSION); /* prints Idata  */
	if (argc == 4){
		if ( (sscanf(argv[1],"%d",&ComSerie) != 1) || (sscanf(argv[2],"%d",&ComTcp) != 1) || (sscanf(argv[3],"%d",&Mb_verbose) != 1) ){
			puts("Error en parametros entrada");
			puts("Parametros axismbus ComSerie(Si=1,No=0) ComTcp(Si=1,No=0) InfoDebug((Si=1,No=0))");
			puts("Ejemplo: #axismbus 0 1 1");
	        sprintf(aux,"%s\tProceso axismbus Error en argumentos",auxch);
	        AxisLog(aux);
			return EXIT_FAILURE;
		}
	}else{
		puts("Parametros axismbus ComSerie(Si=1,No=0) ComTcp(Si=1,No=0) InfoDebug((Si=1,No=0))");
		puts("Ejemplo: #axismbus 0 1 1");
		sprintf(aux,"%s\tProceso axismbus Faltan argumentos",auxch);
        AxisLog(aux);
		return EXIT_SUCCESS;
	}




	Mb_slave_init();	//iniciar cerrojos sincronización hilos

	//IniAxisBd();

	//ConfiguracionInicialBdRemota();

	//CopiaBdRemotaModBus();

	if(ComTcp){
		socket=set_up_tcp_slave();
		if(socket >0){
			Mb_slave_tcp(socket,ESCLAVO);
		}
		else{
			fprintf(stderr,"No se ha podido establecer socket: error %d\n",socket);
			sprintf(aux,"%s\t axismbus:No se ha podido establecer socket (%d): error %d\n",auxch,socket,errno);
	        AxisLog(aux);
			Mb_slave_tcp_stop();
			Mb_close_socket(socket);
			return EXIT_FAILURE;
		}
	}

	if(ComSerie){
		device=Mb_open_device(PUERTOSERIE,VELOCIDAD,PARIDAD,BITDATOS,BITSTOP);	   /* abrir puerto serie */
		if(device >0){
			Mb_slave_rtu(device,ESCLAVO,NULL,Mb_slave_recibido_char,Mb_slave_fin_envio);			/* start slave thread with slave number=1*/
		}
		else{
			fprintf(stderr,"No se ha podido abrir puerto serie: error %d\n",device);
			sprintf(aux,"%s\t axismbus:No se ha podido abrir puerto serie (%d): error %d\n",auxch,device,errno);
	        AxisLog(aux);
			Mb_slave_rtu_stop();									/* kill slave thread */
			Mb_close_device(device);							/* close device */
			return EXIT_FAILURE;
		}
	}

	// Hilo hora modbus
	Mb_slave_hora();

        strcpy(name,"axismbus.pid");				// PID del Proceso
        ipid=getpid();
        PidLog(name,ipid);
        sprintf(aux,"%s\tAXPLC-100:kthread-axismbus OK!",auxch);
        AxisLog(aux);
	
	if(ComTcp)
		pthread_join(Mbs_thread_tcp,NULL);
	if(ComSerie)
		pthread_join(Mbs_thread_rtu,NULL);
	pthread_join(Mbs_thread_hora,NULL);

	//getchar();							/* hit <return> to stop program */
	//Mb_slave_rtu_stop();						/* kill slave thread */
	//Mb_slave_tcp_stop();
	//Mb_close_device(device);					/* close device */
	//Mb_close_socket(socket);
	return EXIT_SUCCESS;
}

/*
void ConfiguracionInicialBdRemota(){
	short i;
	printf("\n\tConfiguracion Basica de la Estacion Remota");
	if((i=ReadLogerBd(&BdConf))!=0){			// Leer Objeto B.D  BDCONF
		printf("\n\tReadLogerBd:Error=%d",i);
		sprintf(aux,"%s\tProceso axismbus ReadAxisBd:Error=%d",auxch,i);
        AxisLog(aux);
		exit(1);}

	//exit(1);
	printf("\n\t\tNOMBRE: %4s:",BdConf.remconf.name);
	printf("\n\t\tDESCRIPCION: %s",BdConf.remconf.desc);
	printf("\n\t\tIHW:%d",BdConf.remconf.ihw);
	printf("\n\t\tIP (PPP):%s",BdConf.remconf.ipname);
	printf("\n\t\tIP (ETHERNET):%s",BdConf.remconf.ipname1);
	printf("\n\t\tHoras Funcionamiento:%ld %s \n\n",BdConf.remconf.segjulfun,"seg");

	short ihw=7;
	strcpy(BdConf.remconf.name,"axion");
	strcpy(BdConf.remconf.desc,"ESTACION IDATA AXPLC");
	BdConf.remconf.ihw=ihw;
	strcpy(BdConf.remconf.ipname,"10.8.1.1");
	strcpy(BdConf.remconf.ipname1,"192.168.1.90");
	BdConf.remconf.segjulfun=0;

	strcpy(BdConf.anaconf.tag[A1],"ID01X01TEMP");					// Primera Analogica
	strcpy(BdConf.anaconf.tag[A2],"ID01X02NRIO");					// Primera Analogica
	strcpy(BdConf.anaconf.tag[A3],"ID01X03HUME");					// Primera Analogica
	strcpy(BdConf.anaconf.tag[A4],"ID01X04PRES");					// Primera Analogica
	strcpy(BdConf.anaconf.tag[C1],"ID01X01CONT");					// Primera Analogica
	strcpy(BdConf.anaconf.tag[C2],"ID01X02CONT");					// Primera Analogica
	strcpy(BdConf.anaconf.tag[Calc1],"ID01X01QRIO1");				// Primera Analogica
	strcpy(BdConf.anaconf.tag[Calc2],"ID01X02QRIO2");				// Primera Analogica
	strcpy(BdConf.anaconf.tag[AOut1],"ID01X01AOUT1");				// Primera Analogica
	strcpy(BdConf.anaconf.tag[AOut2],"ID01X02AOUT2");				// Primera Analogica
	strcpy(BdConf.anaconf.tag[AOut3],"ID01X03AOUT3");				// Primera Analogica
	strcpy(BdConf.anaconf.tag[AOut4],"ID01X04AOUT4");				// Primera Analogica

	strcpy(BdConf.anaconf.desc[A1],"ANALOGICA NUMERO 1");
	strcpy(BdConf.anaconf.desc[A2],"ANALOGICA NUMERO 2");
	strcpy(BdConf.anaconf.desc[A3],"ANALOGICA NUMERO 3");
	strcpy(BdConf.anaconf.desc[A4],"ANALOGICA NUMERO 4");
	strcpy(BdConf.anaconf.desc[C1],"CONTADOR NUMERO 1");
	strcpy(BdConf.anaconf.desc[C2],"CONTADOR NUMERO 2");
	strcpy(BdConf.anaconf.desc[Calc1],"CALCULADA NUMERO 1");
	strcpy(BdConf.anaconf.desc[Calc1],"CALCULADA NUMERO 1");
	strcpy(BdConf.anaconf.desc[AOut1],"ANALOGICA DE SALIDA NUMERO 1");
	strcpy(BdConf.anaconf.desc[AOut2],"ANALOGICA DE SALIDA NUMERO 2");
	strcpy(BdConf.anaconf.desc[AOut3],"ANALOGICA DE SALIDA NUMERO 3");
	strcpy(BdConf.anaconf.desc[AOut4],"ANALOGICA DE SALIDA NUMERO 4");

	strcpy(BdConf.anaconf.uni[A1],"ºC");
	strcpy(BdConf.anaconf.uni[A2],"m");
	strcpy(BdConf.anaconf.uni[A3],"%");
	strcpy(BdConf.anaconf.uni[A4],"%");
	strcpy(BdConf.anaconf.uni[C1],"l/m2");
	strcpy(BdConf.anaconf.uni[C2],"l/m2");
	strcpy(BdConf.anaconf.uni[Calc1],"m3/seg");
	strcpy(BdConf.anaconf.uni[Calc2],"%");
	strcpy(BdConf.anaconf.uni[AOut1],"%");
	strcpy(BdConf.anaconf.uni[AOut2],"%");
	strcpy(BdConf.anaconf.uni[AOut3],"%");
	strcpy(BdConf.anaconf.uni[AOut4],"%");

	BdConf.anaconf.fcm[A1]=1.0;
	BdConf.anaconf.fcm[A2]=1.0;
	BdConf.anaconf.fcm[A3]=1.0;
	BdConf.anaconf.fcm[A4]=1.0;
	BdConf.anaconf.fcm[C1]=1.0;
	BdConf.anaconf.fcm[C2]=1.0;
	BdConf.anaconf.fcm[Calc1]=1.0;
	BdConf.anaconf.fcm[Calc2]=1.0;

	BdConf.anaconf.fca[A1]=0.0;
	BdConf.anaconf.fca[A2]=0.0;
	BdConf.anaconf.fca[A3]=0.0;
	BdConf.anaconf.fca[A4]=0.0;
	BdConf.anaconf.fca[C1]=0.0;
	BdConf.anaconf.fca[C2]=0.0;
	BdConf.anaconf.fca[Calc1]=0.0;
	BdConf.anaconf.fca[Calc2]=0.0;

	BdConf.anaconf.NumAna = 4;				//NUMSENANA;
	BdConf.anaconf.NumGray = NUMSENGRAY;
	BdConf.anaconf.NumRs = NUMSENRS;
	BdConf.anaconf.NumCont = NUMSENCONT;
	BdConf.anaconf.NumCalc = NUMSENCALC;
	BdConf.anaconf.NumOut = NUMSENANAOUT;

	if(i=WriteLogerBd(BdConf)!=0){				// Escribir Objeto B.D  BDCONF
		printf("\n\tWriteLogerBD:Error=%d",i);
		sprintf(aux,"%s\tProceso axismbus WriteAxisBD:Error=%d",auxch,i);
        AxisLog(aux);
		exit(1);}
}
*/

void CopiaBdRemotaModBus(){
	int i;
	printf("\n\tCopia Configuracion Basica de la Estacion Remota a bbdd ModBus");
	i=ReadLogerBd(BdConf);
	if(i!=0){			// Leer Objeto B.D  BDCONF
		printf("\n\tReadAxisBd:Error=%d",i);
		sprintf(aux,"%s\tProceso axismbus ReadAxisBd:Error=%d",auxch,i);
        AxisLog(aux);
		exit(1);}

	if (Mb_verbose){
		printf("\n\t\tNOMBRE: %4s:",BdConf.remconf.name);
		printf("\n\t\tDESCRIPCION: %s",BdConf.remconf.desc);
		printf("\n\t\tIHW:%d",BdConf.remconf.ihw);
//		printf("\n\t\tIP (PPP):%s",BdConf.remconf.ipname);
//		printf("\n\t\tIP (ETHERNET):%s",BdConf.remconf.ipname1);
//		printf("\n\t\tHoras Funcionamiento:%ld %s \n\n",BdConf.remconf.segjulfun,"seg");
	}

	memcpy((char *)&Mbs_data[POS_MB_BDCONF_REMCONF_NAME],(char *)BdConf.remconf.name,2*SIZE_MB_BDCONF_REMCONF_NAME);
	memcpy((char *)&Mbs_data[POS_MB_BDCONF_REMCONF_DESC],(char *)BdConf.remconf.desc,2*SIZE_MB_BDCONF_REMCONF_DESC);
	Mbs_data[POS_MB_BDCONF_REMCONF_HW]=BdConf.remconf.ihw;
/*	memcpy((char *)&Mbs_data[POS_MB_BDCONF_REMCONF_IP],(char *)BdConf.remconf.ipname,2*SIZE_MB_BDCONF_REMCONF_IP);
	memcpy((char *)&Mbs_data[POS_MB_BDCONF_REMCONF_IP1],(char *)BdConf.remconf.ipname1,2*SIZE_MB_BDCONF_REMCONF_IP1);
	memcpy((char *)&Mbs_data[POS_MB_BDCONF_REMCONF_IP2],(char *)BdConf.remconf.ipname2,2*SIZE_MB_BDCONF_REMCONF_IP2);
*/


	for (i=0 ; i< NUMSENANA ; i++){
		memcpy((char *)&Mbs_data[POS_MB_BDCONF_ANACONF_TAG1 + (SIZE_MB_BDCONF_ANACONF*i)],(char *)BdConf.anaconf.tag[A1+i],2*SIZE_MB_BDCONF_ANACONF_TAG1);
		memcpy((char *)&Mbs_data[POS_MB_BDCONF_ANACONF_DESC1 + (SIZE_MB_BDCONF_ANACONF*i)],(char *)BdConf.anaconf.desc[A1+i],2*SIZE_MB_BDCONF_ANACONF_DESC1);
		memcpy((char *)&Mbs_data[POS_MB_BDCONF_ANACONF_UNI1 + (SIZE_MB_BDCONF_ANACONF*i)],(char *)BdConf.anaconf.uni[A1+i],2*SIZE_MB_BDCONF_ANACONF_UNI1);
		Float2Bytes(ISBIGENDIAN,BdConf.anaconf.fcm[A1+i],&Mbs_data[POS_MB_BDCONF_ANACONF_FCM1+ (SIZE_MB_BDCONF_ANACONF*i)]);
		Float2Bytes(ISBIGENDIAN,BdConf.anaconf.fca[A1+i],&Mbs_data[POS_MB_BDCONF_ANACONF_FCA1+ (SIZE_MB_BDCONF_ANACONF*i)]);
	}

        for (i=0 ; i< (NUMSENDIG) ; i++){
                memcpy((char *)&Mbs_data[POS_MB_BDCONF_DIGCONF_TAG1 + (SIZE_MB_BDCONF_DIGCONF*i)],(char *)BdConf.digconf.tag[D1+i],2*SIZE_MB_BDCONF_DIGCONF_TAG1);
                memcpy((char *)&Mbs_data[POS_MB_BDCONF_DIGCONF_DESC1 + (SIZE_MB_BDCONF_DIGCONF*i)],(char *)BdConf.digconf.desc[D1+i],2*SIZE_MB_BDCONF_DIGCONF_DESC1);
                memcpy((char *)&Mbs_data[POS_MB_BDCONF_DIGCONF_ETI01 + (SIZE_MB_BDCONF_DIGCONF*i)],(char *)BdConf.digconf.etiqueta0[D1+i],2*SIZE_MB_BDCONF_DIGCONF_ETI01);
                memcpy((char *)&Mbs_data[POS_MB_BDCONF_DIGCONF_ETI11 + (SIZE_MB_BDCONF_DIGCONF*i)],(char *)BdConf.digconf.etiqueta1[D1+i],2*SIZE_MB_BDCONF_DIGCONF_ETI11);
//		Mbs_data[POS_MB_BDCONF_DIGCONF_NUMDIG1 + (SIZE_MB_BDCONF_DIGCONF*i)]=(unsigned short)BdConf.digconf.Estado[D1+i];
//		memcpy((char *)&Mbs_data[POS_MB_BDCONF_DIGCONF_NUMDIG1 + (SIZE_MB_BDCONF_DIGCONF*i)],(char *)BdConf.digconf.Estado[D1+i],2*SIZE_MB_BDCONF_DIGCONF_ETI11);
        }


	// Simular primeros valores minutales

	memcpy((char *)&Mbs_data[POS_MB_ANAQM_FECHA],"24/02/1973 08:24",SIZE_MB_ANAQM_FECHA);
	Mbs_data[POS_MB_ANAMIN_ANA1]=101;
	Mbs_data[POS_MB_ANAMIN_ANA1+1]=1973;
	Mbs_data[POS_MB_ANAMIN_ANA1+2]=24;

	if (Mb_verbose){
		printf("\n\tPOS_MB_BDCONF_ANACONF_TAG1: %d SIZE_MB_BDCONF_ANACONF: %d",POS_MB_BDCONF_ANACONF_TAG1,SIZE_MB_BDCONF_ANACONF);
		printf("\n\tPOS_MB_BDCONF_DIGCONF_TAG1: %d SIZE_MB_BDCONF_DIGCONF: %d",POS_MB_BDCONF_DIGCONF_TAG1,SIZE_MB_BDCONF_DIGCONF);
		printf("\n\tPOS_MB_ANAMIN_FECHA: %d ",POS_MB_ANAMIN_FECHA);
		printf("\n\tSizeof(QM) %d",sizeof(QM));

	}

	printf("\nFin Copia a bbdd ModBus");

}
