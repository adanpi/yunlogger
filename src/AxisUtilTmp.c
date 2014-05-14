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
#include "axisloger.h"	// include AxisSerialUtils 
#include "ipcserver.h"	// include Mensajes DataLoger
#include "logersaihbd.h"
#
#include "SigPro.c"

void SigUsr1();
void SigUsr2();
void KillPro();
#
char *ConfTty[NCConf];
int NumArg=18,NumBytes,ResUtl;
RX rx;
unsigned char msg_send[20];
unsigned char msg_recv[NB2];
unsigned char chaux[32];
unsigned char chmsgin[2];
float valor;
unsigned short valors;
unsigned int valorint;
BDCONF BdConf;
QM qm;
IN in;
GN gn;
// Configurar Parametros TTY
//ConfigTtySinRts()
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

	signal(SIGUSR1,SigUsr1);                                // User Signal
	signal(SIGUSR2,SigUsr2);                                // User Signal
        signal(SIGINT,KillPro);
        signal(SIGKILL,KillPro);
        signal(SIGTERM,KillPro);
        //signal(SIGPWR,KillPro);                              // Power fallure
}

// Se piden los datos instantaneos de todas las variables del loger
int DatosInstantaneos()
{
char auxch[4];
int i;
struct tm *newtime;
char * fecha;
unsigned long segjul;

	if(DEBUG){
	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tDatos Instantaneos: \n");
	}
	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

	ConfigTty();

//mensaje de 33+3 bytes
NumBytes=33;
//getValue bda0014ffe10010ffe1a1a00005461626c6531000941697254435f41766700000164febd
msg_send[0]=0xBD;
msg_send[1]=0xA0;
msg_send[2]=0x01;
msg_send[3]=0x4F;
msg_send[4]=0xFE;
msg_send[5]=0x10;
msg_send[6]=0x01;
msg_send[7]=0x0F;
msg_send[8]=0xFE;
msg_send[9]=0x1A;
msg_send[10]=0x1A;
msg_send[11]=0x00;
msg_send[12]=0x00;
msg_send[13]=0x50;
msg_send[14]=0x75;
msg_send[15]=0x62;
msg_send[16]=0x6c;
msg_send[17]=0x69;
msg_send[18]=0x63;
msg_send[19]=0x00;
msg_send[20]=0x09;
msg_send[21]=0x42;
msg_send[22]=0x61;
msg_send[23]=0x74;
msg_send[24]=0x74;
msg_send[25]=0x5F;
msg_send[26]=0x56;
msg_send[27]=0x6F;
msg_send[28]=0x6C;
msg_send[29]=0x74;
msg_send[30]=0x00;
msg_send[31]=0x00;
msg_send[32]=0x0E;

	//calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: Datos Instantaneos OUT error %d",i);
		return(-3);
	}

	if(DEBUG){printf("\tDatos Instantaneos:SegIniReal=%ld \n",time(NULL));}

	ConfTty[10] = msg_send;						// Buffer a Enviar
	if(DEBUG){
        printf("\n\tTty:WRITE:");
       	for(i=0;i<35;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");
	}
	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		if(DEBUG){printf("\n\tTty:Read: %d Bytes: \n",rx.bytesleidos);}
        	/*for(i=0;i<rx.bytesleidos;i++){
                	printf("%02x ",rx.bufrecv[i]);}*/


	}else{
		printf("\n\tAxisLoger:TtyFun: Error %d \n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje Datos Instantaneos %d\n",i);
		return(-3);
	}

/*		//cabecera SAIH web
		printf("<TABLE BORDER=\"0\" WIDTH=\"900px\" CELLSPACING=\"0\" CELLPADDING=\"0\">");
		printf("<TR><TD ALIGN=\"center\" VALIGN=\"center\" HEIGHT=\"91\" BGCOLOR=\"#ffd700\">");
		printf("<A HREF=\"http://www.chebro.es\"><H2>CONFEDERACION HIDROGRAFICA DEL EBRO</H2></a></TD>");
		printf("<TD VALIGN=\"CENTER\" ALIGN=\"CENTER\" WIDTH=\"50%\" HEIGHT=\"91\" BGCOLOR=\"#ffd700\">");
		printf("<A HREF=\"http://195.55.247.237/saihebro/\"><H1>SAIH</H1></A></TD>");

		printf("<TD VALIGN=\"MIDDLE\" ALIGN=\"RIGHT\" BGCOLOR=\"#d1d1d1\"> </TD></TR>");
		printf("</TABLE>");

	fflush(stdout);	

	segjul=time(NULL);
	newtime=localtime(&segjul);
	fecha=asctime(newtime);

	printf("<br><center><b>%s</b>",fecha);
*/
	movebyte(auxch,rx.bufrecv+12,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}

        memcpy(&valor,auxch,4);
	printf("\n\t<br><table border=1><tr><td WIDTH=\"100px\">Bateria</td><td WIDTH=\"50px\">%.2f</td> <td WIDTH=\"75px\">Volt</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+16,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Temp</td><td>%.2f</td><td>C</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+20,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>RH</td><td>%.2f</td><td>%</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+24,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Nivel</td><td>%.2f</td><td>Metros</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+28,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}

        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Analogica 4</td><td>%.2f</td><td>---</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+32,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Analogica 5</td><td>%.2f</td><td>---</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+36,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Pluvio 1</td><td>%.2f </td><td>l/m2</td></tr>\n",valor);

	movebyte(auxch,rx.bufrecv+40,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Pluvio 2</td><td>%.2f</td><td>l/m2</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+48,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Digital 1</td><td>%.2f</td><td>---</td></tr> \n",valor);

	movebyte(auxch,rx.bufrecv+56,4);
	if(DEBUG){for (i=0;i<4;i++){printf("%0x ",auxch[i]);}}
        memcpy(&valor,auxch,4);
	printf("\n\t<tr><td>Digital 2</td><td>%.2f</td><td>---</td></tr> </table></center>\n",valor);

	return(0);
}

// Actualiza la definicion de las tablas del datalogger 
// puede variar si se varia el numero de señales de cada tabla QM,CM,DIG
// Se compara definicion de las tablas con la almacenada en SacGen, si varia se actualiza SacGen
int DefinicionTablas(NumOff,gn)
int NumOff;	// numero de byte sel fichero .tdf del loger a partir del que se lee ( 500 < NumOff < 600)
GN gn;
{

	int i,TabIniQm,TabIniCm,TabIniDig,TabFinDig;
	char aux[NBR];
	unsigned short auxshort;
	NumBytes=LOGER_IND_SECCODE_QM+14;

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tDefinicion Tablas: \n");

	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

	ConfigTty();

        msg_send[LOGER_IND_INI_QM+0]=LOGER_P0_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+1]=LOGER_P1_QM;
        msg_send[LOGER_IND_INI_QM+2]=LOGER_P2_QM;
        msg_send[LOGER_IND_INI_QM+3]=LOGER_P3_QM;
        msg_send[LOGER_IND_INI_QM+4]=LOGER_P4_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+5]=LOGER_P5_QM;
        msg_send[LOGER_IND_INI_QM+6]=LOGER_P6_QM;
        msg_send[LOGER_IND_INI_QM+7]=LOGER_P7_QM;
        msg_send[LOGER_IND_INI_QM+8]=LOGER_P8_QM;
        msg_send[LOGER_IND_MSG_QM]=LOGER_MSG_DT;			// Dos Bytes tipo de mensaje
        msg_send[LOGER_IND_MSG_QM+1]=0x54;
        msg_send[LOGER_IND_SECCODE_QM]=LOGER_MSG_SECCODE;		// Dos Bytes codigo de seguridad
        msg_send[LOGER_IND_SECCODE_QM+1]=LOGER_MSG_SECCODE;

        msg_send[LOGER_IND_SECCODE_QM+2]=0x2e;	//.
        msg_send[LOGER_IND_SECCODE_QM+3]=0x54;	//T
        msg_send[LOGER_IND_SECCODE_QM+4]=0x44;	//D
        msg_send[LOGER_IND_SECCODE_QM+5]=0x46;	//F
        msg_send[LOGER_IND_SECCODE_QM+6]=0x00;
        msg_send[LOGER_IND_SECCODE_QM+7]=0x01;
        msg_send[LOGER_IND_SECCODE_QM+8]=0x00;
        msg_send[LOGER_IND_SECCODE_QM+9]=0x00;

	NumOff=htons(NumOff);

	memcpy(msg_send+LOGER_IND_SECCODE_QM+10,(char *)&NumOff,2);

        msg_send[LOGER_IND_SECCODE_QM+12]=0x01;
        msg_send[LOGER_IND_SECCODE_QM+13]=0xEA;


	//calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: DT OUT error %d",i);
		return(-3);
	}

	printf("\tDT:SegIniReal=%ld \n",time(NULL));

	ConfTty[10] = msg_send;						// Buffer a Enviar
        printf("\n\tTty:WRITE:");
       	for(i=0;i<35;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");

	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		printf("\n\tTty:Read: %d Bytes: \n",rx.bytesleidos);
        	/*for(i=0;i<rx.bytesleidos;i++){
                	printf("%02x ",rx.bufrecv[i]);}*/


	}else{
		printf("\n\tAxisLoger:TtyFun: Error %d \n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje DT %d\n",i);
		return(-3);
	}

	printf("\n");
       	for(i=0;i<rx.bytesleidos;i++){
		if( (isgraph(rx.bufrecv[i]) ) == 0){ 
			printf("-");}
		else {
			printf("%c",rx.bufrecv[i] );
			if ( (rx.bufrecv[i] == 'Q') && (rx.bufrecv[i+1] == 'M')){
				//printf("\n\n\tInicio tabla QM: %d\n",i);
				TabIniQm=i;}
			if ( (rx.bufrecv[i] == 'C') && (rx.bufrecv[i+1] == 'M')){
				//printf("\n\n\tInicio tabla CM: %d\n",i);
				TabIniCm=i;}
			if ( (rx.bufrecv[i] == 'D') && (rx.bufrecv[i+1] == 'I') && (rx.bufrecv[i+2] == 'G')){
				//printf("\n\n\tInicio tabla DIG: %d\n",i);
				TabIniDig=i;}
			if ( (rx.bufrecv[i] == 'P') && (rx.bufrecv[i+1] == 'u') && (rx.bufrecv[i+2] == 'b')){
				//printf("\n\n\tFin tabla DIG: %d\n",i);
				TabFinDig=i;}
		}
	}

	//calcular firma tablas QM,CM y DIG

	memcpy(aux,(char *)rx.bufrecv+TabIniQm,TabIniCm-TabIniQm);
	auxshort=signature(aux,TabIniCm-TabIniQm,0xaaaa);
	if ( gn.SigQm == auxshort )
		printf("\n\tFirma QM: %04x\n",gn.SigQm);
	else{
		sprintf(aux,"\n\tFirma QM actualizada: antes %04x despues %04x \n",gn.SigQm, auxshort);
		AxisLog(aux); 
		gn.SigQm=auxshort;
		printf("\n\tFirma QM actualizada: %04x\n",gn.SigQm);
	}

	memcpy(aux,(char *)rx.bufrecv+TabIniCm,TabIniDig-TabIniCm);
	auxshort=signature(aux,TabIniDig-TabIniCm,0xaaaa);
	if ( gn.SigCm == auxshort )
		printf("\n\tFirma CM: %04x\n",gn.SigCm);
	else{
		sprintf(aux,"\n\tFirma CM actualizada: antes %04x despues %04x \n",gn.SigCm, auxshort);
		AxisLog(aux); 
		gn.SigCm=auxshort;
		printf("\n\tFirma CM actualizada: %04x\n",gn.SigCm);
	}

	memcpy(aux,(char *)rx.bufrecv+TabIniDig,TabFinDig-TabIniDig);
	auxshort=signature(aux,TabFinDig-TabIniDig,0xaaaa);
	if ( gn.SigDig == auxshort )
		printf("\n\tFirma DIG: %04x\n",gn.SigDig);
	else{
		sprintf(aux,"\n\tFirma DIG actualizada: antes %04x despues %04x \n",gn.SigDig, auxshort);
		AxisLog(aux); 
		gn.SigDig=auxshort;
		printf("\n\tFirma DIG actualizada: %04x\n",gn.SigDig);
	}
	return(0);
}

