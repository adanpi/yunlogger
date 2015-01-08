
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <memory.h>
#include <errno.h>

#include "cgl.h"
#include "cgi.h"
#include "logersaihbd.h"

BDCONF BdConf;
short DEBUG=DEPURAR;

int
main(int argc, char **argv)
{

	int	i,AnaIni,iMostrar;
	char	AnaFinChar[5],TipSen[24];
	

	if (cgl_initenv() == -1) {
		cgl_perror(stderr, "digitalescgi");
		exit(1);
	}


	if (strcmp(cgl_Env->request_method, "POST") == 0) {

		cgl_html_begin("Configuracion Digitales");

		cgl_put_heading(2, "Configuracion Digitales");

		if (cgl_initformdata() == -1) {
			cgl_perror(stderr, "digitalescgi");
			exit(1);
		}

	// Lectura Analogica inicial
	sscanf(cgl_getvalue("AnaIni"),"%d",&AnaIni);
	if(DEBUG){	printf("debugCGI Analogica Inicial %d\n<BR>",AnaIni);}
	// Conversion Analogica Fin a char
/*	sprintf(&AnaFinChar,"%d%",AnaIni+NUMDIGCONF);
	if(DEBUG){	printf("debugCGI Analogica Final %d %s\n<BR>",AnaIni+NUMDIGCONF,AnaFinChar);}
*/




/***********************************************************/
/* Lectura-Escritura BBDD SAC */
/***********************************************************/

	memset((char *)&BdConf,0,sizeof(BdConf));

	// para Axis por ahora es necesario fijar la variable de entorno a mano
	// en la version 0.94 de Boa se puede establecer en boa.conf
	if(CompilarAxis){
		if(setenv("SAIHBD",SAIHBD_PATH,1) !=0){
			printf(stderr,"\n\ReadLogerBd:No es posible SET Variable Entorno SAIHBD");exit(1);}
	}
	if(DEBUG){	printf("debugCGI %s\n<BR>",(char *)getenv("SAIHBD"));}

	if( (char *)getenv("SAIHBD") == NULL){
		printf(stderr,"\n\ReadLogerBd:Variable Entorno SAIHBD NO SET");fflush(stdout);exit(1);}

	if(i=ReadLogerBd(&BdConf)!=0){	/*Leer Objeto B.D  BDCONF */
		printf("\n\tNumComLoger:Error=%d",i);
		exit(1);}

	fflush(stdout);	

	// formulario conf digitales
	printf("<form action=escribirdigitales.sh method=POST>\n");

	printf("<table class=\"datos\"><tr><td></td><td>Tag</td><td>Descripcion</td><td>Etiqueta 0</td><td>Etiqueta 1</td></tr>");

	for(i=AnaIni;i<AnaIni+NUMDIGCONF;i++){
		if(DEBUG){
		printf("\n\t %s %s %s",BdConf.digconf.tag[i],BdConf.digconf.desc[i],BdConf.digconf.etiqueta0[i]);
		}
		if(i>NUMSENDIG){ break;}
		else{ strcpy(TipSen,"Digital(IN)");iMostrar=i+1;}
				
		printf("<tr>");
		printf("<td>%s %d</td>",TipSen,iMostrar);
		printf("<td><input class=\"anal\" type=text value =\"%s\" size=12 maxlen=12 name=\"TAG%d\"></td>\n",BdConf.digconf.tag[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%s\" size=32 maxlen=32 name=\"DESC%d\"></td>\n",BdConf.digconf.desc[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%s\" size=12 maxlen=12 name=\"ETI0%d\"></td>\n",BdConf.digconf.etiqueta0[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%s\" size=12 maxlen=12 name=\"ETI1%d\"></td>\n",BdConf.digconf.etiqueta1[i],i);

		printf("</tr>");
	}

	printf("</table>");
	cgl_put_hidden("AnaIni",cgl_getvalue("AnaIni"));

	printf("<br><input type=submit value=\"Enviar Configuracion\">\n");

	printf("</form>\n");



/*************** Enlace configuracion analogicas remota*********************/
		printf("<br><hr><br>");
		printf("<table><tr>");
		if (AnaIni > 0){
		sprintf(&AnaFinChar,"%d%",AnaIni-NUMDIGCONF);
		if(DEBUG){	printf("debugCGI Analogica Inicial %d\n<BR>",AnaIni);}
		printf("<td><form action=digitales.sh method=POST>\n");
		printf("<input type=submit value=\"Anteriores\">\n");
		cgl_put_hidden("AnaIni",AnaFinChar);
		printf("</form></td>\n");
		}
		sprintf(&AnaFinChar,"%d%",AnaIni+NUMDIGCONF);
		if(DEBUG){	printf("debugCGI Analogica Inicial %d\n<BR>",AnaIni);}
		printf("<td><form action=digitales.sh method=POST>\n");
		printf("<input type=submit value=\"Siguientes\">\n");
		cgl_put_hidden("AnaIni",AnaFinChar);
		printf("</form></td>\n");
		printf("</tr></table>");
		printf("<br><hr><br>");


		fflush(stdout);

		cgl_freeformdata();

		cgl_html_end();

/*********************************************************************************/

	}

	return 0;
}

/* END */
