#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "srvipc.h"
#include "srvipc_shm.h"
int clnt_actualiza_bd();
CLNTBD_SEND clntbd_send;
FILE *fh;
/*char aux_send[20]={0x0,0x9,0x1,0x1,0x1,0x55,0x0,0x52,0x0,0x0,0x0,0x0,0x0};*/
char aux_send[20]={0xa,0x9,0x1,0x1,0x1,0x55,0x0,0x52,0x0,0x0,0x0,0x0,0x0};

main(argc, argv)
int argc;
char *argv[];
{
	static int cont;
	long sjul,ltime,sjul_qm;
	int i,j,k,x;
	char path[64];
	int ret,num;
	long segjul;
	short ciclico,origen,destino,ihw;


	segjul=700700700;
	ciclico=28245;
	origen=44;
	destino=45;
	ihw=355;
	/*memset((char *)&aux_send,0,sizeof(aux_send));
	memset((char *)&rem_bd,0,sizeof(REM_BD));*/

	if (argc > 1) exit(2);

	for(;;){
	    ltime=segtojul(NULL);
	    *(long*)&aux_send[8] = ltime;
	    for(i=0; i < 12;i++){
		aux_send[12] = aux_send[12]+aux_send[i];}
		/*printf(" %x,", aux_send[i]);}
		printf(" %x,", aux_send[12]);***/

	    if((ret=msg_datquin(ciclico,origen,destino,ihw,segjul,"192.168.1.80",CNCL))<=0){
		fprintf(stderr,"\n\tCLNTTETRA:Error_Con_Axis = %d Fecha=%d",ret,ltime);
		}
	    fprintf(stderr,"\n\tTIME_TINI:ltime=%ld\n\n",ltime); 
	    ciclico++;
	    sleep(900);
	}
}