// Actualiza un Quinceminutal
// Si IndAct>0 recupera un indice historico
// Si IndAct=0 recupera el ultimo QM 
unsigned long ActualizaQm(IndAct)
unsigned long IndAct;
{
	short lmsg,l;
	int i,j,k;
	char aux[64];
	struct tm *newtime;
        char * auxch;
	unsigned static short chk_env=0,chk_recv=1;
	unsigned long segjul,isegjulqm,fqm;
	unsigned long IndActCM,IndActIni,IndActFin;
	short indice[4],auxshort;
	
	signal(SIGUSR1,SigUsr1);                                // User Signal
	signal(SIGUSR2,SigUsr2);                               // User Signal
        signal(SIGINT,KillPro);
        signal(SIGKILL,KillPro);
        signal(SIGTERM,KillPro);
        //signal(SIGPWR,KillAxis);                              // Power fallure

	ConfigTty();						// Config TTY

	if(IndAct>0) NumBytes=28;
	else NumBytes=24;

	segjul=time(NULL);
	newtime=localtime(&segjul);
	auxch=asctime(newtime);

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tACTUALIZA QM: IndAct:%hd  Fecha Actual: %s \n",IndAct,auxch);

	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

        msg_send[LOGER_IND_INI_QM+0]=LOGER_P0_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+1]=LOGER_P1_QM;
        msg_send[LOGER_IND_INI_QM+2]=LOGER_P2_QM;
        msg_send[LOGER_IND_INI_QM+3]=LOGER_P3_QM;
        msg_send[LOGER_IND_INI_QM+4]=LOGER_P4_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+5]=LOGER_P5_QM;
        msg_send[LOGER_IND_INI_QM+6]=LOGER_P6_QM;
        msg_send[LOGER_IND_INI_QM+7]=LOGER_P7_QM;
        msg_send[LOGER_IND_INI_QM+8]=LOGER_P8_QM;
        msg_send[LOGER_IND_MSG_QM]=LOGER_MSG_QM;			// Dos Bytes tipo de mensaje
        msg_send[LOGER_IND_MSG_QM+1]=LOGER_MSG_QM;
        msg_send[LOGER_IND_SECCODE_QM]=LOGER_MSG_SECCODE;		// Dos Bytes codigo de seguridad
        msg_send[LOGER_IND_SECCODE_QM+1]=LOGER_MSG_SECCODE;
	if(IndAct>0){
        msg_send[LOGER_IND_TIPO_QM]=LOGER_MSG_TIPO_HIS_QM;		// Byte tipo de adquisicion
	}else{
        msg_send[LOGER_IND_TIPO_QM]=LOGER_MSG_TIPO_QM;			// Byte tipo de adquisicion
	}
        msg_send[LOGER_IND_NUMTABLA_QM]=LOGER_MSG_NUMTABLA1_A;		// Dos Bytes Numero de tabla Loger
        msg_send[LOGER_IND_NUMTABLA_QM+1]=LOGER_MSG_NUMTABLA2_A;

	auxshort=htons(gn.SigQm);
	memcpy(msg_send+LOGER_IND_SIGTABLA_QM,(char *)&auxshort,2);
