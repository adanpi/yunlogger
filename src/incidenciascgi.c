
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
IN in;
short DEBUG=DEPURAR;

int
main(int argc, char **argv)
{

	int	ParNumAux,i,j,numsen,numincid;
	long	segjulini,segjulact;
	struct	tm *newtime;
	FILE	*fh;
	char	path[80],LisPer[4][3],*auxch,EtiAux[12];

	strcpy(LisPer[0],"10");
	strcpy(LisPer[1],"25");
	strcpy(LisPer[2],"50");
	strcpy(LisPer[3],"100");

	if (cgl_initenv() == -1) {
		cgl_perror(stderr, "digitalescgi");
		exit(1);
	}


	if (strcmp(cgl_Env->request_method, "GET") == 0) {



		printf("<h3>Supervisi&oacute;n Incidencias</h3>");
		printf("<br>");


/*************** Formulario numero incidencias a solicitar*********************/

		printf("<div id=\"bloque\">");

		printf("<form action=mostrarincid.sh method=POST>\n");
		printf("<TABLE class=\"cent\">");
		printf("<TR><TD>N&uacute;mero Incidencias</td>");
		printf("<td><select name=num>\n");
		for (j=0;j<4;j++){
			printf("    <option>%s\n",LisPer[j]);
		}
		printf("  </select>\n");
		printf("\n");
/*
		printf("<TD ALIGN=\"center\" VALIGN=\"center\" HEIGHT=\"91\">");
*/

		printf("     N&uacute;mero Se&ntilde;al</td>");
		printf("<td><select name=sen>\n");
		printf("    <option>Todas\n");
		for (j=1;j<81;j++){
			printf("    <option>Sen %d\n",j);
		}
		printf("  </select>\n");
		printf("</td></tr>\n");
/*
		printf("<TD ALIGN=\"center\" VALIGN=\"center\" HEIGHT=\"91\">");
*/
		printf("<tr><td></td><td>");
		printf("<input type=hidden name=\"flag\" value=\"noreset\">\n");
		printf("<br><input class=\"botonc\" type=submit value=\"Mostrar Incidencias\">\n");
		printf("</td></tr>\n");

		printf("</TABLE></form>");

		printf("<br>");
/*
		printf("<TABLE BORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"0\">");
		printf("<TR><TD ALIGN=\"center\" VALIGN=\"center\" HEIGHT=\"91\">");
*/

		printf("<h3>Forzar Recuperaci&oacute;n Incidencias</h3>");
		//printf("<h5>(Opci&oacute;n no disponible temporalmente)</h5>");
		printf("<br>");

		printf("<form action=mostrarincid.sh method=POST>\n");

		printf("<input type=hidden name=\"flag\" value=\"reset\">\n");
		printf("<input class=\"botonc\" type=submit value=\"Resetear Flag Incidencias\"><br><br>\n");


		printf("</form>");

		printf("</div>");

/*		printf("<br><hr><a href=\"/index.html\">Volver</a><br>");


		cgl_html_end();
*/

	}
	else {




		printf("<h3>Supervisi&oacute;n Incidencias</h3>");
		printf("<br>");

		if (cgl_initformdata() == -1) {
			cgl_perror(stderr, "incidenciascgi");
			exit(1);
		}

		// para Axis por ahora es necesario fijar la variable de entorno a mano
		// en la version 0.94 de Boa se puede establecer en boa.conf
		if(CompilarAxis){
			if(setenv("SAIHBD","/radsys/",1) !=0){
				printf("\n\tReadLogerBd:No es posible SET Variable Entorno SAIHBD");exit(1);}
		}
		if(DEBUG){	printf("debugCGI %s\n<BR>",(char *)getenv("SAIHBD"));fflush(stdout);}

		if( (char *)getenv("SAIHBD") == NULL){
			printf(stderr,"\n\tReadLogerBd:Variable Entorno SAIHBD NO SET");fflush(stdout);exit(1);}

		//si se pide resetear el flag incidencias:
		if(DEBUG){printf("<br>\ndebugCGI %s\n<BR>",cgl_getvalue("flag"));fflush(stdout);}
		if(strcmp(cgl_getvalue("flag"),"reset")==0){
			if(i=ReadLogerIn(in)!=0){			// Leer Objeto IN de Incidencias
				printf("\n\tReadLogerIn:Error=%d",i);
				fflush(stdout);
				exit(1);}

		if(DEBUG){printf("<br>\ndebugCGI FlagIn=%hd IndAct=%hd IndUltIn=%hd\n<BR>",in.FlagIn,in.IndAct,in.IndUltIn);fflush(stdout);}
			in.FlagIn=1;
			if(i=WriteLogerIn(in)!=0){		//Escribir Objeto B.D  BDCONF
				printf("\n\tReadAxisBd:Error=%d",i);
				fflush(stdout);
				exit(1);}
		if(DEBUG){printf("<br>\ndebugCGI FlagIn=%hd IndAct=%hd IndUltIn=%hd\n<BR>",in.FlagIn,in.IndAct,in.IndUltIn);fflush(stdout);}
		cgl_put_heading(3, "..............................Reseteado Flag Recuperacion Incidencias");
		} else {

		if(DEBUG){printf("<br>\ndebugCGI %s\n<BR>",cgl_getvalue("flag"));fflush(stdout);}

		//parsear numero senial
		sscanf(cgl_getvalue("num"),"%d",&numincid);
		sscanf(cgl_getvalue("sen"),"%3s %d",&auxch,&numsen);
		if(DEBUG){printf("<br>\ndebugCGI %d,%d\n<BR>",numincid,numsen);fflush(stdout);}

		//lectura incidencias
		segjulact=time(NULL);
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		segjulini = 10 * SEGPDIA;
		segjulact = segjulact - segjulini;

		if(DEBUG){printf("<br>\ndebugCGI %d\n<BR>",segjulact);fflush(stdout);}


		if(i=ReadLogerIn(in)!=0){				// Leer Objeto IN de Incidencias
			printf("\n\tReadAxisIn:Error=%d",i);
			fflush(stdout);
			exit(1);}

		if(i=ReadLogerBd(&BdConf)!=0){			//Leer Objeto B.D  BDCONF
			printf("\n\tReadAxisBd:Error=%d",i);
			exit(1);}

		printf("\n<table class=\"datos\">");
		printf("\n<tr class=\"cabecera\"><td>Fecha</td><td>Incid Axis</td><td>Se&ntilde;al</td><td>Tag</td><td>Desc</td><td>Estado</td><td>Etiqueta</td></tr>");

		j=1;
		int k=0,l;

		//lectura incidencias desde la mas antigua
		//for(i=0;i<VALMAXIND;i++){

		//lectura incidencias desde la mas reciente
		for(i=in.IndUltIn;i>=in.IndUltIn-VALMAXIND;i--){
			k++;
			if(k>VALMAXIND) break;		//en caso de que no haya ninguna incidencia de la señal
			if(j>numincid) break;		//no se devuelven mas incidencias de las solicitadas
			if(i<=0) i=3999;
			if( in.SegJulIn[i] < segjulact ) continue;
			unsigned long segjulin = in.SegJulIn[i];
			newtime=localtime(&segjulin);
			auxch=asctime(newtime);
			if(strcmp(cgl_getvalue("sen"),"Todas")!=0){
			if(in.NumSen[i] != numsen) continue;
			}
			if(DEBUG){printf("\n\t%d-INCID SAC:[%d] NumSen=%hd Estado=%hd SegJulIn[%d]=%ld FECHA:%s<br>",j,i,in.NumSen[i],in.Estado[i],i,in.SegJulIn[i],auxch);}

			//segun estado se imprime la etiqueta correspondiente
			if(in.Estado[i]==0){strcpy(EtiAux,BdConf.digconf.etiqueta0[in.NumSen[i]-1]);}
			else {strcpy(EtiAux,BdConf.digconf.etiqueta1[in.NumSen[i]-1]);}

			printf("\n<tr class=\"dato\"><td>%s</td><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%hd</td><td class=\"alt\">%s</td></tr>",auxch,i,in.NumSen[i],BdConf.digconf.tag[in.NumSen[i]-1],BdConf.digconf.desc[in.NumSen[i]-1],in.Estado[i],EtiAux);

			printf("\n\tBufferIn \t");
			for(l=0;l<4;l++) printf("%02x ",in.BufferIn[i][l]);

			j++;
		}

		printf("</table>");
		printf("\n\t<br><h5>IndAct: %hd (0x%0x) | IndUltIn: %hd (0x%0x) | NumInAlm: %hd (0x%0x) | ValMaxInd: %hd (0x%0x)</h5>",
		in.IndAct,in.IndAct,in.IndUltIn,in.IndUltIn,in.NumInAlm,in.NumInAlm,in.ValMaxInd,in.ValMaxInd);

		}

		fflush(stdout);

		cgl_freeformdata();

//		printf("<br><br><hr><a href=\"/index.html\">Volver</a>");

//		cgl_html_end();

/*********************************************************************************/

	}

	return 0;
}

/* END */
