/*
*	Prometeo
*	FILE:	        ipcserver.c	
*	AUTHOR:		M.Bibudis
*	DATE:		01-03-2005
*	REVISION:	1.0
*	PRODUCT:	IPC
*	SUBJECTS:	ipc services
*	O.S.:		LINUX - AXIS ine Axion	
*	CUSTOMER:	SAIH
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include "ipcserver.h"
#include "logersaihbd.h"
#include "axisloger.h"

// Comunicacion Internet/TCP; Forma de llamada: modo Server		
char *ConfTty[NCConf];
int NumArg=18;
RX rx;

int proto_saih();
void SigComu();
void KillServer();
int socket_ini;
struct sockaddr_in seraddr;
struct sockaddr_in clntaddr;
struct servent *ser;		// Get Servicion Name
struct hostent *serv;		// Pointer to host, info for Remote Host 
unsigned long ser_idir;
char *hostname;

unsigned char msg_recv[NB2];
unsigned char msg_send[MAX_BS];
unsigned char BufferQm[BYTEQM];
char msg_aux[MAX_BS];
char *buf_aux;
int nbyte_send;
long flmsg;
int lmsg;

QM qm;
IN in;
BDCONF BdConf;
GN gn;

unsigned char chaux[32];
unsigned char chmsgin[2];

static char *version = "1.1";

main(argc,argv)
int argc;
char *argv[];
{

	int i,j,k,n,l,x;
	int resto_bs,num_v_s;
	int s,ss;		// Descriptores de sockets
	struct linger linger;
	int clnt_addr_len;
	unsigned long mask[10];
	long blocktime;		// Time Out en Recepcion
	short pid,ipid;
        char name[20];
        char aux[64];
        unsigned long segjulact;
        struct tm *newtime;
        char *auxch;

    	if ( (argc==2) && (argv[1][1]=='v')){
    		printf("\n************************************");
    		printf("\n\t IData Sistemas de Control S.L.");
    		printf("\n\t ipcserver Version: %s",version);
    		printf("\n************************************\n");
    		return(0);
    	}

        strcpy(name,"ipcserver.pid");				// PID del Proceso
        ipid=getpid();
        PidLog(name,ipid);

        segjulact=time(NULL);                                   // Hora Actual
        newtime=localtime(&segjulact);
        auxch=asctime(newtime);
        sprintf(aux,"Proceso IpcServer Iniciado:%s",auxch);
        AxisLog(aux);                                           // Log

	if(i=ReadLogerBd(&BdConf)!=0){				// Leer Objeto B.D  BDCONF
		printf("\n\tReadLogerBd:Error=%d",i);
        	segjulact=time(NULL);                                   // Hora Actual
        	newtime=localtime(&segjulact);
        	auxch=asctime(newtime);
        	sprintf(aux,"Proceso IpcServer:Error:ReadLogerBd :%s",auxch);
        	AxisLog(aux);                                           // Log
		exit(1);}


	if (argc > 3){
		printf("\n\tAXIS SERVER: %s: Error en Argumentos.\n",argv[0]);
		exit(1);}


	signal(SIGPIPE,SIG_IGN);
	signal(SIGCLD,SIG_IGN);					//Verificar
	signal(SIGINT,KillServer);
	signal(SIGKILL,KillServer);
	signal(SIGTERM,KillServer);

	if( (errno==ENOTCONN) || (errno==ECONNRESET) || (errno==ECONNREFUSED) ||
		(errno==EHOSTDOWN) || (errno==EHOSTUNREACH) || (errno==ENETDOWN) ||
		(errno==ENETRESET) || (errno==ENETUNREACH) )
		printf("\n\tAXIS SERVER:Parakalo Error de LAN:%d Comprobar la Red",errno);

	memset((char *)&clntaddr,0,sizeof(struct sockaddr_in));
	memset((char *)&seraddr,0,sizeof(struct sockaddr_in));
	clntaddr.sin_family=AF_INET;
	seraddr.sin_family=AF_INET;

	if (!(ser=getservbyname(argv[1],"tcp"))){				// Get servicio puerto asociado
		printf("\n\tAXIS SERVER: %s: Error en B.D servicios", argv[0],errno);
		exit(1);}
	seraddr.sin_port=ser->s_port;

	if (argc == 3){								// Nos especifican el nombre del host
		if (strchr(argv[2],'.')){					// Es una direccion internet
			if ((ser_idir=inet_addr(argv[2])) == -1){		// Transformar  direccion  ethernet
				printf("\n\tAXIS SERVER: %s: Error en direccion internet.Servidor",
				argv[2],errno);exit(1);}
			if (!(serv=gethostbyaddr(&ser_idir,4,AF_INET))){	// Get ser by addr
				printf("\n\tAXIS SERVER:%s: Error en base de Datos de hosts",
				argv[2],errno);exit(1);}
		}
		else{								// Es el nombre de un host
			if (!(serv=gethostbyname(argv[2])))			// Get host by name
				printf("\n\tAXIS SERVER:%s: Error en b.d de hosts", argv[2],errno);
			exit(1);}

		//seraddr.sin_addr.s_addr=((struct in_addr *)(serv->h_addr))->s_addr;
		seraddr.sin_addr.s_addr=*(long *)(serv->h_addr_list[0]);
	}
	else{									// Aceptamos conexion de cualquier host
		seraddr.sin_addr.s_addr=INADDR_ANY;
	}

	/*printf("\n serv_s_addr=%ld %ld",seraddr.sin_addr.s_addr,ser_idir);
	printf("\nserv_s_addr=%x %x",seraddr.sin_addr.s_addr,ser_idir);
	printf("\nser_name=%s",serv->h_name);
	hostname=inet_ntoa(seraddr.sin_addr.s_addr);
	printf("\nhost_in_name=%s\n",hostname);*/

	seraddr.sin_addr.s_addr=INADDR_ANY;					// Aceptamos Conexion de cualquier host

	for (;;){
		if ((s=socket(AF_INET,SOCK_STREAM,0)) == -1){			// Open socket
			printf("\n\tAXIS SERVER:%s: Error %d en creacion socket",argv[0],errno);
			exit(1);}

		socket_ini=s;
		i=0;
		/*if (ioctl(s,FIOSNBIO,&i) == -1){					// Quitar modo non-blocking
			printf("\n\tAXIS SERVER:%s: Error %d en ioctl FIOSNBIO",argv[0],errno);
			close(s);
			exit(1);}***/


		i=1;									// Poner cierre en modo soft
		linger.l_onoff=1;
		linger.l_linger=1;
		if (setsockopt(s,SOL_SOCKET,SO_LINGER,&linger,sizeof(linger)) == -1){
			printf("\n\tAXIS SERVER:%s: Error %d en setsockopt",argv[0],errno);
			close(s);
			exit(1);}

		for(;;){
			if (bind(s,&seraddr,sizeof(struct sockaddr)) == -1){			// bind socket
				printf("\n\tAXIS SERVER:%s: Error %d en bind socket.",argv[2],errno);
				if(errno==EADDRINUSE){
					//sleep(60);
					TimeWait(500);
					continue;}
				exit(1);}
			else break;
		}

		if (listen(s,8) == -1){
			printf("\n\tAXIS SERVER:%s: Error %d en listen socket.",argv[0],errno);
			close(s);
			exit(1);}
		printf("\n\tAXIS SERVER:%s: Esperando Conexion en Modo Server.\n",argv[0]);
		clnt_addr_len=sizeof(struct sockaddr_in);

		for(;;){
			if ((ss=accept(s,&clntaddr,&clnt_addr_len)) == -1){			// Aceptamos Conexion
				printf("\n\tAXIS SERVER:%s: Error %d en accept socket",argv[0],errno);
				close(s);
			exit(1);}

			if((pid=fork())==-1)
				printf("\n\tAXIS SERVER:Error en Pid=%d",errno);
			else if(pid==0){
				signal(SIGPIPE,SigComu);
				signal(SIGCLD,SIG_IGN);
				signal(SIGINT,KillServer);
				signal(SIGTERM,KillServer);

				printf("\n\tAXIS SERVER:%s(%d):Establecida Conexion Modo Server socket=%d",argv[0],getpid(),ss);
				close(s);					// A partir de ahora el socket valido es ss

				memset(msg_recv,0,NB2);
				blocktime=segtojul(NULL);
				for (;;){					// bucle para recepcion-transmision
					if(segtojul(NULL) > blocktime+50)
						break;
					if ((j=net_select_n(ss,20,2)) < 0){
						printf("\n\tAXIS SERVER: socket=%d Error %d Select\n",ss,errno);
						break;}
					if (!(j & 0x2)){
						printf("\n\tAXIS SERVER:socket=%d Buffer Recepcion Vacio:Error=%d PID=%d",
						ss,errno,getpid(0));
						timw(10);
						continue;}
					if ((i=recv(ss,msg_recv,NB,0)) == -1){			// recepcion no-blocking
						printf("\n\tAXIS SERVER:%s:recv: Error= %d En Recepcion",argv[0],errno);
						break;}
					if (i){							// Se han recibido bytes
						printf("\n\tAXIS SERVER:Recibidos %d bytes.",i);
						if((x=ProtoSaih(msg_recv))<0){			// Comprobar protocolo SAIH
							printf("\n\tAXIS SERVER:Error: Proto_Saih=%d",x);
							break;}
						num_v_s=lmsg/NB1;					// Enviamos Num Veces de NB2 Byte
						resto_bs=lmsg % NB1;
						//i=NB1;
						i=lmsg;
						for(n=0;n<=num_v_s;n++){				// enviar blocking
							if(segtojul(NULL) > blocktime+50) break;
							if ((j=net_select_n(ss,20,1)) < 0){
								printf("\n\tAXIS SERVER: Error:%d en select",errno);
								break;}
							if (!(j & 0x1)){
								printf("\n\tAXIS SERVER:Error_No_Es: Buffer Trasmision Lleno",errno);
								timw(10); n--;
								continue;}
							if ((i=send(ss,msg_send+n*NB1,i,0)) == -1){
								printf("\n\tAXIS SERVER:%s Error %d send",argv[0],errno);
								break;}
							printf("\n\tAXIS SERVER:Enviados %d Bytes.\n",i);
						}
					}
					else if(i==0){
						printf("\n\tAXIS SERVER:Byte_Recibidos=%d Error=%d",i,errno);
						break;					// Conexion cerrada
					}
				}//for
				close(ss);						// Cerrar nuevo descriptor
				printf("\n\tAXIS SERVER:Proceso Hijo=%d Finaliza Servicio\n",getpid());
				exit(0);
			}
			else if(pid){
				printf("\n\tAXIS SERVER:Proceso Padre Acepta Otro Cliente Socket=%d\n",s);
				close(ss);}
		}// Accept
		close(s);
		printf("\n\tAXIS SERVER:%s: Finalizada Conexion Modo Server.close(s)",argv[0]);
		printf("\n\tAXIS SERVER:%s: Inicio Nueva  PID=%d Conexion Modo Server.\n",argv[0]);
	}
}