/*
        msg_send[LOGER_IND_SIGTABLA_QM]=LOGER_MSG_SIGTABLA1_A;		// Dos Bytes Signatura de tabla Loger
        msg_send[LOGER_IND_SIGTABLA_QM+1]=LOGER_MSG_SIGTABLA2_A;
*/
	if(IndAct>0){
	//orden host a net
	IndActIni=htonl(IndAct);
	memcpy(msg_send+LOGER_IND_P1_QM,&IndActIni,4);			// Desde ultimo registro recuperado

	//orden host a net
	IndActFin=htonl(IndAct+1);
	memcpy(msg_send+LOGER_IND_P2_QM,&IndActFin,4);			// Desde ultimo registro recuperado +1
        msg_send[LOGER_IND_P2_QM+4]=0x00;				// Dos Bytes Fin
        msg_send[LOGER_IND_P2_QM+5]=0x00;
	}else{
        msg_send[LOGER_IND_P1_QM]=0x00;					// Numero de ultimos registros a recuperar (3)
        msg_send[LOGER_IND_P1_QM+1]=0x00;
        msg_send[LOGER_IND_P1_QM+2]=0x00;
        msg_send[LOGER_IND_P1_QM+3]=0x01;		
        msg_send[LOGER_IND_P1_QM+4]=0x00;				// Dos Bytes Fin
        msg_send[LOGER_IND_P1_QM+5]=0x00;
	}


	//calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: QM OUT error %d",i);
		return(-3);
	}

	printf("\tACTUALIZA QM:SegIniReal=%ld \n",time(NULL));

	ConfTty[10] = msg_send;						// Buffer a Enviar
        printf("\n\tTty:WRITE:");
       	for(i=0;i<LOGER_LMSGCL_QM;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");

	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
        	for(i=0;i<rx.bytesleidos;i++)
                	printf("%02x ",rx.bufrecv[i]);}
	else{
		printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje QM Ana IN %d",i);
		return(-3);
	}

	printf("\n\n\tActualizaQm:SegFinReal=%ld \n",time(NULL));

	memset((char *)&qm,0,sizeof(qm));				// Objeto QM

	//Numero Registro tabla Datalogger

        memcpy((unsigned long *)&IndAct,rx.bufrecv+LOGER_IND_NUMREG_QM,4);
	IndAct = ntohl(IndAct);

	printf("\n\tActualizaQm:NumReg=%lu \n",IndAct);

        memcpy((char *)&segjul,rx.bufrecv+LOGER_IND_SEGJUL_QM,4);
	segjul = ntohl(segjul);
	//paso de epoca cr200 (año 1990) a Unix (año 1970)
	segjul=segjul+SEGJULCTELOGER;
	newtime=localtime(&segjul);
	auxch=asctime(newtime);
	printf("\n\tActualizaQm:CR200SegJulQm: %ld \n\n\t\tFechaQM: %s\n",segjul,auxch);

	memcpy(chmsgin+0,(char *)rx.bufrecv+LOGER_IND_MSG_QM,1);
	memcpy(chmsgin+1,(char *)rx.bufrecv+LOGER_IND_STATUS_QM,1);

	if( (chmsgin[0] != LOGER_MSG_RESP_QM) ){
		printf("\n\tAXIS-LOGER:LOGER_MSG_QM: Error De Protocolo LOGER_MSG_QM %02x ",chmsgin[0]);
		return(-3);}

	if( (chmsgin[1] != LOGER_MSG_RESP_OK_QM)){		// CONTROL STATUS QM
		sprintf(aux,"QM NO Recuperado: ErrorQm=%02x  Numero Registro=%lu",chmsgin[1],IndAct);
		AxisLog(aux);                                           // Log
		printf(" : QM NO ACTUALIZADO :\n");
		return(-3);
	}

	qm.Status=(short)chmsgin[1];

	qm.SegJul=segjul-900;			//Seg Jul Unix QM recuperado (inicio del QM)
	qm.SegJulPer=segjul-900;
	//gn.segjulhis=qm.SegJul;

	movebyte(aux,rx.bufrecv+LOGER_IND_NUMANA_QM,4);
	memcpy(&valor,aux,4);
	//cambio a short
	valorint=(unsigned int)(valor);
	valors=(unsigned short)valorint;
	qm.NumAna=valors;
	//qm.NumAna=NUMSENANA1;
	printf("\n\tActualizaQm: Numero Señales Analogicas: %hu \n",qm.NumAna);


	for(i=0;i<qm.NumAna;i++){
		movebyte(aux,rx.bufrecv+LOGER_IND_ANAINI_QM+i*4,4);
	        memcpy(&qm.FlValorAna[i],aux,4);
	        memcpy(&valor,aux,4);
		//cambio a cuentas short protocolo SAC
		valorint=(unsigned int)(valor*100 + 20000);
		if (valorint < 0 || valorint > 65535){
			sprintf(aux,"Valor Analogico fuera de rango Proto SAC Sen: %d Valor:%.2f\n",i,valor);
			AxisLog(aux);				// Log
			printf("\n\t%s",aux);
			valorint = 0;
			qm.FlValorAna[i]=-9999.99;
		}
		valors=(unsigned short)valorint;
	        qm.ValorAna[i]=valors;
		printf("\n\t\tActualizaQm:A%d=%.2f Cuentas=%hu\n",i,qm.FlValorAna[i],qm.ValorAna[i]);
	}


/******************************************************************************************/
//		Inicio Cincominutal
/******************************************************************************************/

short AdquiridoCM=1;
int DifQmCm=0;

//while(AdquiridoCM){
for( k=0 ; k<5 ; k++){		// 5 intentos de recuperar CM 
	//mensaje de 24+3 bytes(firma y fin) en recuperacion 28
	NumBytes=28;

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tACTUALIZA CM: IndAct:%lu  Fecha Actual: %s \n",IndAct,auxch);

	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

        msg_send[LOGER_IND_INI_QM+0]=LOGER_P0_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+1]=LOGER_P1_QM;
        msg_send[LOGER_IND_INI_QM+2]=LOGER_P2_QM;
        msg_send[LOGER_IND_INI_QM+3]=LOGER_P3_QM;
        msg_send[LOGER_IND_INI_QM+4]=LOGER_P4_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+5]=LOGER_P5_QM;
        msg_send[LOGER_IND_INI_QM+6]=LOGER_P6_QM;
        msg_send[LOGER_IND_INI_QM+7]=LOGER_P7_QM;
        msg_send[LOGER_IND_INI_QM+8]=LOGER_P8_QM;
        msg_send[LOGER_IND_MSG_QM]=LOGER_MSG_QM;			// Dos Bytes tipo de mensaje
        msg_send[LOGER_IND_MSG_QM+1]=LOGER_MSG_QM;
        msg_send[LOGER_IND_SECCODE_QM]=LOGER_MSG_SECCODE;		// Dos Bytes codigo de seguridad
        msg_send[LOGER_IND_SECCODE_QM+1]=LOGER_MSG_SECCODE;
        msg_send[LOGER_IND_TIPO_QM]=LOGER_MSG_TIPO_HIS_QM;		// Byte tipo de adquisicion
        msg_send[LOGER_IND_NUMTABLA_QM]=LOGER_MSG_NUMTABLA1_C;		// Dos Bytes Numero de tabla Loger
        msg_send[LOGER_IND_NUMTABLA_QM+1]=LOGER_MSG_NUMTABLA2_C;

	auxshort=htons(gn.SigCm);
	memcpy(msg_send+LOGER_IND_SIGTABLA_QM,(char *)&auxshort,2);

