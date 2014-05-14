/*
*       Prometeo
*	FILE:		IpcFun.c
*	AUTHOR:		M.Bibudis@Enero-00
*	DATE:		01-01-00
*	REVISION:	1.5
*	PRODUCT:	SACIPC
*	SUBJECTS:	Sacipc services
*	O.S.:		UNIX - LINUX-AXIS ine Axion	
*	CUSTOMER:	S.A.C.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include "ipcserver.h"
#include "logersaihbd.h"
//#include "srvipc_shm.h"

extern int h_errno;

int ind_chk;
int ind_flmsg;
int ind_fin;
long flmsg=13;
unsigned char msg_send[NB2];
unsigned char msg_recv[NB2];
char msg_aux[MAX_BS];
char msg_val[MAX_BS];
int ByteLeidos;
static unsigned short iciclico=1;
static unsigned short ciclico=1;
static unsigned short Aciclico=1;
static unsigned short Sciclico=1;
unsigned char chaux[32];
unsigned char chmsgin[2];

connect_socket(servicio,host_name)
char servicio[12];
char host_name[16];
{

	int i,j,k;
	int s;
	static struct sockaddr_in peeraddr;	// Socket  peer address
	static struct servent *ser;		// Get servicio asociado
	static struct hostent *hos;		// Get internet adderess
	struct linger linger;
	static unsigned long idir;
	static int cont_fl=0;
        struct tm *newtime;
	long segjulact;
	char *auxch;


        segjulact=time(NULL);                                   // Hora Actual
        newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	printf("\n\tAXIS:CONNECT SOCKET:Fecha:%ld %s",segjulact,auxch);
	if( (errno==ENOTCONN) || (errno==ECONNRESET) || (errno==ECONNREFUSED) ||
	(errno==EHOSTDOWN) || (errno==EHOSTUNREACH) || (errno==ENETDOWN) ||
	(errno==ENETRESET) || (errno==ENETUNREACH))
		fprintf(stderr,"\n\tCONNECT SOCKET:Error de LAN:%d comprobar la red Parakalo;Fecha:",errno,segtojul(NULL));

	if(cont_fl==0){
		memset((char *)&peeraddr,0,sizeof(struct sockaddr_in));
		peeraddr.sin_family=AF_INET;
		if(!(ser=getservbyname(servicio,"tcp"))){			// Get servicion asoc
			fprintf(stderr,"\nError en B.D servicios=%d ; %d",errno,h_errno);
			exit(1);}
		peeraddr.sin_port=ser->s_port;

		if(strchr(host_name,'.')){					// Es una Direccion Internet
			if((idir=inet_addr(host_name)) == -1){			// Transformar Direccion Ethernet
				fprintf(stderr,"\nError dir. internet:%d ; %d",errno,h_errno);
				exit(1);}
			if (!(hos=gethostbyaddr(&idir,4,AF_INET))){		// Get host by address
				fprintf(stderr,"\nError en B.D de hosts:%d ; %d",errno,h_errno);
				exit(1);}
		}
		else if (!(hos=gethostbyname(host_name))){			// Get host by name
			fprintf(stderr,"\nError en B.D de hosts:%d;%d",errno,h_errno);
			exit(1);}
		peeraddr.sin_addr.s_addr=((struct in_addr *)(hos->h_addr))->s_addr;

		/*peeraddr.sin_addr.s_addr=*(long *)(hos->h_addr);
		printf("\ns_addr=%ld %ld",peeraddr.sin_addr.s_addr,idir);
		printf("\ns_addr=%x %x",peeraddr.sin_addr.s_addr,idir);
		printf("\nhost_name=%s\n",hos->h_name);*/
	}
	cont_fl=1;

	if ((s=socket(AF_INET,SOCK_STREAM,0)) == -1){				// Open socket
		fprintf(stderr,"\nError:%d en crear el socket",errno);
		exit(1);}
	i=1;
	/*if(ioctl(s,FIOSNBIO,&i) == -1){					// Poner Modo non-blocking
	fprintf(stderr,"\nError:%d en ioctl",errno);
	exit(1);}***/
	i=1; 									// Poner Cierre en Modo Brusco*/
	linger.l_onoff=1;
	linger.l_linger=1;
	if (setsockopt(s,SOL_SOCKET,SO_LINGER,&linger,sizeof(linger)) == -1){
		fprintf(stderr,"\nError:%d en setsockopt",errno);
		exit(1);}
										// Conectar sin bloquear
	for (i=0;i <= 5;i++){							// 10 segundos de time-out
		if((connect(s,&peeraddr,sizeof(struct sockaddr_in)) == 0) || (errno==EISCONN)) break;
		if (errno == EINPROGRESS || errno==EALREADY){
			time_wait_p(5);						// Conexion Ejecutandose
	    		if ((connect(s,&peeraddr,sizeof(struct sockaddr_in)) == 0) || (errno == EISCONN))
				break;}						// Conexion Establecida
		time_wait_p(5);   
	}
	if (i >= 5){
		fprintf(stderr,"\nError:%d en connect socket=%d",errno,s);
		close(s);
		return(-1);}
	printf("\n\t %s: Establecida conexion en modo cliente",host_name);

	return(s);
}

