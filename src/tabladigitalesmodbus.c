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
#include "axisbd.h"
#include <stdlib.h>

struct termio tnew,tsaved;
int sfd;

int table[MAX_DATA_LENGTH];
int result;
int tabindex,j;
BDCONF BdConf;

int main()
{


  // Leer descripcion señales
	if(j=ReadAxisBd(&BdConf)!=0){				// Leer Objeto B.D  BDCONF
		printf("\n\tReadAxisBd:Error=%d",j);
		exit(1);}

  /* IP esclavo ModBus */

  sfd = set_up_tcp(NUMIPAXIS);


  /*
   *  Leer estados digitales desde el 0 NUMSENDIGIN posiciones
   */
  result = leer_estados_digitales_modbus(  1, POS_MB_DIG_IN1, NUMSENDIGIN, table, NUMSENDIGIN, sfd );


  if(result >= 0) /* if no comms erros */
  {
		printf("\n\t<BR><H3>Digitales Entrada</H3>");

		// Tabla
		printf("\n<a href=\"#nowhere\" onclick=\"Switch('tablaDIA'),Desplegar('aDIA')\" style=\"text-align: left; \"><div id=\"aMIN\">[-] Digitales Entrada</div></a>");
		printf("\n<div id=\"tablaDIA\" style=\"overflow:auto; height:450px;\">");
		printf("\n<table id=\"tablaDIA\" class=\"dig\">");


		//printf("\n<table class=\"dig\">");
		for(j=4;j<NUMSENDIGIN;j++){
			printf("\n\t<tr class=\"cabecera\">");
			printf("\n\t<td> %s</td>",BdConf.digconf.desc[j]);
			if(table[j]==0)
				if(BdConf.digconf.Estado[j]==1)		//No invertida
					printf("\n\t<td bgcolor=\"#FF0000\"> %s </td> <td bgcolor=\"#FF0000\">%d</td>",BdConf.digconf.etiqueta0[j],table[j]);
				else
					printf("\n\t<td bgcolor=\"#00FF00\"> %s </td> <td bgcolor=\"#00FF00\">%d</td>",BdConf.digconf.etiqueta0[j],table[j]);
			else
				if(BdConf.digconf.Estado[j]==1)		//No invertida
					printf("\n\t<td bgcolor=\"#00FF00\"> %s </td> <td bgcolor=\"#00FF00\">%d</td>",BdConf.digconf.etiqueta1[j],table[j]);
				else
					printf("\n\t<td bgcolor=\"#FF0000\"> %s </td> <td bgcolor=\"#FF0000\">%d</td>",BdConf.digconf.etiqueta1[j],table[j]);

			printf("\n\t</tr>");
		}

		printf("\n</table></div><BR>");

  }else
  	printf("No hay respuesta modbus\n");

  /*
   *  Leer estados digitales salida el 0 NUMSENDIGOUT posiciones
   */
  result = leer_estados_digitales_modbus(  1, POS_MB_DIG_OUT1, NUMSENDIGOUT, table, NUMSENDIGOUT, sfd );


  if(result >= 0) /* if no comms erros */
  {
    //Digitales 1-16
	printf("\n\t<BR><H3>Digitales Salida</H3>");
	// Tabla
	printf("\n<a href=\"#nowhere\" onclick=\"Switch('tablaDO'),Desplegar('aDO')\" style=\"text-align: left; \"><div id=\"aMIN\">[-] Digitales Salida</div></a>");
	printf("\n<div id=\"tablaDO\" style=\"overflow:auto; height:450px;\">");
	printf("\n<table id=\"tablaDO\" class=\"dig\">");
	for(j=0;j<NUMSENDIGOUT;j++){
		printf("\n\t<tr class=\"cabecera\">");
		printf("\n\t<td> %s</td>",BdConf.digconf.desc[DOut1+j]);
		if(table[j]==0)
			if(BdConf.digconf.Estado[DOut1+j]==1)		//No invertida
				printf("\n\t<td bgcolor=\"#FF0000\"> %s </td> <td bgcolor=\"#FF0000\">%d</td>",BdConf.digconf.etiqueta0[DOut1+j],table[j]);
			else
				printf("\n\t<td bgcolor=\"#00FF00\"> %s </td> <td bgcolor=\"#00FF00\">%d</td>",BdConf.digconf.etiqueta0[DOut1+j],table[j]);
		else
			if(BdConf.digconf.Estado[DOut1+j]==1)		//No invertida
				printf("\n\t<td bgcolor=\"#00FF00\"> %s </td> <td bgcolor=\"#00FF00\">%d</td>",BdConf.digconf.etiqueta1[DOut1+j],table[j]);
			else
				printf("\n\t<td bgcolor=\"#FF0000\"> %s </td> <td bgcolor=\"#FF0000\">%d</td>",BdConf.digconf.etiqueta1[DOut1+j],table[j]);

		printf("\n\t</tr>");
	}

	printf("\n</table></div><BR>");

  }else
  	printf("No hay respuesta modbus\n");

  //ioctl(sfd,TCSETA,&tsaved);

  close(sfd);

  //getchar();

  return 0;
}