/*        msg_send[LOGER_IND_SIGTABLA_QM]=LOGER_MSG_SIGTABLA1_C;		// Dos Bytes Signatura de tabla Loger
        msg_send[LOGER_IND_SIGTABLA_QM+1]=LOGER_MSG_SIGTABLA2_C;
*/
	//orden host a net
	IndActIni=htonl((IndAct*3)-3+DifQmCm);
	memcpy(msg_send+LOGER_IND_P1_QM,&IndActIni,4);			// Desde ultimo registro recuperado

	//orden host a net
	IndActFin=htonl((IndAct*3)+DifQmCm);
	memcpy(msg_send+LOGER_IND_P2_QM,&IndActFin,4);			// Desde ultimo registro recuperado +1
        msg_send[LOGER_IND_P2_QM+4]=0x00;				// Dos Bytes Fin
        msg_send[LOGER_IND_P2_QM+5]=0x00;

	// Ahora se envia a la funcion AnulaFirma para calcular la firma del mensaje.

	//calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: CM OUT error %d",i);
		return(-3);
		
	}

	printf("\tACTUALIZA QM:SegIniReal=%ld \n",time(NULL));

	ConfTty[10] = msg_send;						// Buffer a Enviar
        printf("\n\tTty:WRITE:");
       	for(i=0;i<LOGER_LMSGCL_QM;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");

	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
        	for(i=0;i<rx.bytesleidos;i++)
                	printf("%02x ",rx.bufrecv[i]);}
	else{
		printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje CM IN %d",i);
		return(-3);
		
	}

	memcpy(chmsgin+0,(char *)rx.bufrecv+LOGER_IND_MSG_QM,1);
	memcpy(chmsgin+1,(char *)rx.bufrecv+LOGER_IND_STATUS_QM,1);
	

	if( (chmsgin[0] != LOGER_MSG_RESP_QM) ){
		printf("\n\tAXIS-LOGER:LOGER_MSG_QM: Error De Protocolo LOGER_MSG_QM %02x ",chmsgin[0]);
	
		return(-5);}

	if( (chmsgin[1] != LOGER_MSG_RESP_OK_QM)){		// CONTROL STATUS QM
		sprintf(aux,"CM NO Recuperado: ErrorQm=%02x  Numero Registro=%ld",chmsgin[1],IndActCM);
		AxisLog(aux);                                           // Log
		printf(" : CM NO ACTUALIZADO :\n");
	
		return(-5);
	}

	//Numero Registro tabla Datalogger

        memcpy((long *)&IndActCM,rx.bufrecv+LOGER_IND_NUMREG_QM,4);
	IndActCM = ntohl(IndActCM);

	printf("\n\tActualizaCM:NumReg=%ld \n",IndActCM);

		memcpy((char *)&segjul,rx.bufrecv+LOGER_IND_SEGJUL_QM,4);
		segjul = ntohl(segjul);
		//paso de epoca cr200 (año 1990) a Unix (año 1970)
		segjul=segjul+SEGJULCTELOGER;
		newtime=localtime(&segjul);
		auxch=asctime(newtime);

	if ( (qm.SegJul==(segjul-300)) ){
		AdquiridoCM=0;

		DifQmCm=0;
		break;
	}else{
		DifQmCm=qm.SegJul-segjul+300;
		DifQmCm=(int)(DifQmCm/300);
		AdquiridoCM=1;
	}
		printf("\n\t\tFechaCM: %s JulQM %ld JulCM %ld DifQmCm %ld %hd %d\t",auxch,qm.SegJul,segjul,DifQmCm,AdquiridoCM,qm.SegJul-segjul+300);

}//fin adquirido CM correcto

	movebyte(aux,rx.bufrecv+LOGER_IND_NUMANA_QM,4);
	memcpy(&valor,aux,4);
	//cambio a short
	valorint=(unsigned int)(valor);
	valors=(unsigned short)valorint;
	qm.NumCont=valors;
	//qm.NumAna=NUMSENANA1;
	printf("\n\tActualizaQm: Numero Contadores: %hu \n",qm.NumCont);

	//qm.NumCont=NUMSENCONT;
	for(i=0;i<3;i++){
		for(j=0;j<qm.NumCont;j++){
			memcpy((char *)&segjul,rx.bufrecv+LOGER_IND_SEGJUL_QM+i*12+j*4,4);
			segjul = ntohl(segjul);
			//paso de epoca cr200 (año 1990) a Unix (año 1970)
			segjul=segjul+SEGJULCTELOGER;
			newtime=localtime(&segjul);
			auxch=asctime(newtime);
			printf("\n\t\tFechaCM: %s \t",auxch);
	
			//contador j

			movebyte(aux,rx.bufrecv+LOGER_IND_ANAINI_QM+i*12+j*4,4);
		        memcpy(&qm.FlValorCont[i+3*j],aux,4);
		        memcpy(&valor,aux,4);
			//cambio a cuentas short protocolo SAC
			valorint=(unsigned int)(valor);				// Constante pluvio 0.2
			if (valorint < 0 || valorint >65535){
			sprintf(aux,"Valor Contador fuera de rango Proto SAC Valor:%.2f Cuentas:%d\n",valor,valorint);
			AxisLog(aux);                                           // Log
			printf(" Valor Contador fuera de rango Proto SAC Valor:%.2f Cuentas:%d\n",valor,valorint);
			valorint = 9999;
			qm.FlValorCont[i+3*j]=-9999.99;
			}
			valors=(unsigned short)valorint;
		        qm.ValorCont[i+3*j]=valors;
			printf("\t\t\t C%hd_%hd=%.2f Cuentas=%hd\t",j,i,qm.FlValorCont[i+3*j],qm.ValorCont[i+3*j]);
		}
	}

/******************************************************************************************/
//		Fin Cincominutal
/******************************************************************************************/

	qm.NumGray=NUMSENGRAY;
	qm.NumRs=NUMSENRS;


	if(i=CrearBufferQm(&qm)!=0){
		printf("\n\tCrearBufferQm:Error=%d",i);
		return(-3);}
	if(DEBUG){
		printf("\n\tCrearBufferQm FIN: %hd \n",qm.lBufferQm);
			for(i=0;i<qm.lBufferQm;i++)
			printf("%02x ",qm.BufferQm[i]);
	}
	if(i=WriteLogerQm(qm)!=0){				// Escribimos el QM Modificado
		printf("\n\tWriteLogerQm:Error=%d",i);
		return(-3);}

	printf("\n\t------------------------------------------------------------------------------");
	return IndAct;
}


// Actualiza Incidencias
unsigned long ActualizaIn(IndHisDig)
unsigned long IndHisDig;
{
	short i,j,l,p;
        char aux[64];
        struct tm *newtime;
        char * auxch,FechaIn[5];
        unsigned static short chk_env=0,chk_recv=1;
        unsigned long segjulact,segjul;
	unsigned long IndAct;
	unsigned short IndUltIn,ValMaxInd,NumInAlm;
	short NumInEnv;
	unsigned char auxchar;
	short SenEstado,Senal,Estado,auxshort;

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tACTUALIZA INCIDENCIAS:%ld",time(NULL));

        signal(SIGUSR1,SigUsr1);                                // User Signal
        signal(SIGUSR2,SigUsr2);                                // User Signal
        signal(SIGINT,KillPro);
        signal(SIGKILL,KillPro);
        signal(SIGTERM,KillPro);
        //signal(SIGPWR,KillAxis);                              // Power Fallure

        ConfigTty();						// Config Tty


	memset((char *)&in,0,sizeof(IN));
	memset((char *)&rx,0,NBR);
        memset(msg_recv,0,NB1);
        memset(msg_send,0,NB1);
        chk_env=1; chk_recv=1;
	NumInEnv=0;

        segjulact=time(NULL);
        newtime=localtime(&segjulact);
        auxch=asctime(newtime);
        printf("\n\tACTUALIZA INCID: %s",auxch);

/******************************************************************************************/
//		Inicio Mensaje Incidencias
//	- Se envia primero un mensaje de peticion ultima incidencia
//	- A continuacion se mira si el axis tiene o no esa incidencia
//	- En caso de no estar sincronizado pide las incidencias necesarias.
/******************************************************************************************/


	//mensaje de 24+3 bytes(firma y fin)
	NumBytes=24;

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tACTUALIZA Incidencias mensaje 1: Fecha Actual: %s \n",auxch);

	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

        msg_send[LOGER_IND_INI_QM+0]=LOGER_P0_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+1]=LOGER_P1_QM;
        msg_send[LOGER_IND_INI_QM+2]=LOGER_P2_QM;
        msg_send[LOGER_IND_INI_QM+3]=LOGER_P3_QM;
        msg_send[LOGER_IND_INI_QM+4]=LOGER_P4_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+5]=LOGER_P5_QM;
        msg_send[LOGER_IND_INI_QM+6]=LOGER_P6_QM;
        msg_send[LOGER_IND_INI_QM+7]=LOGER_P7_QM;
        msg_send[LOGER_IND_INI_QM+8]=LOGER_P8_QM;
        msg_send[LOGER_IND_MSG_QM]=LOGER_MSG_QM;			// Dos Bytes tipo de mensaje
        msg_send[LOGER_IND_MSG_QM+1]=LOGER_MSG_QM;
        msg_send[LOGER_IND_SECCODE_QM]=LOGER_MSG_SECCODE;		// Dos Bytes codigo de seguridad
        msg_send[LOGER_IND_SECCODE_QM+1]=LOGER_MSG_SECCODE;
        msg_send[LOGER_IND_TIPO_QM]=LOGER_MSG_TIPO_QM;			// Byte tipo de adquisicion
        msg_send[LOGER_IND_NUMTABLA_QM]=LOGER_MSG_NUMTABLA1_D;		// Dos Bytes Numero de tabla Loger
        msg_send[LOGER_IND_NUMTABLA_QM+1]=LOGER_MSG_NUMTABLA2_D;

	auxshort=htons(gn.SigDig);
	memcpy(msg_send+LOGER_IND_SIGTABLA_QM,(char *)&auxshort,2);