void KillServer()
{
	short i;
        char path[80];
        char aux[64];
        long segjulact;
        struct tm *newtime;
        char *auxch;

	signal(SIGINT,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	signal(SIGKILL,SIG_IGN);
	//printf("\n\tAXIS SERVER:Proceso Servidor Terminado:socket=%d\n",socket_ini);
	close(socket_ini);
	printf("\n\tAXIS SERVER:Procesos Terminados:%d :%d\n",getpid(),getpgrp());

	segjulact=time(NULL);                                   // Hora Actual
        newtime=localtime(&segjulact);
        auxch=asctime(newtime);

        if(i=killpg(getpgrp(),SIGTERM) !=0)
                killpg(getpgrp(),SIGKILL);
        sprintf(aux,"Proceso: IpcServer Terminado :%s",auxch);
        AxisLog(aux);
	
	printf("\n");
        exit(0);

}

void SigComu()
{
	signal(SIGPIPE,SIG_IGN);
	printf("\n\tAXIS SERVER:Parakalo Comprobar la Red ; Proceso Servidor Parado:%d",
	getpid());
}

// Protocolo SAIH
ProtoSaih(msg)
char *msg;
{
        short i,j,l;
        short nbyte_recv,nbyte_val;
        unsigned static int chk_cal=0,chk_recv=1,chk_env=0;
        short ciclico,estacion;
        long segjulqm;
	char aux_fecha[64];
	char msgfin[4];
	unsigned char aux[2],msg_aux[NB1];;
	char auxlog[64];

	short IndActFe,iIndActFe,IndActAx,ValMaxInd;
	short NumInAlm,NumInEnvFe,NumWord,p,x,k;

	struct tm *newtime,*newtimeax,TmAx;
	unsigned long segjulfe,isegjulfe,segjulax,dif;
        char *auxchfe,*auxchax;
	int EstComLoger;


	memset(msg_send,0,MAX_BS);
	chk_cal=0; chk_recv=1;
	switch(msg[IND_PROTO]){
		case PROTO_104:
		break;

		case SAC_MSG_TM:
			printf("\n\tAXIS SERVER:SAC_MSG_TIME:-------------------------------------------------------------------");
			nbyte_recv=SAC_LMSGCL_TM;

			printf("\n\tREAD:");
			for(i=0;i<nbyte_recv;i++)
				printf("%02x ",msg[i]);

			memcpy(chaux+0,(char *)msg+SAC_IND_INI,1);			// 0x80  Control CHK, PROTO SAC 
			memcpy(chaux+3,(char *)msg+SAC_IND_INI+3,1);			// 0x03
			memcpy(chaux+26,(char *)msg+SAC_IND_FIN_TM,1);			// 0xa5
			memcpy(chaux+27,(char *)msg+SAC_IND_FIN_TM+1,1);		// 0xa5

			if( (chaux[0] != SAC_P0) || (chaux[3] != SAC_P3) || (chaux[26] != SAC_FIN) || (chaux[27] != SAC_FIN) ){
				printf("\n\tAXIS-SAC:SAC_MSG_TM: Error De Protocolo SAC_MSG_TM");
				return(-1);}

			memcpy(&estacion,msg+SAC_IND_EST,2);
			if(BdConf.remconf.ihw != estacion){
				segjulax=time(NULL);
				newtime=localtime(&segjulax);
				auxchax=asctime(newtime);
				sprintf(auxlog,"IpcServer:Error Numero Fisico Remota:(:%d != :%d) Fecha=%s",
				estacion,BdConf.remconf.ihw,auxchax);
				AxisLog(auxlog);
				return(-1);
			}

			memcpy(&chk_recv,msg+SAC_IND_CHK_TM,2);
			memcpy(&ciclico,msg+SAC_IND_CIC,2);
			memcpy(&segjulfe,msg+SAC_IND_INFO,4);
			isegjulfe=segjulfe;
			segjulfe=ntohl(segjulfe);				// SegJulQm Orden Byte Rem SAC
			segjulfe = segjulfe + SEGJULCTESAC;			// Time desde Epoca=1970
			newtime=localtime(&segjulfe);				// Comprobar SEGJULCTE entre FrontEnd y Axis.
			auxchfe=asctime(newtime);

			segjulax=time(NULL);
			newtime=localtime(&segjulax);
			auxchax=asctime(newtime);
			dif=segjulax - segjulfe;
				
			printf("\n\tAXIS SERVER:nbyte_recv=%hd Estacion=%d isegjulfe=%X segjulfe=%ld (%X) FechaFe=%s",
			nbyte_recv,estacion,isegjulfe,segjulfe,segjulfe,auxchfe);
			printf("\n\tAXIS SERVER:DifAxisFe=%d Seg segjulax=%ld FechaAxis=%s",dif,segjulax,auxchax);

			k=20;


                        if( (dif<=10) || (dif>k+10) ){
                                segjulax = segjulfe + k;
                                newtime=localtime(&segjulax);
// primero establecemos hora del systema
                                sprintf(aux_fecha,"/bin/date %02d%02d%02d%02d%04d.%02d",newtime->tm_mon+1,newtime->tm_mday,
                                        newtime->tm_hour,newtime->tm_min,newtime->tm_year+1900,newtime->tm_sec);
                                if ( (i=system(aux_fecha)) <0 ){
                                        sprintf(auxlog,"ERROR SET_TIME %d ",i);
                                        AxisLog(auxlog);
                                        AxisLog(aux_fecha);
                                        return(-1);
                                }
// despues se establece la hora RTC con la del sistema
                                strcpy(aux_fecha,"/sbin/hwclock -w");
                                if ( (i=system(aux_fecha)) <0 ){
                                        sprintf(auxlog,"ERROR SET_TIME %d ",i);
                                        AxisLog(auxlog);
                                        AxisLog(aux_fecha);
                                        return(-1);
                                }
                                sprintf(auxlog,"DIFERENCIA HORA FrontEnd-Axis=%d Seg:Fecha=%s",dif,auxchax);
                                AxisLog(auxlog);
                        }


/*
			if(dif <= 10){
				segjulax = segjulfe + k;
				newtimeax=localtime(&segjulax);				// Comprobar SEGJULCTE entre FrontEnd y Axis.
				if(set_time(newtimeax)!=0){
					sprintf(auxlog,"ERROR SET_TIME DIFERENCIA HORA FrontEnd-Axis=%d Seg:Fecha=%s",dif,auxchax);
					AxisLog(auxlog);
				}	
				sprintf(auxlog,"DIFERENCIA HORA FrontEnd-Axis=%d Seg:Fecha=%s",dif,auxchax);
				AxisLog(auxlog);
			} 
			if(dif > k+10){
				segjulax = segjulfe + k;
				newtimeax=localtime(&segjulax);				// Comprobar SEGJULCTE entre FrontEnd y Axis.
				set_time(newtimeax);	
				sprintf(auxlog,"DIFERENCIA HORA FrontEnd-Axis=%d Seg:Fecha=%s",dif,auxchax);
				AxisLog(auxlog);
			} 
*/
		break;
		case SAC_MSG_QM:
			printf("\n\tAXIS SERVER:SAC_MSG_QM:---------------------------------------------------------------------");
			nbyte_recv=SAC_LMSGCL_QM;

			printf("\n\tREAD:");
			for(i=0;i<nbyte_recv;i++)
				printf("%02x ",msg[i]);

			memcpy(chaux+0,(char *)msg+SAC_IND_INI,1);			// 0x80  Control CHK, PROTO SAC 
			memcpy(chaux+3,(char *)msg+SAC_IND_INI+3,1);			// 0x03
			memcpy(chaux+18,(char *)msg+SAC_IND_FINCL_QM,1);		// 0xa5
			memcpy(chaux+19,(char *)msg+SAC_IND_FINCL_QM+1,1);		// 0xa5

			if( (chaux[0] != SAC_P0) || (chaux[3] != SAC_P3) || (chaux[18] != SAC_FIN) || (chaux[19] != SAC_FIN) ){
				printf("\n\tAXIS-SAC:SAC_MSG_QM: Error De Protocolo SAC_MSG_QM");
				return(-1);}

			memcpy(&estacion,msg+SAC_IND_EST,2);
			if(BdConf.remconf.ihw != estacion){
				segjulax=time(NULL);
				newtime=localtime(&segjulax);
				auxchax=asctime(newtime);
				sprintf(auxlog,"IpcServer:Error Numero Fisico Remota:(:%d != :%d) Fecha=%s",
				estacion,BdConf.remconf.ihw,auxchax);
				AxisLog(auxlog);
				return(-1);
			}

			move(&chk_recv,msg+SAC_IND_CHKCL_QM,2);
			move(&ciclico,msg+SAC_IND_CIC_QM,2);
			move(&estacion,msg+SAC_IND_EST_QM,2);
			move(&segjulqm,msg+SAC_IND_JUL_QM,4);

			segjulqm=ntohl(segjulqm);				// SegJulQm Orden Byte Rem SAC
			segjulqm = segjulqm + SEGJULCTESAC;			// Time desde Epoca=1970
			//segjulqm = segjulqm + 0;				// Time desde Epoca=1980
                        newtime=localtime(&segjulqm);
                        auxchax=asctime(newtime);

			printf("\n\tAXIS SERVER:nbyte_recv=%hd Estacion=%d segjulqm=%ld (%X) (%s)",nbyte_recv,estacion,segjulqm,segjulqm,auxchax);
			
			if( i=ReadLogerQm(segjulqm,&qm) !=0){				// Control
				printf("\n\tAXIS SERVER:SAC_MSG_QM: Sin Datos:Error=%d",i);
				//memcpy(msg_send,qm.BufferQm,qm.lBufferQm);		// QM Status=0 , QM No Adquirido
				//lmsg=qm.lBufferQm;					// Num Byte Enviar
				return(3);}

			lmsg = CrearBufferQm_v2(estacion);

			memcpy(BufferQm+SAC_IND_CIC_QM,msg+SAC_IND_CIC_QM,2);
									// Num Byte Enviar
			memcpy(msg_send,BufferQm,lmsg);

			l=(lmsg - 4) - SAC_IND_MSG;					// ChkSumIpc
			chk_env=ChkSumaIpc(msg_send+SAC_IND_MSG,l);
			chk_env=htons(chk_env);                                         // Orden Byte Host to Net
			memcpy(msg_send+(lmsg-4),(char *)&chk_env,2);

			// añadido flags SAICA
			memcpy(msg_send+lmsg,qm.Flag,qm.NumAna);
			lmsg=lmsg+qm.NumAna;
			// fin SAICA

			printf("\n\tWRITE:");
			for(i=0;i<lmsg;i++)
				printf("%02x ",msg_send[i]);
		break;

		case SAC_MSG_IN:
			printf("\n\tAXIS SERVER:SAC_MSG_INCID: ----------------------------------------------------------------");
			if(i=ReadLogerGn(gn)!=0)					// Si Remota Loger No Comunica return
                        	printf("\n\tReadLogerGn:Error=%d",i);
			if(gn.NumComLoger[0]!=0)
				EstComLoger=(gn.NumComLoger[1] * 100) / gn.NumComLoger[0];
			else EstComLoger=50;
			//printf("\n\tEstComLoger=%d : %d %d",EstComLoger,gn.NumComLoger[0],gn.NumComLoger[1]);
			if(EstComLoger < 20 ){
				printf("\n\tAxis:EstComLoger=%d",EstComLoger);
				lmsg=0;
				return(3);
			}

			if(i=ReadLogerIn(&in) !=0){
				printf("\n\tReadLogerIn:No Read");
				lmsg=0;
				return(-1);}
			IndActAx=in.IndAct;
			NumInAlm=in.NumInAlm;

			printf("\n\tAXIS:IndAct=%hd (%0x) IndUltIn=%hd (%0x) NumInAlm=%hd (%0x)",
			in.IndAct,in.IndAct,in.IndUltIn,in.IndUltIn,in.NumInAlm,in.NumInAlm);

                        nbyte_recv=SAC_LMSGCL_IN;
                        printf("\n\tREAD:");
                        for(i=0;i<nbyte_recv;i++)
                                printf("%02x ",msg[i]);

			memcpy(chaux+0,(char *)msg+SAC_IND_INI,1);			// 0x80  Control CHK, PROTO SAC 
			memcpy(chaux+3,(char *)msg+SAC_IND_INI+3,1);			// 0x03
			memcpy(chaux+16,(char *)msg+SAC_IND_FIN_IN,1);			// 0xa5
			memcpy(chaux+17,(char *)msg+SAC_IND_FIN_IN+1,1);		// 0xa5

			if( (chaux[0] != SAC_P0) || (chaux[3] != SAC_P3) || (chaux[16] != SAC_FIN) || (chaux[17] != SAC_FIN) ){
				printf("\n\tAXIS-SAC:SAC_MSG_IN: Error De Protocolo SAC_MSG_IN");
				return(-1);}

			memcpy(&estacion,msg+SAC_IND_EST,2);
			if(BdConf.remconf.ihw != estacion){
				segjulax=time(NULL);
				newtime=localtime(&segjulax);
				auxchax=asctime(newtime);
				sprintf(auxlog,"IpcServer:Error Numero Fisico Remota:(:%d != :%d) Fecha=%s",
				estacion,BdConf.remconf.ihw,auxchax);
				AxisLog(auxlog);
				return(-1);
			}
                        memcpy(&chk_recv,msg+SAC_IND_CHK_IN,2);
                        memcpy(&ciclico,msg+SAC_IND_CIC,2);
                        memcpy(&estacion,msg+SAC_IND_EST,2);
                        memcpy(&iIndActFe,msg+SAC_IND_INFO,2);
                        IndActFe=ntohs(iIndActFe);					// 
                        printf("\n\tAXIS SERVER:Nbyte_Recv=%hd Estacion=%d IndActFe=%hd iIndActFe:%X ",
			nbyte_recv,estacion,IndActFe,iIndActFe);

			memcpy(msg_send+0,msg,SAC_IND_INFO);				// Copiamos Cabecera hasta SAC_IND_INFO  
			msg_send[SAC_IND_MSG] = (~SAC_MSG_IN) + 1;			// Codigo MSG IN
			if(iIndActFe == -1){
				msg_send[SAC_IND_WORD] = SAC_P55_IN;				// Word del Mensaje: 5*2 Byte
				NumWord= SAC_P55_IN;                				// Word del Mensaje 0x05

                        	IndActAx=ntohs(IndActAx);					// Indice Actual Axis = IndUltIn Axis
				memcpy(msg_send+SAC_IND_INFO,(char *)&IndActAx,2);		// Indice a partir del cual se envian Incid al FE

				ValMaxInd=VALMAXIND;
                        	ValMaxInd=ntohs(ValMaxInd);					// ValMaxInd
                        	memcpy(msg_send+SAC_IND_INFO+2,(char *)&ValMaxInd,2);			

                        	NumInAlm=ntohs(NumInAlm);					// 
                        	memcpy(msg_send+SAC_IND_INFO+4,(char *)&NumInAlm,2);			// Num Incid Almacenadas e Axis

				lmsg= 8 + (NumWord*2) + 4;
				l=(lmsg - 4) - SAC_IND_MSG;					// ChkSum
				chk_env=ChkSuma(msg_send+SAC_IND_MSG,l);
				chk_env=htons(chk_env);                                         // Orden Byte Host to Net
				memcpy(msg_send+8+(NumWord*2),(char *)&chk_env,2);

				msg_send[8+(NumWord*2)+2]=SAC_FIN;                               // Fin MSG A5
				msg_send[8+(NumWord*2)+3]=SAC_FIN;
				lmsg= 8 + (NumWord*2) + 4;					// 8Byte hasta Ruta inclusive +...+4B Chk+a5

				printf("\n\tWRITE:");
				for(i=0;i<lmsg;i++)
					printf("%02x ",msg_send[i]);
				printf("\n\tAXIS:IndAct=%hd (%0x) IndUltIn=%hd (%0x) ValMaxInd=%hd (%0X) NumInAlm=%hd (%0x)",
				in.IndAct,in.IndAct,in.IndUltIn,in.IndUltIn,VALMAXIND,VALMAXIND,in.NumInAlm,in.NumInAlm);
				return(0);
			}
			// IF IndActFe == IndActAx ;


			NumInEnvFe = IndActAx - IndActFe +1;				// Calculamos Indices
			if(NumInEnvFe < 0 ) NumInEnvFe = NumInEnvFe + VALMAXIND + 1;
			if(NumInEnvFe > NumInAlm){
				NumInEnvFe=NumInAlm;
				IndActFe = IndActAx - NumInAlm +1;
				if(IndActFe < 0 ) IndActFe = IndActFe + VALMAXIND + 1;
			}

			if(NumInEnvFe > 120) NumInEnvFe=120;				// Max Num incid Env Fe
			for(i=0;i<NumInEnvFe;i++){
				p=IndActFe + i;						// Indice Incid en BufferIn
				if(p > VALMAXIND) p=p - VALMAXIND - 1;
				x=(SAC_IND_NIN_IN +2) + i*4;				// Indice Incid en MsgSend 
                        	memcpy(msg_send+x,in.BufferIn[p],4);
			}
			printf("\n\tIndActAx=%hd (%0X) IndActFe=%hd (%0X) NumInEnvFe=%hd (%0X)",
			IndActAx,IndActAx,IndActFe,IndActFe, NumInEnvFe,NumInEnvFe);
			NumWord= (8 + (NumInEnvFe * 4) ) / 2;                		// Word del Mensaje
                        memcpy(aux,(char *)&NumWord,1);
			msg_send[SAC_IND_WORD] = aux[0];				// Word del Mensaje

                        IndActFe=ntohs(IndActFe);					// 
			memcpy(msg_send+SAC_IND_INFO,(char *)&IndActFe,2);		// Indice a partir del cual se envian Incid al FE

                        NumInEnvFe=ntohs(NumInEnvFe);					// 
                        memcpy(msg_send+SAC_IND_NIN_IN,(char *)&NumInEnvFe,2);		// Num Incid Enviadas al FE

			lmsg= 8 + (NumWord*2) + 4;					// 8Byte hasta inclusive el Ciclico
			l=(lmsg - 4) - SAC_IND_MSG;					// ChkSum
			chk_env=ChkSuma(msg_send+SAC_IND_MSG,l);
			chk_env=htons(chk_env);                                         // Orden Byte Host to Net
			memcpy(msg_send+8+(NumWord*2),(char *)&chk_env,2);

			msg_send[8+(NumWord*2)+2]=SAC_FIN;                               // Fin MSG A5
			msg_send[8+(NumWord*2)+3]=SAC_FIN;
			lmsg= 8 + (NumWord*2) + 2 +2;

			printf("\n\tWRITE:");
			for(i=0;i<lmsg;i++)
				printf("%02x ",msg_send[i]);
		break;
		case SAC_MSG_DS:
			printf("\n\tAXIS SERVER:SAC_MSG_DimensionSeñales:---------------------------------------------------------");
			nbyte_recv=SAC_LMSGCL_DS;

			printf("\n\tREAD:");
			for(i=0;i<nbyte_recv;i++)
				printf("%02x ",msg[i]);

			memcpy(aux,(char *)msg+SAC_IND_INI,1);
			if(aux[0] != SAC_P0){				// Control CHK, PROTO SAC QM
				printf("\n\tAXIS SERVER:SAC_MSG_TIME: Error De Protocolo SAC_MSG_TIME_INI");
				return(-1);
			}
			memcpy(aux,msg+SAC_IND_FIN_TM,1);
			if(aux[0] !=SAC_FIN){				// Control CHK, PROTO SAC QM
				printf("\n\tAXIS SERVER:SAC_MSG_TIME: Error De Protocolo SAC_MSG_TIME_FIN");
				return(-1);
			}
			move(&estacion,msg+SAC_IND_EST,2);
				
			printf("\n\tAXIS SERVER:nbyte_recv=%hd Estacion=%d",nbyte_recv,estacion);
			memcpy(msg_aux,msg,nbyte_recv);

			ConfTty[10] = msg_aux;				// Buffer a Enviar
			printf("\n\tTty:WRITE:");
			for(i=0;i<SAC_LMSGCL_DS;i++){                   // Buffer Enviar 28 byte
				printf("%02x ",ConfTty[10][i]);}
			printf("\n");

			int ResUtl = TtyFunc(NumArg,ConfTty);                           // Write Tty
			//int ResUtl = DebugTty(NumArg,ConfTty);

			if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){                           // Read Tty
				printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
				for(i=0;i<rx.bytesleidos;i++)
					printf("%02x ",rx.bufrecv[i]);}
			else{
				printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
				return(-1);}

			memcpy(msg_send,(char *)&rx.bufrecv,rx.bytesleidos);
			lmsg=rx.bytesleidos;


		break;

		default:
			printf("\n\tAXIS SERVER:PROTO_SAIH:Ningun Protocolo Conocido");
			printf("\n\tREAD:");
				for(i=0;i<NB1;i++)
					printf("%02x ",msg[i]);
		break;
	}
	printf("\n\tAXIS SERVER:FIN_MSG_SAIH:-----------------------------------------------------------------------");
	return(0);
}

