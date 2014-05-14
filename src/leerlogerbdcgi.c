

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
	int	i,j;
	int	ParNum;
	long	segjulqm;
	char	**ParVal,LisPar[8][32],NumPar[8]="00000000";

//	ListaParametros; 

	strcpy(LisPar[0],"Codigo");
	strcpy(LisPar[1],"Descripcion");
	strcpy(LisPar[2],"Direccion IP Servidor FTP");
	strcpy(LisPar[3],"Usuario");
	strcpy(LisPar[4],"Password");
	strcpy(LisPar[5],"Directorio Remoto");
	strcpy(LisPar[6],"Frecuencia Envio Datos");
	strcpy(LisPar[7],"Identificador Estacion");


	if (cgl_initenv() == -1) {
		cgl_perror(stderr, "leerlogerbdcgi");
		exit(1);
	}


	if (strcmp(cgl_Env->request_method, "GET") == 0) {

		//cgl_html_header();

		cgl_html_begin("Configuraci&oacute;n Gestor Comunicaciones - Datalogger");


		cgl_put_heading(3, "Configuraci&oacute;n Gestor Comunicaciones - Datalogger");


/*************** Formulario configuracion opciones generales remota*********************/
		printf("<center><form action=configuracion.sh method=POST>\n");
		printf("<h3>Configuracion parametro(s) generales:</h3><br><br>");
		printf("<select name=parametro multiple valign=top size=8>\n");
		for (j=0;j<8;j++){
			printf("    <option>%s\n",LisPar[j]);
		}
		printf("  </select>\n");
		//printf("<p>\n");
		printf("<input type=submit value=\"Consultar - Modificar Valor Actual\">\n");
		printf("</form></center>\n");

		printf("<br><center><h3>Configuracion se&ntilde;ales:</h3><br>");
/*************** Enlace configuracion analogicas remota*********************/
		printf("<form action=analogicas.sh method=POST>\n");
		printf("<input type=submit value=\"Analogicas\">\n");
		cgl_put_hidden("AnaIni","0");
		printf("</form>\n");



/*************** Enlace configuracion digitales remota*********************/
		printf("<form action=digitales.sh method=POST>\n");
		printf("<input type=submit value=\"Digitales \">\n");
		cgl_put_hidden("AnaIni","0");
		printf("</form></center>\n");


		cgl_html_end();

	} else {
		//cgl_html_header();
		cgl_html_begin("Configuracion Remota axis loger");

		cgl_put_heading(2, "Configuraci&oacute;n Gestor Comunicaciones - Datalogger");

		if(DEBUG){cgl_dumpenv(stdout);cgl_dumpform(stdout);fflush(stdout);}

		if (cgl_initformdata() == -1) {
			printf("<br>Error initFormData<BR>");
			fflush(stdout);
			cgl_perror(stderr, "leerlogerbdcgi");
			exit(1);
		}

		ParVal = cgl_getvalues(&ParNum, "parametro");
		if (ParNum == -1) {
			printf("<br>Error getValues<BR>");
			fflush(stdout);
			cgl_perror(stderr, "leerlogerbdcgi");
 			exit(1);
		}


	if(DEBUG){	for(j=0;j<ParNum;j++)	printf("<br>\n %d \t debugCGI %s\n<BR>",j,ParVal[j]);}

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

/***********************************************************/
/* Presentacion datos leidos para configurar */
/***********************************************************/

	printf("<form action=escribirConf.sh method=POST>\n");
	printf("\n\nParametro a configurar:\n<br><br> ");

	for(j=0;j<ParNum;j++){ //bucle sobre los parametros seleccionados para configurar


		printf("%s: ",ParVal[j]);

		fflush(stdout);


		if(strcmp(ParVal[j],LisPar[0]) == 0){		
			printf("<input type=text value =\"%s\" size=12 maxlen=12 name=\"%s\">\n",BdConf.remconf.name,LisPar[0]);
			NumPar[0]='1';
		}
		if(strcmp(ParVal[j],LisPar[1]) == 0){		
			printf("<input type=text value =\"%s\" size=32 maxlen=32 name=\"%s\">\n",BdConf.remconf.desc,LisPar[1]);
			NumPar[1]='1';
		}

		if(strcmp(ParVal[j],LisPar[2]) == 0){		
			printf("<input type=text value =\"%s\" size=16 maxlen=16 name=\"%s\">\n",BdConf.remconf.ipnameFTP,LisPar[2]);
			NumPar[2]='1';
		}
		if(strcmp(ParVal[j],LisPar[3]) == 0){		
			printf("<input type=text value =\"%s\" size=16 maxlen=16 name=\"%s\">\n",BdConf.remconf.usuario,LisPar[3]);
			NumPar[3]='1';
		}
		if(strcmp(ParVal[j],LisPar[4]) == 0){		
			printf("<input type=text value =\"%s\" size=16 maxlen=16 name=\"%s\">\n",BdConf.remconf.contrasenia,LisPar[4]);
			NumPar[4]='1';
		}
		if(strcmp(ParVal[j],LisPar[5]) == 0){		
			printf("<input type=text value =\"%s\" size=16 maxlen=16 name=\"%s\">\n",BdConf.remconf.directorio,LisPar[5]);
			NumPar[5]='1';
		}

		if(strcmp(ParVal[j],LisPar[6]) == 0){		
			printf("<input type=text value =\"%ld\" size=3 maxlen=3 name=\"%s\">\n",BdConf.remconf.frecuencia,LisPar[6]);
			NumPar[6]='1';
		}
		if(strcmp(ParVal[j],LisPar[7]) == 0){		
			printf("<input type=text value =\"%ld\" size=3 maxlen=3 name=\"%s\">\n",BdConf.remconf.ihw,LisPar[7]);
			NumPar[7]='1';
		}

		printf("\n<br><br>");

		fflush(stdout);

	} //fin bucle for sobre parametros seleccionados

		printf("\n<br><br><input type=submit value=\"Enviar Nueva Configuracion\">\n");
	
		if(DEBUG){	printf("debugCGI %s\n<BR>",NumPar);}

		fflush(stdout);

		cgl_put_hidden("NumPar", NumPar);

		printf("</form>\n");

		cgl_freeformdata();

		cgl_html_end();

/*********************************************************************************/

	}

	return 0;
}

/* END */