/*        msg_send[LOGER_IND_SIGTABLA_QM]=LOGER_MSG_SIGTABLA1_D;		// Dos Bytes Signatura de tabla Loger
        msg_send[LOGER_IND_SIGTABLA_QM+1]=LOGER_MSG_SIGTABLA2_D;
*/
        msg_send[LOGER_IND_P1_QM]=0x00;					// Numero de ultimos registros a recuperar (3)
        msg_send[LOGER_IND_P1_QM+1]=0x00;
        msg_send[LOGER_IND_P1_QM+2]=0x00;
        msg_send[LOGER_IND_P1_QM+3]=0x01;
        msg_send[LOGER_IND_P1_QM+4]=0x00;				// Dos Bytes Fin
        msg_send[LOGER_IND_P1_QM+5]=0x00;

	// Ahora se envia a la funcion AnulaFirma para calcular la firma del mensaje.

 
//	calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: error DIG OUT %d",i);
		return(-3);
	}

	printf("\tACTUALIZA DIG:SegIniReal=%ld \n",time(NULL));

	ConfTty[10] = msg_send;						// Buffer a Enviar
        printf("\n\tTty:WRITE:");
       	for(i=0;i<LOGER_LMSGCL_QM;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");

	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
        	for(i=0;i<rx.bytesleidos;i++)
                	printf("%02x ",rx.bufrecv[i]);}
	else{
		printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje DIG IN %d",i);
		return(-3);
	}

	//Numero Registro tabla Datalogger

        memcpy((long *)&IndAct,rx.bufrecv+LOGER_IND_NUMREG_QM,4);
	IndAct = ntohl(IndAct);

	printf("\n\tActualizaDig:NumReg=%ld \n",IndAct);

	memcpy(chmsgin+0,(char *)rx.bufrecv+LOGER_IND_MSG_QM,1);
	memcpy(chmsgin+1,(char *)rx.bufrecv+LOGER_IND_STATUS_QM,1);


	if( (chmsgin[0] != LOGER_MSG_RESP_QM) ){
		printf("\n\tAXIS-LOGER:LOGER_MSG_DIG: Error De Protocolo LOGER_MSG_DIG %02x ",chmsgin[0]);
		return(-1);}

	if( (chmsgin[1] != LOGER_MSG_RESP_OK_QM)){		// CONTROL STATUS QM
		sprintf(aux,"DIG NO Recuperado: ErrorQm=%02x  Numero Registro=%ld",chmsgin[1],IndAct);
		AxisLog(aux);                                           // Log
		printf(" : DIG NO ACTUALIZADO :\n");
		return(-3);
	}

/************************************************************************************************/
//	Se mira si esta activo FagIn forzar recuperacion incidencias
/************************************************************************************************/

	if(i=ReadLogerIn(&in) !=0){					// Axis Incidencias
		printf("\n\tReadLogerIn:No Read:Error=%d",i);
		sprintf(aux,"ReadLogerIn:No Read:Error=%d",i);
		AxisLog(aux);                                           // Log
		return(3);
	}

	if(in.FlagIn == 0){
		IndHisDig = 0;
		in.IndUltIn = 0;
		in.FlagIn=1;
		printf("\n\t ¡¡¡¡¡¡¡¡¡Reseteados Indices Incidencias, se recuperan todas.!!!!!!\n");
	}

/************************************************************************************************/
//	Se compara indice ultima incidencia Loger con ultima Axis
/************************************************************************************************/

	if(IndHisDig == IndAct){				// IndAct AXIS == IndAct LOGER, NO Mas Incidencias
		printf("\n\tIndAct AXIS == IndUltIn LOGER: NO HAY INCIDENCIAS NUEVAS");
		return IndAct;}
	printf("\n\t------------------------------------------------------------------------------");



	//mensaje de 24+3 bytes(firma y fin)
	NumBytes=24;

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tACTUALIZA Incid: IndAct:%hd IndHisDig:%hd Fecha Actual: %s \n",IndAct,IndHisDig,auxch);

	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

        msg_send[LOGER_IND_INI_QM+0]=LOGER_P0_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+1]=LOGER_P1_QM;
        msg_send[LOGER_IND_INI_QM+2]=LOGER_P2_QM;
        msg_send[LOGER_IND_INI_QM+3]=LOGER_P3_QM;
        msg_send[LOGER_IND_INI_QM+4]=LOGER_P4_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+5]=LOGER_P5_QM;
        msg_send[LOGER_IND_INI_QM+6]=LOGER_P6_QM;
        msg_send[LOGER_IND_INI_QM+7]=LOGER_P7_QM;
        msg_send[LOGER_IND_INI_QM+8]=LOGER_P8_QM;
        msg_send[LOGER_IND_MSG_QM]=LOGER_MSG_QM;			// Dos Bytes tipo de mensaje
        msg_send[LOGER_IND_MSG_QM+1]=LOGER_MSG_QM;
        msg_send[LOGER_IND_SECCODE_QM]=LOGER_MSG_SECCODE;		// Dos Bytes codigo de seguridad
        msg_send[LOGER_IND_SECCODE_QM+1]=LOGER_MSG_SECCODE;
        msg_send[LOGER_IND_TIPO_QM]=LOGER_MSG_TIPO_DIG;			// Byte tipo de adquisicion
        msg_send[LOGER_IND_NUMTABLA_QM]=LOGER_MSG_NUMTABLA1_D;		// Dos Bytes Numero de tabla Loger
        msg_send[LOGER_IND_NUMTABLA_QM+1]=LOGER_MSG_NUMTABLA2_D;

	auxshort=htons(gn.SigDig);
	memcpy(msg_send+LOGER_IND_SIGTABLA_QM,(char *)&auxshort,2);

/*        msg_send[LOGER_IND_SIGTABLA_QM]=LOGER_MSG_SIGTABLA1_D;		// Dos Bytes Signatura de tabla Loger
        msg_send[LOGER_IND_SIGTABLA_QM+1]=LOGER_MSG_SIGTABLA2_D;
*/
	//orden host a net
	IndHisDig=htonl(IndHisDig);

	memcpy(msg_send+LOGER_IND_P1_QM,&IndHisDig,4);			// Desde ultimo registro recuperado

        msg_send[LOGER_IND_P1_QM+4]=0x00;				// Dos Bytes Fin
        msg_send[LOGER_IND_P1_QM+5]=0x00;

	// Ahora se envia a la funcion AnulaFirma para calcular la firma del mensaje.

 
