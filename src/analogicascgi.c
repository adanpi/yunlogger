
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
		cgl_perror(stderr, "analogicascgi");
		exit(1);
	}


	if (strcmp(cgl_Env->request_method, "POST") == 0) {


		cgl_html_begin("Configuracion Analogicas");

		cgl_put_heading(2, "Configuracion Analogicas");

		if (cgl_initformdata() == -1) {
			cgl_perror(stderr, "analogicascgi");
			exit(1);
		}

	// Lectura Analogica inicial
	sscanf(cgl_getvalue("AnaIni"),"%d",&AnaIni);
	if(DEBUG){	printf("debugCGI Analogica Inicial %d\n<BR>",AnaIni);}
	// Conversion Analogica Fin a char
/*	sprintf(&AnaFinChar,"%d%",AnaIni+NUMANACONF);
	if(DEBUG){	printf("debugCGI Analogica Final %d %s\n<BR>",AnaIni+NUMANACONF,AnaFinChar);}
*/




/***********************************************************/
/* Lectura-Escritura BBDD SAC */
/***********************************************************/

	memset((char *)&BdConf,0,sizeof(BdConf));

	// para Axis por ahora es necesario fijar la variable de entorno a mano
	// en la version 0.94 de Boa se puede establecer en boa.conf
	if(CompilarAxis){
		if(setenv("SAIHBD",SAIHBD_PATH,1) !=0){
			printf(stderr,"\n\tReadLogerBd:No es posible SET Variable Entorno SAIHBD");exit(1);}
	}
	if(DEBUG){	printf("debugCGI %s\n<BR>",(char *)getenv("SAIHBD"));}

	if( (char *)getenv("SAIHBD") == NULL){
		printf(stderr,"\n\tReadLogerBd:Variable Entorno SAIHBD NO SET");fflush(stdout);exit(1);}

	if(i=ReadLogerBd(&BdConf)!=0){	/*Leer Objeto B.D  BDCONF */
		printf("\n\tReadLogerBd:Error=%d",i);
		exit(1);}

	fflush(stdout);	

	// formulario conf analogicas
	printf("<form action=escribiranalogicas.sh method=POST>\n");

	printf("<table class=\"datos\"><tr><td></td><td>Tag</td><td>Descripcion</td><td>Unidades</td></tr>");

	for(i=AnaIni;i<AnaIni+NUMANACONF;i++){
		if(DEBUG){
		printf("\n\t%-12s: %-30s Unidades=%-7s \t\tFCM=%f  FCA=%f\n<br>",BdConf.anaconf.tag[i],BdConf.anaconf.desc[i],BdConf.anaconf.uni[i],BdConf.anaconf.fcm[i],BdConf.anaconf.fca[i]);
		}

	//presentacion en campos de texto

	// segun el numero de la senial analogica se diferencia el tipo:
	// 0-15: analogicas
	// 16-43: grays
	// 44-59: RS232
	// 60-63: contadores

	if(i<C1){ strcpy(TipSen,"Anal&oacute;gica(IN)");iMostrar=i+1;}
					else {break;}

		printf("<tr>");
		printf("<td>%s %d</td>",TipSen,iMostrar);

		printf("<td><input class=\"anal\" type=text value =\"%s\" size=12 maxlen=12 name=\"TAG%d\"></td>\n",BdConf.anaconf.tag[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%s\" size=35 maxlen=35 name=\"DESC%d\"></td>\n",BdConf.anaconf.desc[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%s\" size=6 maxlen=6 name=\"UNI%d\"></td>\n",BdConf.anaconf.uni[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%f\" size=9 maxlen=9 name=\"FCM%d\"></td>\n",BdConf.anaconf.fcm[i],i);
		printf("<td><input class=\"anal\" type=text value =\"%f\" size=9 maxlen=9 name=\"FCA%d\"></td>\n",BdConf.anaconf.fca[i],i);

		printf("</tr>");

		iMostrar++;

	}
	printf("</table>");

	cgl_put_hidden("AnaIni",cgl_getvalue("AnaIni"));

	printf("<br><input type=submit value=\"Enviar Configuracion\">\n");

	printf("</form>\n");



/*************** Enlace configuracion analogicas remota*********************/
		printf("<br><hr><br>");
		printf("<table><tr>");
		if (AnaIni > 0){
		sprintf(&AnaFinChar,"%d%",AnaIni-NUMANACONF);
		if(DEBUG){	printf("debugCGI Analogica Inicial %d\n<BR>",AnaIni);}
		printf("<td><form action=analogicas.sh method=POST>\n");
		printf("<input type=submit value=\"Anteriores\">\n");
		cgl_put_hidden("AnaIni",AnaFinChar);
		printf("</form></td>\n");
		}
		sprintf(&AnaFinChar,"%d%",AnaIni+NUMANACONF);
		if(DEBUG){	printf("debugCGI Analogica Inicial %d\n<BR>",AnaIni);}
		printf("<td><form action=analogicas.sh method=POST>\n");
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