// *** chksuma  segun Loger ***
ChkSuma(men,lmen)
unsigned char *men;
int lmen;
{
        unsigned int isum;
	unsigned char chisum[3];
        int i,j;

	/*printf("\n");
	for(j=0;j<lmen;j++) 
		printf(" %0x ",men[j]);***/
        isum=0;
        for(i=0;i<lmen;i+=2)
                isum+=((men[i] << 8) | men[i+1]);

	//memcpy(chisum,(char *)&isum,2);
	//printf("\n\t isum=%d :%0x %0x :",isum,chisum[0],chisum[1]);
        return(isum & 0xffff);
}

// *** chksuma  segun Sac ***
ChkSumaIpc(men,lmen)
unsigned char *men;
int lmen;
{
        unsigned int isum;
	unsigned char chisum[3];
        int i,j;

	/*printf("\n");
	for(j=0;j<lmen;j++)
		printf(" %0x ",men[j]);***/
        isum=0;
        for(i=0;i<lmen;i+=2)
                isum+=((men[i] << 8) | men[i+1]);

	//memcpy(chisum,(char *)&isum,2);
	//printf("\n\t isum=%d :%0x %0x :",isum,chisum[0],chisum[1]);
        return(isum & 0xffff);
}

 
/****************************************************************************
chk_sum_OLD(chkaux,l,chk)
int *chkaux;
int l;
int *chk;
{
	int i,j;

    for(j=0;j<l/sizeof(int);j++){
	*chk+=chkaux[j] & 0xff;
	fprintf(stderr,"\nchk=%d chkaux[%d]=%0x",
	*chk,j,chkaux[j]);}
}

write_socket_OLD(s,msg,lmsg)
int s;
char *msg;
int lmsg;
{
	int i,j,k;

    if ((i=net_select_n(s,30,1)) < 0){ 	test envio con 5seg. time-out
	fprintf(stderr,"\n Error:%d en select",errno);
	return(-1);}
    if (!(i & 0x1)){
	fprintf(stderr,"\nError_Buffer transmis. lleno:%d",errno);
	return(-1);}
    if ((i=send(s,msg,lmsg,0)) == -1){
	fprintf(stderr,"\nError:%d en send.",errno);
	return(-1);}

    return(i);
}

read_socket_OLD(s,msg,lmsg)
int s;
char msg[1024];
long lmsg;
{
	int i,j,k;
	static int len=0;
	long segtime;

    if ((i=net_select_n(s,30,2)) < 0){ test recepcion 2seg. time-out
	fprintf(stderr,"\nsocket=%d: Error %d en select.\n",s,errno);
	return(-1);}
    if (!(i & 0x2)){
	fprintf(stderr,"\nsocket=%d: Buffer recepcion vacio:Er=%d",s,errno);
	return(-1);}

    segtime=segtojul(NULL);
    k=0;len=0;
    while(len < lmsg){
	if(segtojul(NULL) > segtime+20) return(-1);
	if((i=recv(s,&msg[len],lmsg-len,0)) == -1){
	    if(errno==EWOULDBLOCK){
		timw(2);
		continue;}
	    fprintf(stderr,"\nsocket=%d: Error %d en recv.\n",s,errno);
	    return(-1);}
	if(i==0){
	    fprintf(stderr,"\nError_No buffer de recepcion vacio");
	    return(0);}
	len+=i;
    }
    printf("\n\t Recibidos:%d bytes :\n",i);
    printf("\n");
    for(j=0;j<i;j++) printf("%02x",msg[j]);
    printf("\n");

    return(i);

}

net_select_n_OLD(s,t,op)
int s;
long t;
int op;
{
    int stat;
    struct timeval tim;
    unsigned long read_mask[10],write_mask[10];
    memset((char *)read_mask,0,sizeof(long)*10);
    memset((char *)write_mask,0,sizeof(long)*10);
    if (op & 1) write_mask[s/32] |= (1 << (s%32));
    if (op & 2) read_mask[s/32] |= (1 << (s%32));
    tim.tv_sec=t;
    tim.tv_usec=0;
    if (select(s+1,read_mask,write_mask,(int *)0,&tim) == -1) return(-errno);
    stat=0;
    if (write_mask[s/32] & (1 << (s%32))) stat |= 1;
    if (read_mask[s/32] & (1 << (s%32))) stat |= 2;
    return(stat);
}
*******************************************************************************/
timw(ticks)
int ticks;
{
	int i,j,k;

	//sleep(2);
	TimeWait(10);
}
//Condif Tty RS232
ConfigTty()
{

	short i,j;

	ConfTty[0] = "SerialCom";       // Configurar Parametros tty
	ConfTty[1] = "-d";
        ConfTty[2] = COM1;
//      ConfTty[2] = PTS;               // Para Debug en terminal
        ConfTty[3] = "-b";
        ConfTty[4] = Baudios;
        ConfTty[5] = "-rts";
        ConfTty[6] = "0";
        ConfTty[7] = "-v";
        ConfTty[8] = Verbose;
        ConfTty[9] = "-tx";
        ConfTty[10] = "80010203190485960001000020495f321e16a5a5";       /*BufSend*/
        ConfTty[11] = "-rts";
        ConfTty[12] = "1";
        ConfTty[13] = "-rxsaih";
        ConfTty[14] = TimeOut;

}