//	calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: error DIG OUT %d",i);
		return(-3);
	}

	printf("\tACTUALIZA DIG:SegIniReal=%ld \n",time(NULL));

	ConfTty[10] = msg_send;						// Buffer a Enviar
        printf("\n\tTty:WRITE:");
       	for(i=0;i<LOGER_LMSGCL_QM;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");

	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
        	for(i=0;i<rx.bytesleidos;i++)
                	printf("%02x ",rx.bufrecv[i]);}
	else{
		printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje: error DIG IN %d",i);
		return IndHisDig;
	}

	memcpy(chmsgin+0,(char *)&rx.bufrecv+LOGER_IND_MSG_QM,1);
	memcpy(chmsgin+1,(char *)&rx.bufrecv+LOGER_IND_STATUS_QM,1);


	if( (chmsgin[0] != LOGER_MSG_RESP_QM) ){
		printf("\n\tAXIS-LOGER:LOGER_MSG_DIG: Error De Protocolo LOGER_MSG_DIG %02x ",chmsgin[0]);
		return(-1);}

	if( (chmsgin[1] != LOGER_MSG_RESP_OK_QM)){		// CONTROL STATUS QM
		sprintf(aux,"DIG NO Recuperado: ErrorQm=%02x  Numero Registro=%ld",chmsgin[1],IndAct);
		AxisLog(aux);                                           // Log
		printf(" : DIG NO ACTUALIZADO :\n");
		return(-3);
	}

	//Numero Registro tabla Datalogger

        memcpy((long *)&IndHisDig,(char *)&rx.bufrecv+LOGER_IND_NUMREG_QM,4);
	IndHisDig = ntohl(IndHisDig);

        memcpy((short *)&NumInEnv,(char *)&rx.bufrecv+LOGER_IND_NUMREG_QM+5,1);

	printf("\n\tActualizaDig:IndRegInicial=%ld NumeroRegistros=%d IndUltIn=%hd",IndHisDig,NumInEnv,in.IndUltIn);

	for (i=0;i<NumInEnv;i++){

		memcpy((char *)&segjul,(char *)&rx.bufrecv+LOGER_IND_SEGJUL_QM+i*12,4);
		segjul = ntohl(segjul);
		//paso de epoca cr200 (año 1990) a Unix (año 1970)
		segjul=segjul+SEGJULCTELOGER;
		newtime=localtime(&segjul);
		auxch=asctime(newtime);

		in.SegJulIn[IndHisDig+i]=segjul;

		//señal
		movebyte(aux,rx.bufrecv+LOGER_IND_DIGINI+i*12,4);
	        memcpy(&valor,aux,4);
		valorint=(int)(valor);
		valors=(short)valorint;       
		in.NumSen[IndHisDig+i]=valors;
		//estado
		movebyte(aux,rx.bufrecv+LOGER_IND_DIGINI+4+i*12,4);
	        memcpy(&valor,aux,4);
		valorint=(int)(valor);
		valors=(short)valorint;       
		in.Estado[IndHisDig+i]=valors;

		//crear BufferIn, primero se copia la fecha
		segjul=htonl(in.SegJulIn[IndHisDig+i]-SEGJULCTESAC);
		memcpy(in.BufferIn[IndHisDig+i],(char *)&segjul,4);
		//el bit alto primer byte (numero señal -1 (¿¿FE??) estado de la señal
		valors=in.NumSen[IndHisDig+i] - 1;
		memcpy((char *)&auxchar,(char *)&valors,1);
		if ( in.Estado[IndHisDig+i] == 1){
			auxchar |= 0x80;
			printf("\n\tCrearBufferIn Estado 1\t %02x",auxchar);
		}else{ 
			printf("\n\tCrearBufferIn Estado 0\t %02x",auxchar);
		}
		//se asigna el primer byte modificado numero se señal y estado
		memcpy((char *)in.BufferIn[IndHisDig+i],(char *)&auxchar,1);

		if(DEBUG){
		printf("\n\tCrearBufferIn \t");
			for(j=0;j<4;j++)
			printf("%02x ",in.BufferIn[IndHisDig+i][j]);
		}

		printf("\n\t\tFecha Incidencia(%d): %s \t Señal: %d \t Estado: %d \n",i,auxch,in.NumSen[IndHisDig+i],in.Estado[IndHisDig+i]);

	}
	
	IndHisDig=IndHisDig+NumInEnv-1;
	in.IndUltIn=in.IndUltIn+NumInEnv-1;
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

	return IndHisDig;

}
// Actualiza Time
int ActualizaTime()
{
	short i,j,l;
	char aux[64];
        struct tm *newtime;
        char * auxch;
        unsigned static short chk_env=0,chk_recv=1;
	unsigned long SegJulAux,SegJul;
	int ndia,nmes,nano,h,m,s;

	signal(SIGUSR1,SigUsr1);                                // User Signal
	signal(SIGUSR2,SigUsr2);                                // User Signal
        signal(SIGINT,KillPro);
        signal(SIGKILL,KillPro);
        signal(SIGTERM,KillPro);
	ConfigTty();

        memset(msg_send,0,NB1);
	memset((char *)&rx,0,NBR);

	NumBytes=21;

	//envio peticion hora actual

        SegJul=time(NULL);
        newtime=localtime(&SegJul);
        auxch=asctime(newtime);

	printf("\n\t------------------------------------------------------------------------------");
	printf("\n\tACTUALIZA TIME mensaje 1: Fecha Actual: %s \n",auxch);

	memset(msg_recv,0,NBR);
	memset(msg_send,0,NBW);

        msg_send[LOGER_IND_INI_QM+0]=LOGER_P0_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+1]=LOGER_P1_QM;
        msg_send[LOGER_IND_INI_QM+2]=LOGER_P2_QM;
        msg_send[LOGER_IND_INI_QM+3]=LOGER_P3_QM;
        msg_send[LOGER_IND_INI_QM+4]=LOGER_P4_QM;			// Msg Send
        msg_send[LOGER_IND_INI_QM+5]=LOGER_P5_QM;
        msg_send[LOGER_IND_INI_QM+6]=LOGER_P6_QM;
        msg_send[LOGER_IND_INI_QM+7]=LOGER_P7_QM;
        msg_send[LOGER_IND_INI_QM+8]=LOGER_P8_QM;
        msg_send[LOGER_IND_MSG_QM]=LOGER_MSG_TM;			// Dos Bytes tipo de mensaje
        msg_send[LOGER_IND_MSG_QM+1]=LOGER_MSG_TM;
        msg_send[LOGER_IND_SECCODE_QM]=LOGER_MSG_SECCODE;		// Dos Bytes codigo de seguridad
        msg_send[LOGER_IND_SECCODE_QM+1]=LOGER_MSG_SECCODE;
	msg_send[13]=0x00;
	msg_send[14]=0x00;
	msg_send[15]=0x00;
	msg_send[16]=0x00;
	msg_send[17]=0x00;
	msg_send[18]=0x00;
	msg_send[19]=0x00;
	msg_send[20]=0x00;

	// Ahora se envia a la funcion AnulaFirma para calcular la firma del mensaje.

//	calcular firma y añadir al mensaje
	if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
		printf("Error en calculo firma: error DIG OUT %d",i);
		return(-3);
	}

	printf("\tACTUALIZA TIME:SegIniReal=%ld \n",time(NULL));

	ConfTty[10] = msg_send;							// Buffer a Enviar
        printf("\n\tTty:WRITE:");
       	for(i=0;i<LOGER_LMSGCL_QM;i++){
               	printf("%02x ",ConfTty[10][i]);}
        printf("\n");

	if(DEBUG){
		ResUtl = DebugTty(NumArg,ConfTty);	
	}else{
		ResUtl = TtyFunc(NumArg,ConfTty);
	}
	
	if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
		printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
        	for(i=0;i<rx.bytesleidos;i++)
                	printf("%02x ",rx.bufrecv[i]);}
	else{
		printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
		return(-1);}

	//comprobar firma mensaje recibido
	if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
		printf("Error en firma mensaje: error DIG IN %d",i);
		return(-3);
	}

	memcpy(chmsgin+0,(char *)rx.bufrecv+LOGER_IND_MSG_QM,1);
	memcpy(chmsgin+1,(char *)rx.bufrecv+LOGER_IND_STATUS_QM,1);

	if( (chmsgin[0] != LOGER_MSG_RESP_TM) ){
		printf("\n\tAXIS-LOGER:LOGER_MSG_TM: Error De Protocolo LOGER_MSG_DIG %02x ",chmsgin[0]);
		return(-1);}

	if( (chmsgin[1] != LOGER_MSG_RESP_OK_TM)){		// CONTROL STATUS QM
		sprintf(aux,"Actualiza TIME : ErrorTM=%02x",chmsgin[1]);
		AxisLog(aux);                                           // Log
		printf(" : Time NO ACTUALIZADO :\n");
		return(-3);
	}