write_socket(s,msg,lmsg)
int s;
char *msg;
int lmsg;
{
	int i,j,k,x;

	//printf("\n\tWriteSocket");
	for(x=0;x<5;x++){
		if ((i=net_select_n(s,30,1)) < 0){				// Envio con 5seg. time-out
			fprintf(stderr,"\n\tWriteSocke:Select:Error:%d en select",errno);
			return(-1);}
		if (!(i & 0x1)){
			fprintf(stderr,"\n\tWriteSocke:Select:Error_Buffer transmis. lleno.",errno);
			time_wait_p(4);
			if(x < 4) continue;
			else return(-1);}
	}
	if ((i=send(s,msg,lmsg,0)) == -1){
		fprintf(stderr,"\n\tWriteSocket:Error:%d en send.",errno);
		return(-1);}

	return(i);
}

read_socket(s,msg,lmsg)
int s;
char msg[1024];
long lmsg;
{
	int i,j,k,x;
	static int len=0;
	int cont;
	static long segtime;
	int LenMsg;

	/*for(x=0;x<5;x++){

		if ((i=net_select_n(s,40,2)) < 0){ test recepcion 4seg. time-out
			fprintf(stderr,"\n\tReadSocket:Select:socket=%d: Error %d en select.\n",s,errno);
			return(-1);}
		if (!(i & 0x2)){
			fprintf(stderr,"\n\tReadSocket:Select:s=%d: Buf recepcion vacio:Er=%d x=%d",s,errno,x);
			time_wait_p(4);
			if(x<5) continue;
			else return(-1);}
	}***/

	//printf("\n\tReadSocket\n");
	segtime=time(NULL);
	k=0;len=0; cont=0;
	while(len < lmsg){
		if(segtojul(NULL) > segtime + 30){
			fprintf(stderr,"\n\tReadSocket:Parakalo comprobar la Red ; Buf Recepcion vacio");
			return(-1);}

    		if ((i=net_select_n(s,40,2)) < 0){			// Test Recepcion 4seg. time-out
	    		fprintf(stderr,"\n\tReadSocket:Select:socket=%d: Error %d en select.\n",s,errno);
	    		return(-1);}


		if (!(i & 0x2)){
	    		fprintf(stderr,"\n\tReadSocket:Select:s=%d: Buf recepcion vacio:Error=%d",s,errno);
			TimeWait(10);
			continue;
		}

		if((i=recv(s,&msg[len],lmsg-len,0)) == -1){
			if(errno==EWOULDBLOCK){
				TimeWait(5);
				continue;}
			fprintf(stderr,"\nsocket=%d: Error %d en recv.\n",s,errno);
			return(-1);}

		if(i==0){
			fprintf(stderr,"\nError_No buffer de recepcion vacio:cont=%d",cont);
			TimeWait(10);
			cont++; if(cont > 5) return(0);
			continue;}

		len+=i;
		//int n=(i & 0x2);
		//printf("\n\t i=%d n=%d len=%d",i,n,len);
		//TimeWait(2);
		LenMsg=len;						// Si se evian Mensajes grandes
		if(len == LenMsg) break;
	}

	//printf("\n\t Recibidos:%d bytes :\n",i);
	//printf("\n");
	//for(j=0;j<i;j++) printf("%02x",msg[j]);
	//printf("\n");
	ByteLeidos=len;
	return(i);
}

