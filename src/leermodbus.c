/*
 ============================================================================
 Name        : modbus.c
 Author      : Adan
 Version     :
 Copyright   : IData Sistemas
 Description : leermodbus para datos en web
 ============================================================================
 */

#include <stdio.h>	/* Standard input/output */
#include <termio.h>	/* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include "modbus_tcp.h"
#include "pantalla.h"
#include <stdlib.h>

#define IP "10.10.10.239"

struct termio tnew,tsaved;
int sfd;

int table[MAX_DATA_LENGTH];
int result;
int tabindex,j;
float mA=0.0F;
char * ip;
static char *version = "1.0 (28/02/2015)";
int esclavo,port,reg_ini,numreg,rtu=0,debug=0;
main(argc,argv)
char *argv[];
int argc;
{

        if ( (argc==1) || ((argc>2) && (argc<8)) || ((argc==2) && (argv[1][1]=='v'))){
                printf("\n************************************");
                printf("\n\t radsys.es");
                printf("\n\t leermodbus Version: %s",version);
                printf("\n\t uso: leermodbus -v (version y uso)");
                printf("\n\t uso: leermodbus host port id_esclavo reg_ini numregs rtu debug(leer registros modbus)");

                printf("\n************************************\n");
                return(0);
        }

  /* IP esclavo ModBus */

  //sscanf(argv[2],"%s",ip);
if(sscanf(argv[2],"%d",&port)!=1){
	 printf("\n\t Error en port");
	return(-1);
}
if(sscanf(argv[3],"%d",&esclavo)!=1){
	 printf("\n\t Error en esclavo");
	return(-1);
}
if(sscanf(argv[4],"%d",&reg_ini)!=1){
	 printf("\n\t Error en reg_ini");
	return(-1);
}
if(sscanf(argv[5],"%d",&numreg)!=1){
	 printf("\n\t Error en numreg");
	return(-1);
}
if(sscanf(argv[6],"%d",&rtu)!=1){
	 printf("\n\t Error en rtu");
	return(-1);
}
if(sscanf(argv[7],"%d",&debug)!=1){
	 printf("\n\t Error en debug");
	return(-1);
}

	if(debug>0){
		printf("\n\t host [%s:%d] esclavo [%d] reg_ini [%d] numreg [%d] rtu [%d]",argv[1],port,esclavo,reg_ini,numreg,rtu);
		if(numreg>0)
			printf("\n\t\t lectura registros funcion 3");
		else{
			// si se pasa valor negativo de numero de registros es el valor a escribir
			printf("\n\t\t escritura registro funcion 6, valor: %d",0-numreg);	
		}
	}

 sfd = set_up_tcp_port(argv[1],port);

if(numreg>0){
  
  /*
   * Leer en esclavo 1 NUMSENANATOT registros desde posicion POS_MB_ANAMIN_ANA1
   */

  result = leer_registros_modbus( esclavo, reg_ini, numreg, table, numreg, sfd ,rtu);

  if(result > 0) /* if no comms erros */
  {
	if(debug>0)
		printf("\nLeermodbus\n");

		for(j=0;j<numreg;j++){
			printf("%d\n",table[j]);
		}
	if(debug>0)
		printf("\nFin Leermodbus\n");
  }else
  	printf("No hay respuesta modbus\n");

}else{
	result = escribir_registro_modbus( esclavo, reg_ini, 0-numreg, sfd ,rtu);
  if(result > 0) /* if no comms erros */
  {
	if(debug>0)
		printf("\nLeermodbus->%d \n",result);

	if(debug>0)
		printf("\nFin Leermodbus\n");
  }else
  	printf("No hay respuesta modbus\n");
}
  //ioctl(sfd,TCSETA,&tsaved);

  close(sfd);

  //getchar();

  return result;
}