//fin envio peticion hora actual leer tiempo

        memcpy((char *)&SegJul,rx.bufrecv+LOGER_IND_RESP_TM,4);
	SegJul = ntohl(SegJul);
	//printf("\n\tActualizaQm:SegJul=%ld \n",SegJul);
	//paso de epoca cr200 (año 1990) a Unix (año 1970)
	SegJul=SegJul+SEGJULCTELOGER;
	newtime=localtime(&SegJul);
	auxch=asctime(newtime);
	SegJulAux=time(NULL);
	printf("\n\tActualiza Time INI:\n\t\tCR200SegJulQm: %ld \tAxisSegJulQm: %ld \t Dif_Axis_CR200 %ld\t\tFechaCR200: %s\n",SegJul,SegJulAux,(SegJulAux - SegJul),auxch);

	// Si Axis adelantado a Datalogger o retrasado mas de 20 seg se pone hora 10 segundos adelantado
	if ( (SegJulAux > SegJul) || (SegJul > SegJulAux + 20) ){
		//ajuste reloj segun hora Axis + 10 seg
		SegJul=SegJulAux-SegJul+10;
		printf("\n\t Cambio Time INI: CR200 actualizar: %ld segundos\n",SegJul);

		//orden host a net
		SegJul=htonl(SegJul);

		//memcpy(msg_send+SAC_IND_JUL_QM,&segjulqm,4);
		memcpy(msg_send+LOGER_IND_ENVIO_TM,&SegJul,4);

		// calcular firma y añadir al mensaje
		if( (i=AnulaFirma(msg_send,NumBytes,0))<0){
			printf("Error en calculo firma: error DIG OUT %d",i);
			return(-3);
		}
	
	
		ConfTty[10] = msg_send;							// Buffer a Enviar
        	printf("\n\tTty:WRITE:");
       		for(i=0;i<LOGER_LMSGCL_QM;i++){
        	       	printf("%02x ",ConfTty[10][i]);}
        	printf("\n");
	
		if(DEBUG){
			ResUtl = DebugTty(NumArg,ConfTty);	
		}else{
			ResUtl = TtyFunc(NumArg,ConfTty);
		}
		
		if( ResUtl > 0 && rx.bytesleidos >MINBYTEMSG){				// Read Tty
			printf("\n\tTty:Read: %d Bytes: ",rx.bytesleidos);
        		for(i=0;i<rx.bytesleidos;i++)
        	        	printf("%02x ",rx.bufrecv[i]);}
		else{
			printf("\n\tAxisLoger:TtyFun: Leidos %d bytes\n",ResUtl);
			return(-1);}
	
		//comprobar firma mensaje recibido
		if( (i=AnulaFirma(rx.bufrecv,rx.bytesleidos,1)) < 0){
			printf("Error en firma mensaje: error DIG IN %d",i);
			return(-3);
		}
	
		memcpy(chmsgin+0,(char *)rx.bufrecv+LOGER_IND_MSG_QM,1);
		memcpy(chmsgin+1,(char *)rx.bufrecv+LOGER_IND_STATUS_QM,1);
	
		if( (chmsgin[0] != LOGER_MSG_RESP_TM) ){
			printf("\n\tAXIS-LOGER:LOGER_MSG_TM: Error De Protocolo LOGER_MSG_DIG %02x ",chmsgin[0]);
			return(-1);}
	
		if( (chmsgin[1] != LOGER_MSG_RESP_OK_TM)){		// CONTROL STATUS QM
			sprintf(aux,"Actualiza TIME : ErrorTM=%02x",chmsgin[1]);
			AxisLog(aux);                                           // Log
			printf(" : Time NO ACTUALIZADO :\n");
			return(-3);
		}
	
	
		//fin envio peticion hora actual leer tiempo
	
	        memcpy((char *)&SegJul,rx.bufrecv+LOGER_IND_RESP_TM,4);
		SegJul = ntohl(SegJul);
		//printf("\n\tActualizaQm:SegJul=%ld \n",SegJul);
		//paso de epoca cr200 (año 1990) a Unix (año 1970)
		SegJul=SegJul+SEGJULCTELOGER;
		newtime=localtime(&SegJul);
		auxch=asctime(newtime);
		printf("\n\t Cambio Time FIN: CR200SegJul actualizado: %ld \t Fecha: %s\n",SegJul,auxch);
	
	}else{

	printf("\n\tActualiza Time FIN: CR200SegJul actualizado: %ld \t Fecha: %s\n",SegJul,auxch);

	return(0);
	}
}

// *** chksuma  segun SAC ***
ChkSuma(men,lmen)
unsigned char *men;
int lmen;
{
        unsigned int isum;
        int i;

	isum=0;
        for(i=0;i<lmen;i+=2)
                isum+=((men[i] << 8) | men[i+1]);
        return(isum & 0xffff);
}

//crea BufferQm para envio frontend
CrearBufferQm(qm)
QM qm;
{
short NumBytes=0,i;
unsigned int chk_env=0;
//char *msg[NB1];
unsigned long SegJulAux;
short aux;

		printf("\n\tCrearBufferQm:---------------------------------------------------------------------");

		//memset(msg,0,NB1);
		memset((char *)&BdConf,0,sizeof(BDCONF));

		if(i=ReadLogerBd(BdConf)!=0){
			printf("\n\t CrearBufferQm Error lectura BdConfig:Error=%d",i);
			return(-3);}

		qm.BufferQm[LOGER_IND_INI]=LOGER_P0;			// 0x80  Control CHK, PROTO SAC 
		qm.BufferQm[LOGER_IND_INI+1]=LOGER_P1;			// 0x01
		qm.BufferQm[LOGER_IND_INI+2]=LOGER_P2;			// 0x02
		qm.BufferQm[LOGER_IND_INI+3]=LOGER_P3;			// 0x03
		qm.BufferQm[LOGER_IND_MSG]=LOGER_P4;			// 0xE7 Resp QM

		qm.BufferQm[LOGER_IND_EST]=BdConf.remconf.ihw;	// Num Hardware Remota
		SegJulAux=htonl(qm.SegJul-SEGJULCTESAC);
		memcpy(qm.BufferQm+LOGER_IND_INFO,(char *)&SegJulAux,4);
		qm.BufferQm[LOGER_IND_INFO+4]=0x02;
		memcpy(qm.BufferQm+LOGER_IND_INFO+6,(char *)&SegJulAux,4);
		qm.BufferQm[LOGER_IND_INFO+10]=LOGER_CINC1;
		qm.BufferQm[LOGER_IND_INFO+11]=LOGER_CINC2;
		qm.BufferQm[LOGER_IND_INFO+12]=LOGER_CINC1;
		qm.BufferQm[LOGER_IND_INFO+13]=LOGER_CINC2;
		qm.BufferQm[LOGER_IND_INFO+14]=LOGER_CINC1;
		qm.BufferQm[LOGER_IND_INFO+15]=LOGER_CINC2;

		aux=htons(qm.NumCont);
		memcpy(qm.BufferQm+LOGER_IND_CONT,(char *)&aux,2);		// Num Contadores
		for (i=0;i<3*qm.NumCont;i++){
		aux=htons(qm.ValorCont[i]);
		memcpy(qm.BufferQm+LOGER_IND_CONT+2+2*i,(char *)&aux,2);
		}

		aux=htons(qm.NumAna);
		memcpy(qm.BufferQm+LOGER_IND_CONT+2+6*qm.NumCont,(char *)&aux,2);	// Num Analogicas
		for (i=0;i<qm.NumAna;i++){
		aux=htons(qm.ValorAna[i]);
		memcpy(qm.BufferQm+LOGER_IND_CONT+4+6*qm.NumCont+2*i,(char *)&aux,2);
		}

		qm.BufferQm[LOGER_IND_CONT+25+6*qm.NumCont+2*qm.NumAna]=0x01;
		qm.BufferQm[LOGER_IND_CONT+27+6*qm.NumCont+2*qm.NumAna]=0x08;
		qm.BufferQm[LOGER_IND_CONT+29+6*qm.NumCont+2*qm.NumAna]=0x01;
		qm.BufferQm[LOGER_IND_CONT+31+6*qm.NumCont+2*qm.NumAna]=0x01;
		qm.BufferQm[LOGER_IND_CONT+33+6*qm.NumCont+2*qm.NumAna]=0x1D;
		qm.BufferQm[LOGER_IND_CONT+34+6*qm.NumCont+2*qm.NumAna]=0x0A;
		qm.BufferQm[LOGER_IND_CONT+35+6*qm.NumCont+2*qm.NumAna]=0x60;
		// Numero de bytes hasta el checksum
		NumBytes = LOGER_IND_CONT + 2 + 6*qm.NumCont + 2 + 2*qm.NumAna + 2 + 2 + 28;
		qm.BufferQm[LOGER_IND_WORD]=(char *)((NumBytes - 4 - LOGER_IND_MSG)/2);		// Num Word
		chk_env=ChkSuma(qm.BufferQm+LOGER_IND_MSG,NumBytes - 4 - LOGER_IND_MSG);
		chk_env=htons(chk_env);                                         // Orden Byte Host to Net
		memcpy(qm.BufferQm+NumBytes,(char *)&chk_env,2);
		qm.BufferQm[NumBytes+2]=LOGER_FIN;                               // Fin MSG A5
		qm.BufferQm[NumBytes+3]=LOGER_FIN; 
		qm.lBufferQm=NumBytes+4;
		if(DEBUG){
		printf("\n\tCrearBufferQm : %hd \n",qm.lBufferQm);
			for(i=0;i<qm.lBufferQm;i++)
			printf("%02x ",qm.BufferQm[i]);
		}
		return(0);
}