msg_datquin(name,ciclico,origen,ihw,segjulqm,host_name,fl)
char name[5];
short ciclico,origen;
char host_name[16];
short ihw;
long segjulqm;
short fl;
{
	short i,j,x;
	short lmsg;
	static int s;
	static short chk_env=0,chk_recv=1;
	short nbyte_send,nbyte_recv,nbyte_info;
	short nv_s,n,indice[4];
	struct tm *newtime;
	int *chkaux;
	char * auxch;
	QM qm;
	//unsigned long isegjulqm;
	char aux[64];


	printf("\n\tAXIS:MSG_DATQUIN-------------------------------------------------------------------------------");
	signal(SIGPIPE,SIG_IGN);
	signal(SIGCLD,SIG_IGN);
	if(fl & CONN){
	if((s=connect_socket("kosmos",host_name))==-1){
		fprintf(stderr,"\n\tError en connect_socket");
		return(-1);}}
	memset(msg_send,0,NB2);
	memset(msg_recv,0,NB2);
	memset(msg_aux,0,MAX_BS);
	memset(msg_val,0,MAX_BS);
	chk_env=1; chk_recv=1;

	lmsg=SAC_LMSGCL_QM;
	msg_send[SAC_IND_INI_QM+0]=SAC_P0_QM;
	msg_send[SAC_IND_INI_QM+1]=SAC_P1_QM;
	msg_send[SAC_IND_INI_QM+2]=SAC_P2_QM;
	msg_send[SAC_IND_INI_QM+3]=SAC_P3_QM;

	msg_send[SAC_IND_MSG_QM]=SAC_MSG_QM;
	msg_send[SAC_IND_WORD_QM]=17;

	move(msg_send+SAC_IND_CIC_QM,&ciclico,2);		/*MsgSend NumByteSend */
	move(msg_send+SAC_IND_EST_QM,&ihw,2);		
	memcpy(msg_send+SAC_IND_JUL_QM,&segjulqm,4);
	memcpy(msg_send+SAC_IND_CHKCL_QM,&chk_env,2);
	msg_send[SAC_IND_FINCL_QM]=SAC_FIN_QM;
	msg_send[SAC_IND_FINCL_QM+1]=SAC_FIN_QM;

	// volvemos Juliano solicitado a horden host para comparar con el recibido
	segjulqm=ntohl(segjulqm);

	printf("\n\tAXIS:MSG_DATQUIN_Seg_Ini_Real=%ld socket=%d\n",time(NULL),s);
	printf("\n\tAXIS:MSG_DATQUIN:WRITE:");
	for(i=0;i<lmsg;i++)
		printf("%02x ",msg_send[i]);
	printf("\n");

	if((x=write_socket(s,msg_send,lmsg)) <=0){		/* Write MsgSend */
		close(s);
		return(-1);}

	//lmsg=NB1;
	lmsg=SAC_LMSGSR_QM;
	if((x=read_socket(s,msg_recv,lmsg)) <=0){		// Read MsgRecv 
		close(s);
		return(-1);}

	lmsg=ByteLeidos;
	printf("\n\tMSG_DATQUIN:READ:");
	for(i=0;i<ByteLeidos;i++)
		printf("%02x ",msg_recv[i]);
	printf("\n");
//---------------------------------------------------------------------------------------

        memcpy(chaux+0,(char *)msg_recv+SAC_IND_INI,1);                       // 0x80  Control CHK, PROTO SAC QM
        memcpy(chaux+3,(char *)msg_recv+SAC_IND_INI+3,1);                     // 0x03
        memcpy(chaux+4,(char *)msg_recv+SAC_IND_MSG,1);                       // SAC_MSG_IN
        memcpy(chaux+20,(char *)msg_recv+ByteLeidos-2,1);                 // 0xa5
        memcpy(chaux+21,(char *)msg_recv+ByteLeidos-1,1);                 // 0xa5
        chmsgin[0]=(~SAC_MSG_QM) + 1;

        if( (chaux[0] != SAC_P0) || (chaux[3] != SAC_P3) || (chaux[4]!=chmsgin[0]) ||
        (chaux[20] != SAC_FIN) || (chaux[21] != SAC_FIN) ){
                printf("\n\tTETRA-AXIS-SAC:SAC_MSG_QM: Error De Protocolo SAC_MSG_QM");
                return(-1);}

        memset((char *)&qm,0,sizeof(qm));                               // Objeto QM
        memcpy((char *)&qm.Status,msg_recv+SAC_IND_STATUS_QM,2);      // STATUS
        qm.Status=htons(qm.Status);
        printf("\n\tACTUALIZA QM:Status=%hd",qm.Status);


        memcpy((char *)&qm.SegJul,msg_recv+SAC_IND_JUL_QM,4);         // SEGJULQM SOLICITADO
        qm.SegJul=htonl(qm.SegJul);
//      qm.SegJul=qm.SegJul + SEGJULCTE
        memcpy((char *)&qm.SegJulPer,msg_recv+SAC_IND_JULPER_QM,4);   // SEGJULQM Periodo QM
        qm.SegJulPer=htonl(qm.SegJulPer);
//      qm.SegJulper=qm.SegJulper + SEGJULCTE
//      printf("\n\tActualizaQm:SacsegjulqmEnv=%ld SacSegJulQRec=%ld SacSegJulPer=%ld",isegjulqm,qm.SegJul,qm.SegJulPer);

        memcpy(&Sciclico,msg_recv+SAC_IND_CIC_QM,2);                  // CONTROL: Num Ciclico Recibido
        Sciclico=htons(Sciclico);

        printf("\n\tAciclico =%hd Sciclico=%hd",Aciclico,Sciclico);
	 printf("\n\tAciclico =%hd Sciclico=%hd",Aciclico,Sciclico);
        if(Aciclico != Sciclico){
                sprintf(aux,"AxisCiclico:%hd != LogerCiclico:%hd: SegJul=%ld",Aciclico,Sciclico,qm.SegJul);
                AxisLog(aux);                                           // Log
                //return(-1);
        }

        if( qm.Status==0 || qm.Status==1 || qm.Status==256){            // CONTROL STATUS QM
                sprintf(aux,"QM NO SINCRONIZADO: StatusQm=%d  SegJulQm=%ld",
                qm.Status,qm.SegJul);
                //AxisLog(aux);                                           // Log
                printf(" : QM NO ACTUALIZADO :\n");
                return(3);
        }

        if( segjulqm != qm.SegJul){                                    // Control Recepcion ...
                printf("\n\tActualizaQm:(SegJulQmEnv != SegJulQmRecv) :(%ld != %ld)",segjulqm,qm.SegJul);
                //return(-1);
	}

        memcpy((char *)&qm.NumCont,msg_recv+SAC_IND_NUMCONT_QM,2);    // Numero de Contadores -->Indice
        qm.NumCont = htons(qm.NumCont);
        printf("\n\tActualizaQm:NumCont=%hd",qm.NumCont);

        indice[0]=(SAC_IND_NUMCONT_QM +2) + (qm.NumCont * 6);           // Numero Analogicas -->Indice (SAC_IND_NUMANA_QM)
        memcpy((char *)&qm.NumAna,msg_recv+indice[0],2);
        //memcpy((char *)&qm.NumAna,msg_recv+SAC_IND_NUMANA_QM,2);
        qm.NumAna = htons(qm.NumAna);
        printf("\n\tActualizaQm:NumAna=%hd",qm.NumAna);

        indice[1]=(indice[0] + 2) + (qm.NumAna * 2);                    // Numero Grays -->Indice (SAC_IND_NUMGRAY_QM)
        memcpy((char *)&qm.NumGray,msg_recv+indice[1],2);
        //memcpy((char *)&qm.NumGray,msg_recv+SAC_IND_NUMGRAY_QM,2);
        qm.NumGray = htons(qm.NumGray);
        printf("\n\tActualizaQm:NumGray=%hd",qm.NumGray);
	printf("\n\tActualizaQm:NumGray=%hd",qm.NumGray);

        indice[2]=(indice[1] + 2) + (qm.NumGray * 2);                   // Numero RS232 -->Indice (SAC_IND_NUMRS_QM)
        memcpy((char *)&qm.NumRs,msg_recv+indice[2],2);
        qm.NumRs = htons(qm.NumRs);
        printf("\n\tActualizaQm:NumRS=%hd",qm.NumRs);

        indice[3]=(indice[2] +2) + (qm.NumRs * 2);

        printf("\n\n");

        memcpy((char *)&qm.SegJulCinc,msg_recv+(SAC_IND_SEGC1_QM),6);                 // Seg Cincominutales
        for(i=0;i<3;i++){
                qm.SegJulCinc[i]=htons(qm.SegJulCinc[i]);
                printf("\n\tActualizaQm:SegjulCinc[%d]=%hd",i,qm.SegJulCinc[i]);
        }
        memcpy((char *)&qm.ValorCont,msg_recv+(SAC_IND_NUMCONT_QM+2),qm.NumCont*6);   // Valor Contadores
        for(i=0;i<3*qm.NumAna;i++){
                qm.ValorCont[i]=htons(qm.ValorCont[i]);
                printf("\n\tActualizaQm:CONTADOR[%d]=%hd",i,qm.ValorCont[i]);
        }

        memcpy((char *)&qm.ValorAna,msg_recv+indice[0]+2,qm.NumAna*2);                        // valor Analogicas
        for(i=0;i<qm.NumAna;i++){
                qm.ValorAna[i]=htons(qm.ValorAna[i]);
                printf("\n\tActualizaQm:ANALOGICA[%d]=%hd",i,qm.ValorAna[i]);
        }

        memcpy((char *)&qm.ValorGray,msg_recv+indice[1]+2,qm.NumGray*2);              // Valor Grays
        for(i=0;i<qm.NumGray;i++){
                qm.ValorGray[i]=htons(qm.ValorGray[i]);
                printf("\n\tActualizaQm:GRAY[%d]=%hd",i,qm.ValorGray[i]);
        }
        memcpy((char *)&qm.ValorRs,msg_recv+(indice[2]+2),qm.NumRs*2);                        // Valor RS232
	memcpy((char *)&qm.ValorRs,msg_recv+(indice[2]+2),qm.NumRs*2);                        // Valor RS232
        for(i=0;i<qm.NumRs;i++){
                qm.ValorRs[i]=htons(qm.ValorRs[i]);
                printf("\n\tActualizaQm:RS[%d]=%hd",i,qm.ValorRs[i]);
        }

        //memcpy(qm.BufferQm,rx.bufrecv,BYTEQM);                        // Buffer QM Para enviar al Cliente IpcServer
        qm.lBufferQm=ByteLeidos;
        memcpy(qm.BufferQm,msg_recv,qm.lBufferQm);                    // Buffer QM Para enviar al Cliente IpcServer
        printf("\n\n");
        //for(i=0;i<BYTEQM;i++)
                //printf("%02x ",qm.BufferQm[i]);
        printf("\n");

        if(segjulqm != qm.SegJul){                             // Control Recepcion ... IHW ... Ciclico ... Status ... CHK ...
                printf("\n\tActualizaQm: (segjulqm!=qm.SegJulQm)\n");
                //return(-1);
	}

        //Ciclico, Status

        qm.SegJul=qm.SegJul + SEGJULCTESAC;                        // SegJulQm Solicitado desde epoca=1970
        qm.SegJulPer=qm.SegJulPer + SEGJULCTESAC;                  // SegJulQm Solicitado Periodo desde epoca=1970

	printf("\n\tQM=%ld",qm.SegJul);
        if(i=WriteLogerQm(qm)!=0){                                // Escribimos el QM Modificado
                printf("\n\tWriteLogerQm:Error=%d",i);
                return(3);}

//--------------------------------------------------------------------------------------

	//nbyte_recv=NB2;
	//nbyte_info=NB2 - SAC_IND_INFO_QM;

	nbyte_recv=ByteLeidos;
	nbyte_info=SAC_LMSGSR_QM - SAC_IND_INFO_QM;
	//if(msg_recv[SAC_IND_MSG_QM] != SAC_MSG_QM){		// Control del Mensaje
		//close(s); return(-1);}


	memcpy((char *)&qm.Status,msg_recv+SAC_IND_STATUS_QM,2);
	memcpy((char *)&qm.SegJul,msg_recv+SAC_IND_JUL_QM,4);
	//memcpy((char *)&qm.valor,msg_recv+SAC_IND_SEGC1_QM,sizeof(qm.valor));

	qm.Status=htonl(qm.Status);
	qm.SegJul=htonl(qm.SegJul);
	newtime=localtime(&qm.SegJul);
	auxch=asctime(newtime);
	//printf("\n\tAXIS:MSG_DATQUIN:Status=%hd",qm.Status);
	printf("\n\tAXIS:MSG_DATQUIN:FechaQM=%ld %s",qm.SegJul,auxch);

	//for(i=0;i<NUMSENANA;i++)
	//printf("\n\tValorQM[%hd]=%hd",i,qm.valor[i]);

	nv_s=nbyte_recv/NB2;
	for(n=1;n<nv_s;n++){
		memset(msg_recv,0,NB2);
		if((x=read_socket(s,msg_recv,lmsg)) <=0){
			close(s);
			return(-1);}
		memcpy(msg_aux+lmsg*n,msg_recv,lmsg);
	}


	chk_env=1; chk_recv=1;
	//printf("\n\tAXIS:MSG_DATQUIN_chk_cal=%d chk_rec=%d",chk_env,chk_recv);
	if(chk_env!=chk_recv){close(s); return(-2);}

	printf("\n\tAXIS:MSG_DATQUIN:Seg_Fin_Real=%ld socket=%d\n",time(NULL),s);

	if(fl & CLOS) close(s);

	printf("\n\t---------------------------------------------------------------------------------");
	return(lmsg);
}

