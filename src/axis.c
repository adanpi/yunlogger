// AXIS - MAIN
// Axis ine Axion
/*
*       Prometeo
*       FILE:           axis.c
*       AUTHOR:         M.Bibudis
*       DATE:           18-05-2005
*       REVISION:       1.0
*       PRODUCT:        Axis control de Procesos
*       O.S.:           LINUX - AXIS ine Axion
*       CUSTOMER:       SAIH
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include "ipcserver.h"
#include "logersaihbd.h"
#
FILE *fhf;
GN gn;
int Pidlog();
#//////i
int main(int argc, char *argv[])
{
	short i,j,ipid,linea;
	unsigned long segjulact,segjulqm;
	struct tm *newtime;
	char *auxch;
	char ch[132],bufppp[22][134];
	//para axis modificados
	//char bufping[132];
	//para axis de fabrica
	char bufping[22][132];
	char up[134],iptetra[134],inetaddr[134];
	int x,ipt[4],EstComLoger,PacketesTx,PacketesRx,flag,flaglog;
	char name[92],path[92],aux[92];
	char Tx[134],Rx[134];
	struct stat bufstat;


	strcpy(name,"axis.pid");				// PID del Proceso
	ipid=getpid();
	PidLog(name,ipid);

	segjulact=time(NULL);                                   // Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);

	sprintf(aux,"Proceso Axis Iniciado:Pid=%d %s",ipid,auxch);
	AxisLog(aux);						// Log
	// signal(-,SIG_IGN);

	flag=flaglog=0;
	
	for(;;){   
	    // Copia logs ppp y sistema a memoria permanente
	    strcpy(name,"cp -f /tmp/pppd.log /mnt/flash/loger/log/pppd.log");
	    system(name);
	    strcpy(name,"cp -f /var/log/messages /mnt/flash/loger/log/messages");
	    system(name);
	
/*	    // para Reset Profilactico Axis
	    flag++;
	    if(flag >= 120){
		flag=0;	    
		segjulact=time(NULL);                                   // Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);

		sprintf(aux,"Proceso Axis Timer: %s",auxch);
		AxisLog(aux);						// Log
		
		strcpy(name,"/sbin/reboot");
		system(name);	    
	    }
*/	    
	    flaglog++;
	    if(flaglog >= 10){
		// Copia logs ppp y sistema a memoria permanente cada hora
		strcpy(name,"cp -f /mnt/flash/loger/log/pppd.log /mnt/flash/loger/log/pppd.log.old");
		system(name);
		strcpy(name,"cp -f /mnt/flash/loger/log/messages /mnt/flash/loger/log/messages.old");
		system(name);
	    }

		for(i=0;i<22;i++){
			memset(bufppp[i],0,132);
		}
		memset(ch,0,132);
		memset(bufping,0,132);

		strcpy(path,("/tmp/pppd.log"));				// PATH pppd.log
		if(!stat(path,&bufstat)){				// Borrar si > 20000 Byte
			if(bufstat.st_size > (20000)){
				strcpy(name,"cp /tmp/pppd.log /tmp/pppd.log.old");
				system(name);
				unlink(path);
				strcpy(name,"touch /tmp/pppd.log");
				system(name);
				}
		}

		segjulact=time(NULL);                                   // Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		printf("\n\tAxis:Kalimera...%s",auxch);

		if( (char *)getenv("SAIHBD") ==NULL){
			printf("\n\tVariable Entorno SAIHBD NO SET");
			return;}

		if(i=ReadLogerGn(gn)!=0)
                        printf("\n\tReadLogerGn:Error=%d",i);
		if(gn.NumComLoger[0]!=0) 
			EstComLoger=(gn.NumComLoger[1] * 100) / gn.NumComLoger[0];
		else EstComLoger=50;
		printf("\n\tEstComLoger=%d : %d %d",EstComLoger,gn.NumComLoger[0],gn.NumComLoger[1]);

		if(EstComLoger < 20 ){
			printf("\n\tAxis:EstComLoger=%d",EstComLoger);
			strcpy(name,"$SAIHBD/bin/axisloger.sh");
			system(name);
		}
		strcpy(name,"/sbin/ifconfig ppp0 > $SAIHBD/log/ppp0.log");
		system(name);

		//ping version fabrica
		strcpy(name,"/bin/ping -c 10 10.31.237.254 > $SAIHBD/log/pingtetra.log");
		system(name);
		

	/*	//ping version R201
		strcpy(name,"/bin/ping 10.31.237.254 > $SAIHBD/log/pingtetra.log");
		system(name);
	*/
		strcpy(name,"/bin/ps > $SAIHBD/log/procesos.log");
		system(name);

		TimeWait(300);	

		sprintf(path,"%s/log/ppp0.log",(char *)getenv("SAIHBD"));
		//strcpy(path,"/mnt/flash/loger/log/ppp0.log");
		if((fhf=fopen(path,"r"))==NULL){
			printf("\n\tAxis:No se puede abrir:%s Error:%d",path,errno);
			return(-1);}
		i=0;
		while(fgets(ch,130,fhf)){
			strcpy(bufppp[i],ch);
			//printf("\n\tAxis:Buffer PPP[%d]=%s",i,bufppp[i]);
			i++;
		}
		fclose(fhf);

		ipt[0]=ipt[1]=ipt[2]=ipt[3]=0;
		if( (bufppp[0][0]=='p') && (bufppp[0][1]=='p') && (bufppp[0][3]=='0') ){		// ifconfig ppp0
			sprintf(inetaddr,"%4.4s",bufppp[1]+10);
			if(i=sscanf(bufppp[1]+10+10,"%02d%*c%03d%*c%03d%*c%03d",&ipt[0],&ipt[1],&ipt[2],&ipt[3])>7) continue;
			//sprintf(iptetra,"%8.8s",bufppp[1]+10+10);
			sprintf(up,"%2.2s",bufppp[2]+10);
			if( strncmp(up,"UP",2) || (ipt[0]!=10) || (ipt[1]!=8) ){			// ppp0 DOWN
				printf("\n\tAxis:Interface ppp0 No UP: Inicia pppd");
				strcpy(name,"$SAIHBD/bin/killpid pppd 9");
				system(name);
				TimeWait(50);	
				strcpy(name,"/$SAIHBD/bin/pppd.sh");
				system(name);
				segjulact=time(NULL);                                   // Hora Actual
				newtime=localtime(&segjulact);
				auxch=asctime(newtime);
				sprintf(aux,"Proceso pppd No UP: Iniciado: %s",auxch);
				AxisLog(aux);						// Log
				TimeWait(3000);	
				continue;
			}
			printf("\n\tAxis:IP TETRA = %s : %d.%d.%d.%d\n",up,ipt[0],ipt[1],ipt[2],ipt[3]);
		}
		else{
			strcpy(name,"$SAIHBD/bin/killpid pppd 9");
			system(name);
			TimeWait(50);	
			printf("\n\tAxis: No Interface ppp0: Inicia pppd");
			strcpy(name,"$SAIHBD/bin/pppd.sh");
			system(name);
			segjulact=time(NULL);                                   // Hora Actual
			newtime=localtime(&segjulact);
			auxch=asctime(newtime);
			sprintf(aux,"Proceso pppd :Interface ppp0: Iniciado: %s",auxch);
			AxisLog(aux);						// Log
			TimeWait(3000);	
			continue;
		}

		sprintf(path,"%s/log/pingtetra.log",(char *)getenv("SAIHBD"));			
		if((fhf=fopen(path,"r"))==NULL){
			printf("\n\tAxis:No se puede abrir:%s",path);
			return(-1);}

		//analisis pingtetra para axis de fabrica
		i=0;
		while(fgets(ch,130,fhf)){
			strcpy(bufping[i],ch);
			//printf("\n\tAxis:Buffer Ping[%d]=%s",i,bufping[i]);
			i++;
		}
		fclose(fhf);

		if(!strncmp(bufping[i-1],"10",2)){
			linea=i-1;
		}
		else if(!strncmp(bufping[i-1],"round",5)){
			linea=i-2;
		}
		else{
			printf("\n\tAxis:ping No contemplado");
			continue;
		}
		
		PacketesTx=PacketesRx=0;
		printf("\n\tAxis:Buffer Ping=%s",bufping[linea]+0);
		printf("\n\tAxis:Buffer Ping=%s",bufping[linea]+24);

		strcpy(Tx,bufping[linea]);
		strcpy(Rx,bufping[linea]+24);

		sscanf(Tx,"%02d",&PacketesTx);
		sscanf(Rx,"%02d",&PacketesRx);

		//printf("\n\tAxis -Tetra:Medias=%s",bufping[linea]);
		printf("\n\tAxis - Tetra : PacketesTr=%d PacketesRx=%d\n",PacketesTx,PacketesRx);

		if( (PacketesTx < 0) || (PacketesTx >10) || (PacketesRx < 0) || (PacketesRx >10) )
			continue;
		if(PacketesRx < 3){
			strcpy(name,"$SAIHBD/bin/killpid pppd 9");
			system(name);
			printf("\n\tAxis:Ping packetes Rx < 3 : Inicia pppd");
			TimeWait(300);	
			strcpy(name,"$SAIHBD/bin/pppd.sh");
			system(name);
			segjulact=time(NULL);                                   // Hora Actual
			newtime=localtime(&segjulact);
			auxch=asctime(newtime);
			sprintf(aux,"Interface ppp0:Ping Rx<3: Iniciado pppd: %s",auxch);
			AxisLog(aux);						// Log
			TimeWait(300);	
		}

/*		//analisis pingtetra.log para version devboard_R2_01		
		fgets(bufping,130,fhf);
			printf("\n\tAxis:Buffer Ping=%s",bufping);
		fclose(fhf);
		
		if( bufping[0] == 'N'){
			//strcpy(name,"$SAIHBD/bin/killpid pppd 9");
			sprintf(name,"%s/bin/killpid pppd 9",(char *)getenv("SAIHBD"));
			system(name);
			printf("\n\tAxis:No hay Ping : Inicia pppd");
			TimeWait(300);	
			//strcpy(name,"$SAIHBD/bin/pppd.sh");
			sprintf(name,"%s/bin/pppd.sh",(char *)getenv("SAIHBD"));
			system(name);
			segjulact=time(NULL);                                   // Hora Actual
			newtime=localtime(&segjulact);
			auxch=asctime(newtime);
			sprintf(aux,"Interface ppp0: No hay PING: Iniciado pppd: %s",auxch);
			AxisLog(aux);						// Log
			TimeWait(300);	
		}
*/		TimeWait(3000);						// TimeOut
	}
	exit(0);
}