//Funcion que anula la forma del mensaje como CRC para verificar mensaje correcto.
//InOut diferencia lectura de escritura a la hora de quitar-añadir bytes de quota:
// 0=escritura		1=lectura	2=calculo firma
// Devuelve 1 si todo correcto -1 si algo falla
int AnulaFirma(unsigned char msg_send[],int NumBytes,int InOut){
int len,NumQuotas=0,i;
unsigned short u,SerSyncByte=0xBD,QuoteByte=0xBC;

//primero comprobamos presencia bytes BD o BC
int UnQuoteNext = 0,cnt=1;
unsigned char temp,msg[NBR],firma[3],firmaAux[3];

if(InOut){
	NumQuotas=0;
	msg[0]=SerSyncByte;
	for (len =1; len < NumBytes ; len++){
		temp = msg_send[len];
		if(UnQuoteNext){
			NumQuotas--;
			UnQuoteNext = 0;
			msg[cnt++] = temp - 0x20;
			if(DEBUG){printf ("\nEncontrado BC Lectura \t%02x\t%02x\t%02x\t%hd",temp, temp-0x20,msg[cnt],NumQuotas);}
		}else if (temp == 0xBC){
			UnQuoteNext = 1;
		}else{		
			msg[cnt++] = temp;
		}
	}

}else {
	NumQuotas=0;
	msg[0]=SerSyncByte;
	for (len =1; len < NumBytes ; len++){
		temp = msg_send[len];
		if(temp == 0xBD || temp == 0xBC){
			NumQuotas++;
			msg[cnt++] = 0xBC;			
			msg[cnt++] = temp + 0x20;
			if(DEBUG){printf ("\nEncontrado BC Escritura \t%02x\t%02x\t%02x\t%hd",temp, temp-0x20,msg[cnt],NumQuotas);}
		}else{		
			msg[cnt++] = temp;
		}
	}
}
//fin comprobacion

	if(DEBUG){	
	printf ("\nmsg_send\n\tnum bytes %hd\n",cnt);
	for( len = 0; len < NumBytes; len++){
		printf ("%02x",msg_send[len]);
	}	
	printf ("%02x\n",0xBD);
	}

	if(DEBUG){	
	printf ("\nmsg\n\tnum bytes %hd\n",cnt);
	for( len = 0; len < NumBytes+NumQuotas; len++){
		printf ("%02x",msg[len]);
	}	
	printf ("%02x\n",0xBD);
	}

unsigned char uc;
unsigned short sig = 0xaaaa;

if(DEBUG){printf ("%02x",0xBD);}

if(InOut){
	for( len = 1; len < NumBytes+NumQuotas-3; len++){
		sigAndSend (msg[len], &sig);
	}
}else {
	for( len = 1; len < NumBytes+NumQuotas; len++){
		if(msg[len]!=0xBC)sigAndSend (msg[len], &sig);
		else{ sigAndSend(msg[len+1]-0x20, &sig);len++;}
	}
}

uc = 0;
sig = signature(&uc, 1, sig);
uc = 0x100 - (sig & 0xff);
sigAndSend (uc, &u);
firma[0]=uc;
uc = 0x100 - (sig >>8);
sigAndSend ( uc, &u);
firma[1]=uc;
if(DEBUG){printf ("%02x\n",0xBD);}
firma[2]=0xBD;

if (InOut){

	for(i=0;i<3;i++){
               	firmaAux[i]=rx.bufrecv[rx.bytesleidos-3+i];}
	//comprobar que en firma entrada no este BC
	if(firma[0] == 0xBC) firma[0]=0xDC;
	if(firma[0] == 0xBD) firma[0]=0xDD;
	if(firma[1] == 0xBC){ firma[0]=0xBC;firma[1]=0xDC;}
	if(firma[1] == 0xBD){ firma[0]=0xBC;firma[1]=0xDD;}
	if(DEBUG){
        printf("\n\tFirma:");
       	for(i=0;i<3;i++){
               	printf("%02x ",firma[i]);}
        printf("\n");
        printf("\n\tFirmaAux:");
       	for(i=0;i<3;i++){
               	printf("%02x ",firmaAux[i]);}
        printf("\n");
	}

	//comprobar firma mensaje si no coincide salir.
	if(!strncmp(firma,firmaAux,2)){
	        if(DEBUG){printf("\n Firma OK\n");}
	//todo bien, entonces copiar mensaje sin quotas a buffer recepcion
	memmove(rx.bufrecv,msg,rx.bytesleidos-3+NumQuotas);

	if(DEBUG){
        printf("\n\t%hd \tFinal: ",strncmp(firma,firmaAux,2));
       		for(i=0;i<rx.bytesleidos-3+NumQuotas;i++){
        	       	printf("\t%02x %02x",rx.bufrecv[i],msg[i]);}
	        printf("\n");
	}
	return 1;

	}else{
	        printf("\n Firma No Ok \t%hd\n",strncmp(firma,firmaAux,2));
        	printf("\n\tFirma:");
       		for(i=0;i<3;i++){
        	       	printf("%02x ",firma[i]);}
        	printf("\n");
        	printf("\n\tFirmaAux:");
       		for(i=0;i<3;i++){
        	       	printf("%02x ",firmaAux[i]);}
	        printf("\n");
		//exit(1);
		return -1;
	}

}else {
	memmove(msg_send,msg,NumBytes+NumQuotas);
	if(DEBUG){
        printf("\n\tFirma:");
       	for(i=0;i<3;i++){
               	printf("%02x ",firma[i]);}
        printf("\n");
	}
// para el caso de que BC incluido en la propia firma
//short BCenFirma=0;
//if(firma[0]==0xBC || firma[0]==0xBD || firma[1]==0xBC || firma[1]==0xBD) BCenFirma=1;

	//comprobar que en firma salida no este BC o BD
	if(firma[0] == 0xBC){ firma[0]=0xDC; msg_send[NumBytes+NumQuotas]=0xBC; NumQuotas++;}
	if(firma[0] == 0xBD){ firma[0]=0xDD; msg_send[NumBytes+NumQuotas]=0xBC; NumQuotas++;}
	if(firma[1] == 0xBC){ msg_send[NumBytes+NumQuotas] = firma[0]; firma[0]=0xBC;firma[1]=0xDC; NumQuotas++;}
	if(firma[1] == 0xBD){ msg_send[NumBytes+NumQuotas] = firma[0]; firma[0]=0xBC;firma[1]=0xDD; NumQuotas++;}
//	copiar firma en mensaje a enviar
	for(i=0;i<3;i++){
               	msg_send[NumBytes+NumQuotas+i]=firma[i];}

//	if(BCenFirma){
		
//		return -3;
//	}

	return 1;

} 

}

// Funcion que va calculando firma mensajes
void sigAndSend(unsigned short uc, unsigned short *pSig){
unsigned short SerSyncByte=0xBD,QuoteByte=0xBC;

	*pSig = signature (&uc,1,*pSig);
	if(DEBUG){printf("\tfirmando: ");printf("%02x",uc);}
}

// Funcion de calculo de firma para cada byte
unsigned short signature(buf, swath, seed)
unsigned char *buf;
int swath;
unsigned short seed;
{
unsigned char msb,lsb;
unsigned char b;
int i;
msb = seed >>8;
lsb = seed;
	for (i=0;i<swath;i++){
		b = (lsb << 1) + msb + *buf++;
		if( lsb & 0x80 ) b++;
		msb = lsb;
		lsb = b;
	}
	return (unsigned short)((msb << 8)+lsb);
}