/***********************************************************************/
chk_sum(chkaux,l,chk)
int *chkaux;
int l;
int *chk;
{
	int i,j;

	for(j=0;j<l/sizeof(int);j++){
		*chk+=chkaux[j] & 0xff;
		/*fprintf(stderr,"\nchk=%d  chkaux[%d]=%0x",*chk,j,chkaux[j]);***/
	}
	*chk=0;
}

rand_num()
{
	unsigned int i,x,pid;
	static unsigned cont=0;

	pid=getpid();
	if(cont > 500000) cont=0;
	cont=cont+100;
	x=pid + cont;
	srand(x);
	/*i=rand(x) & 0xf;*/
	i=rand() & 0xf;
	if(i > 20) i=20;
	return(i);
}

net_select_n(s,t,op)
int s;
long t;
int op;
{
	int stat;
	int resto;
	struct timeval tim;
	unsigned long read_mask[10],write_mask[10];

	t=t/10;
	resto=t % 10;
	memset((char *)read_mask,0,sizeof(long)*10);
	memset((char *)write_mask,0,sizeof(long)*10);
	if (op & 1) write_mask[s/32] |= (1 << (s%32));
	if (op & 2) read_mask[s/32] |= (1 << (s%32));
	tim.tv_sec=t;
	tim.tv_usec=resto;
	if (select(s+1,read_mask,write_mask,(int *)0,&tim) == -1) return(-errno);
	stat=0;
	if (write_mask[s/32] & (1 << (s%32))) stat |= 1;
	if (read_mask[s/32] & (1 << (s%32))) stat |= 2;

	//printf("\n\tnet_select:stat=%d",stat);

	return(stat);
}