short CrearBufferQm_v2( short ihw){

short NumBytes=0,i;
unsigned int chk_env=0;
//char *msg[NB1];
unsigned long SegJulAux;
short aux,lBufferQm=-1;

		printf("\n\tCrearBufferQm:---------------------------");

		BufferQm[SACBUFFER_IND_INI_QM]=SACBUFFER_P0;			// 0x80  Control CHK, PROTO SAC
		BufferQm[SACBUFFER_IND_INI_QM+1]=SACBUFFER_P1;			// 0x01
		BufferQm[SACBUFFER_IND_INI_QM+2]=SACBUFFER_P2;			// 0x02
		BufferQm[SACBUFFER_IND_INI_QM+3]=SACBUFFER_P3;			// 0x03
		BufferQm[SACBUFFER_IND_MSG]=SACBUFFER_P4;			// 0xE7 Resp QM

		BufferQm[SACBUFFER_IND_EST]=ihw;				// Num Hardware Remota
		SegJulAux=htonl(qm.SegJul-SEGJULCTESAC);
		memcpy(BufferQm+SACBUFFER_IND_INFO,(char *)&SegJulAux,4);
		BufferQm[SACBUFFER_IND_INFO+4]=0x02;
		memcpy(BufferQm+SACBUFFER_IND_INFO+6,(char *)&SegJulAux,4);

		// STATUS QM
		aux=htons(qm.Status);
		memcpy(BufferQm+SAC_IND_STATUS_QM,(char *)&aux,2);		// Num Contadores

		BufferQm[SACBUFFER_IND_INFO+10]=SACBUFFER_CINC1;
		BufferQm[SACBUFFER_IND_INFO+11]=SACBUFFER_CINC2;
		BufferQm[SACBUFFER_IND_INFO+12]=SACBUFFER_CINC1;
		BufferQm[SACBUFFER_IND_INFO+13]=SACBUFFER_CINC2;
		BufferQm[SACBUFFER_IND_INFO+14]=SACBUFFER_CINC1;
		BufferQm[SACBUFFER_IND_INFO+15]=SACBUFFER_CINC2;

		aux=htons(NUMSENCONT);
		memcpy(BufferQm+SACBUFFER_IND_CONT,(char *)&aux,2);		// Num Contadores


		for (i=0;i<3*NUMSENCONT;i++){
		aux=htons(qm.ValorCont[i]);
		memcpy(BufferQm+SACBUFFER_IND_CONT+2+2*i,(char *)&aux,2);
		}

		aux=htons(qm.NumAna);
		memcpy(BufferQm+SACBUFFER_IND_CONT+2+6*NUMSENCONT,(char *)&aux,2);	// Num Analogicas
		for (i=0;i<qm.NumAna;i++){
		aux=htons(qm.ValorAna[i]);
		memcpy(BufferQm+SACBUFFER_IND_CONT+4+6*NUMSENCONT+2*i,(char *)&aux,2);
		}

		BufferQm[SACBUFFER_IND_CONT+25+6*NUMSENCONT+2*qm.NumAna]=0x01;
		BufferQm[SACBUFFER_IND_CONT+27+6*NUMSENCONT+2*qm.NumAna]=0x08;
		BufferQm[SACBUFFER_IND_CONT+29+6*NUMSENCONT+2*qm.NumAna]=0x01;
		BufferQm[SACBUFFER_IND_CONT+31+6*NUMSENCONT+2*qm.NumAna]=0x01;
		BufferQm[SACBUFFER_IND_CONT+33+6*NUMSENCONT+2*qm.NumAna]=0x1D;
		BufferQm[SACBUFFER_IND_CONT+34+6*NUMSENCONT+2*qm.NumAna]=0x0A;
		BufferQm[SACBUFFER_IND_CONT+35+6*NUMSENCONT+2*qm.NumAna]=0x60;
		// Numero de bytes hasta el checksum
		NumBytes = SACBUFFER_IND_CONT + 2 + 6*NUMSENCONT + 2 + 2*qm.NumAna + 2 + 2 + 28;
		BufferQm[SACBUFFER_IND_WORD]=(char *)((NumBytes - 4 - SACBUFFER_IND_MSG)/2);		// Num Word
		chk_env=ChkSumaIpc(BufferQm+SACBUFFER_IND_MSG,NumBytes - 4 - SACBUFFER_IND_MSG);
		chk_env=htons(chk_env);                                         // Orden Byte Host to Net
		memcpy(BufferQm+NumBytes,(char *)&chk_env,2);
		BufferQm[NumBytes+2]=SACBUFFER_FIN;                               // Fin MSG A5
		BufferQm[NumBytes+3]=SACBUFFER_FIN;
		lBufferQm=NumBytes+4;
#ifdef DEBUG
		printf("\n\tCrearBufferQm : %hd bytes\n",lBufferQm);
			for(i=0;i<lBufferQm;i++)
			printf("%02x ",BufferQm[i]);
		printf("\n\tFin CrearBufferQm:---------------------------");
#endif


		return lBufferQm;
}
