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
#include "logersaihbd.h"
#include <stdlib.h>

struct termio tnew,tsaved;
int sfd;

int table[MAX_DATA_LENGTH];
int result;
int tabindex,j;
BDCONF BdConf;
float mA=0.0F;

int main()
{


  // Leer descripcion señales
	if(j=ReadLogerBd(BdConf)!=0){				// Leer Objeto B.D  BDCONF
		printf("\n\tReadLogerBd:Error=%d",j);
		exit(1);}

  /* IP esclavo ModBus */

  sfd = set_up_tcp(NUMIPAXIS);
  /*
   * Leer en esclavo 1 NUMSENANATOT registros desde posicion POS_MB_ANAMIN_ANA1
   */

  result = leer_registros_modbus( 1, POS_MB_ANAMIN_ANA1, NUMSENANA, table, NUMSENDIG, sfd );

  if(result >= 0) /* if no comms erros */
  {
	printf("\n\t<BR><H3>Anal&oacute;gicas Entrada</H3>");
	printf("\n<div id=\"tablaDIA\">");
	printf("\n<table id=\"tablaDIA\" class=\"datos\">");
	printf("\n\t<tr class=\"cabecera\">");

	for(j=0;j<NUMSENANA;j++)
		printf("\n\t<td width=\"%d\%\"> %-12s</td>",100/NUMSENANA,BdConf.anaconf.tag[j]);

	printf("\n\t</tr>");
	printf("\n\t<tr class=\"dato\">");
		for(j=0;j<NUMSENANA;j++){
		// estimacion mA para las cuentas:
		// mA=4+(16/32768)*cuentas
		if(table[j]==3)
			mA=0;
		else
			mA=4+(16*table[j])/(32768.0);
		printf("\n\t<td width=\"%d\%\"> %d (%6.2f mA)</td>",100/NUMSENANA,table[j],mA);
		}
	printf("\n\t</tr>");
	printf("\n</table></div><BR>");

  }else
  	printf("No hay respuesta modbus\n");

  /*
   * Leer en esclavo 1 los contadores desde posicion POS_MB_CONT_NUMMIN
   */

  result = leer_registros_modbus( 1, POS_MB_CONT_NUMMIN, 1+NUMSENCONT*2, table, NUMSENDIG, sfd );

  if(result >= 0) /* if no comms erros */
  {
	printf("\n\t<BR><H3>Contadores</H3>");
	printf("\n<div id=\"tablaDIA\">");
	printf("\n<table id=\"tablaDIA\" class=\"datos\">");
	printf("\n\t<tr class=\"cabecera\">");
	printf("\n\t<td width=\"%d\%\"> Minutal (0-14) </td>",100/(1+NUMSENCONT*2));
	for(j=0;j<NUMSENCONT;j++)
		printf("\n\t<td width=\"%d\%\"> Contador %d minutal actual</td> ",100/(1+NUMSENCONT*2),j+1);
	for(j=0;j<NUMSENCONT;j++)
		printf("\n\t<td width=\"%d\%\"> Contador %d acumulado QM actual</td>",100/(1+NUMSENCONT*2),j+1);

	printf("\n\t</tr>");
	printf("\n\t<tr class=\"dato\">");
		for(j=0;j<1+NUMSENCONT*2;j++)
			printf("\n\t<td width=\"%d\%\"> %d</td>",100/(1+NUMSENCONT*2),table[j]);

	printf("\n\t</tr>");
	printf("\n</table></div><BR>");

  }else
  	printf("No hay respuesta modbus\n");

  /*
   *  Leer estados digitales desde el 0 NUMSENDIG posiciones
   */
  result = leer_estados_digitales_modbus(  1, POS_MB_DIG_IN1, NUMSENDIG, table, NUMSENDIG, sfd );


  if(result >= 0) /* if no comms erros */
  {
    //Digitales 1-16
	printf("\n\t<BR><H3>Digitales Entrada</H3>");
	printf("\n<div>");
	printf("\n<table class=\"dig\">");
	printf("\n\t<tr class=\"cabecera\">");
	for(j=0;j<NUMSENDIG/2;j++)
		printf("\n\t<td title=\"%s\"> %d</td>",BdConf.digconf.desc[j],j+1);

	printf("\n\t</tr>");
	printf("\n\t<tr class=\"dato\">");
		for(j=0;j<4;j++)
			printf("\n\t<td> </td>");
		for(j=4;j<NUMSENDIG/2;j++)

/*			if(table[j]==0)
				if(BdConf.digconf.Estado[j]==1)		//No invertida
					printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
				else
					printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
			else
				if(BdConf.digconf.Estado[j]==1)		//No invertida
					printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
				else
					printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
*/
///*
			if(table[j]==1)
				printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
			else
				printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
//*/
	printf("\n\t</tr>");

	printf("\n</table></div><BR>");

    //Digitales 16-32
	printf("\n<div>");
	printf("\n<table class=\"dig\">");
	printf("\n\t<tr class=\"cabecera\">");
	for(j=NUMSENDIG/2;j<NUMSENDIG;j++)
		printf("\n\t<td title=\"%s\"> %d</td>",BdConf.digconf.desc[j],j+1);

	printf("\n\t</tr>");
	printf("\n\t<tr class=\"dato\">");
	for(j=NUMSENDIG/2;j<NUMSENDIG;j++)

/*		if(table[j]==0)
			if(BdConf.digconf.Estado[j]==1)		//No invertida
				printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
			else
				printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
		else
			if(BdConf.digconf.Estado[j]==1)		//No invertida
				printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
			else
				printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
*/
///*
		if(table[j]==1)
			printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
		else
			printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
//*/
	printf("\n\t</tr>");

	printf("\n</table></div><BR>");

  }else
  	printf("No hay respuesta modbus\n");

  /*
   *  Leer estados digitales salida el 0 NUMSENDIG posiciones
   */
  result = leer_estados_digitales_modbus(  1, POS_MB_DIG_OUT1, NUMSENDIG, table, NUMSENDIG, sfd );


  if(result >= 0) /* if no comms erros */
  {
    //Digitales 1-16
	printf("\n\t<BR><H3>Digitales Salida</H3>");
	printf("\n<div>");
	printf("\n<table class=\"dig\">");
	printf("\n\t<tr class=\"cabecera\">");
	for(j=0;j<NUMSENDIG;j++)
		printf("\n\t<td title=\"%s\"> %d</td>",BdConf.digconf.desc[D1+j],j+1);

	printf("\n\t</tr>");
	printf("\n\t<tr class=\"dato\">");
		for(j=0;j<NUMSENDIG;j++)

/*			if(table[j]==0)
				if(BdConf.digconf.Estado[D1+j]==1)		//No invertida
					printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[D1+j],table[j]);
				else
					printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[D1+j],table[j]);
			else
				if(BdConf.digconf.Estado[D1+j]==1)		//No invertida
					printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[D1+j],table[j]);
				else
					printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[D1+j],table[j]);
*/
///*
			if(table[j]==1)
				printf("\n\t<td bgcolor=\"#FF0000\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta1[j],table[j]);
			else
				printf("\n\t<td bgcolor=\"#00FF00\" title=\"%s\"> %d</td>",BdConf.digconf.etiqueta0[j],table[j]);
//*/
	printf("\n\t</tr>");

	printf("\n</table></div><BR>");

  }else
  	printf("No hay respuesta modbus\n");

  //ioctl(sfd,TCSETA,&tsaved);

  close(sfd);

  //getchar();

  return 0;
}