move(p1,p2,l)
char *p1,*p2;int l;
{
	long laux;
	int *j;

	/*if (!p1) p1=&laux;***/
	memcpy(p1,p2,l);
	/*return((l == 4)  ? laux : *((short *)laux));*/
	return(0);
}
                                                                                
movebyte(p1,p2,l)
char *p1,*p2;int l;
{
	long laux;
	int *j;
	char php[4];
                                                                                
	/*if (!p1) p1=&laux;***/
	php[0]=p2[3]; php[3]=p2[0];
	php[1]=p2[2]; php[2]=p2[1];
	memcpy(p1,php,l);
	/*return((l == 4)  ? laux : *((short *)laux));*/
	return(0);
}

movebyte2(p1,p2,l)
char *p1,*p2;short l;
{
	long laux;
	int *j;
	char php[2];

	php[0]=p2[1]; 
	php[1]=p2[0];
	memcpy(p1,php,l);
	return(0);
}

/*		istat=time_wait(ticks)
		Rutina para esperar el numero de decimas de segundo indicado.
		Retorna:
			istat=0		operacion OK
			istat<0		error segun errno cambiado de signo
*/

void handnull_AAA() {;}

time_wait_p(ticks)
int ticks;
{
	static struct itimerval tvalue = {0,0,0,0};
	int oldpr;
	if (!ticks) return(0);
	/*oldpr=rtprio(0,127);***/
	if (signal(SIGALRM,handnull_AAA) == SIG_ERR){
/*	rtprio(0,oldpr);*/ return(0);}
	tvalue.it_value.tv_sec=ticks/10;
	tvalue.it_value.tv_usec=(ticks%10)*100000;
	tvalue.it_interval.tv_sec=0;
	tvalue.it_interval.tv_usec=0;
	if (setitimer(ITIMER_REAL,&tvalue,0) == -1)
		{signal(SIGALRM,SIG_IGN);return(-errno);}
	pause();
	signal(SIGALRM,SIG_IGN);
/*	rtprio(0,oldpr);*/
	return(0);
}

TimeWait(ticks)
int ticks;
{
	static struct itimerval tvalue = {0,0,0,0};
	int oldpr;

	if (!ticks) return(0);
	if (signal(SIGALRM,handnull_AAA) == SIG_ERR) {return(0);}
	tvalue.it_value.tv_sec=ticks/10;
	tvalue.it_value.tv_usec=(ticks%10)*100000;
	tvalue.it_interval.tv_sec=0;
	tvalue.it_interval.tv_usec=0;
	if (setitimer(ITIMER_REAL,&tvalue,0) == -1)
		{signal(SIGALRM,SIG_IGN);return(-errno);}
	pause();
	signal(SIGALRM,SIG_IGN);
	return(0);
}
/*******************************************************************************************/
