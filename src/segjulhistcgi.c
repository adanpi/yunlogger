
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

GN gn;
short DEBUG=DEPURAR;

int
main(int argc, char **argv)
{

	int	ParNumAux;
	long	segjulqm,IndHis;
	struct tm *newtime;
	FILE *fh;
	char path[80];

	if (cgl_initenv() == -1) {
		cgl_perror(stderr, "segjulhistcgi");
		exit(1);
	}


	if (strcmp(cgl_Env->request_method, "POST") == 0) {

		cgl_html_header();

		cgl_html_begin("Modificar Ultimo QM Recibido");

		printf("<TABLE BORDER=\"0\" WIDTH=\"900px\" CELLSPACING=\"0\" CELLPADDING=\"0\">");
		printf("<TR><TD ALIGN=\"center\" VALIGN=\"center\" HEIGHT=\"91\" BGCOLOR=\"#ffd700\">");
		printf("<A HREF=\"http://www.chebro.es\"><H2>CONFEDERACION HIDROGRAFICA DEL EBRO</H2></a></TD>");
		printf("<TD VALIGN=\"CENTER\" ALIGN=\"CENTER\" WIDTH=\"50%\" HEIGHT=\"91\" BGCOLOR=\"#ffd700\">");
		printf("<A HREF=\"http://195.55.247.237/saihebro/\"><H1>SAIH</H1></A></TD>");

		printf("<TD VALIGN=\"MIDDLE\" ALIGN=\"RIGHT\" BGCOLOR=\"#d1d1d1\"> </TD></TR>");
		printf("</TABLE>");


		printf("<br><br><center>");

		cgl_put_heading(2, "Modificar Ultimo QM Recibido");

		printf("<br><br></center>");

		if (cgl_initformdata() == -1) {
			cgl_perror(stderr, "segjulhistcgi");
			exit(1);
		}


		//lectura estructura gn
		memset((char *)&gn,0,sizeof(GN));

		// para Axis por ahora es necesario fijar la variable de entorno a mano
		// en la version 0.94 de Boa se puede establecer en boa.conf
		if(CompilarAxis){
			if(setenv("SAIHBD","/mnt/flash/loger/",1) !=0){
				printf(stderr,"\n\tReadSacBd:No es posible SET Variable Entorno SAIHBD");exit(1);}
		}
		if(DEBUG){	printf("debugCGI %s\n<BR>",(char *)getenv("SAIHBD"));}
	
		if( (char *)getenv("SAIHBD") == NULL){
			printf(stderr,"\n\tReadSacBd:Variable Entorno SAIHBD NO SET");fflush(stdout);exit(1);}
		strcat(strcpy(path,(char *)getenv("SAIHBD")),("LogerGen.dat"));      // Parametros Generales
		if((fh=fopen(path,"r+b"))==NULL){
			fprintf(stderr,"\nReadSacGN:No se puede abrir:%s",path);
			exit(1);}
		if(!fread(&gn,sizeof(GN),1,fh)){
			fprintf(stderr,"\nReadSacGN:Error_En_read:%s Errn=%d",path,errno);
			fclose(fh);
			exit(1);}

		printf("<br><br><table border=\"1\">");
		printf("<tr><td>Ultimo Indice Anterior </td><td><input type=text value=\"%ld\" size=4 maxlen=4 name=\"ind\"></td></tr>",gn.IndHisAna);

		sscanf(cgl_getvalue("ind"),"%ld",&IndHis);
		gn.IndHisAna=IndHis;

		if((fseek(fh,0L,SEEK_SET))==-1){
			fprintf(stderr,"\n\tFseekSacGn:Error:%s errno=%d",path,errno);
			fclose(fh);
			return(-1);}
		if(!fwrite(&gn,sizeof(GN),1,fh)){
			fprintf(stderr,"\nWriteSacGn:No se puede crear:%s Errno=%d",path,errno);
			fclose(fh);
			return(-1);}
		if(fclose(fh)!=0) fprintf(stderr,"\n\tReadSacGN:Error en fclose:%d ",errno);


		printf("<tr><td>Ultimo Indice Final </td><td><input type=text value=\"%ld\" size=4 maxlen=4 name=\"ind\"></td></tr></table>",gn.IndHisAna);

		fflush(stdout);

		cgl_freeformdata();

		printf("<br><br><hr><a href=\"/index.html\">Volver</a>");

		cgl_html_end();

/*********************************************************************************/

	}

	return 0;
}

/* END */
