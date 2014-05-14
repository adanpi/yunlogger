

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
	int	i;
	int	NumPar;
	char	*valor,*param[8],LisPar[8][32],Param[8]="00000000";

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
		cgl_perror(stderr, "escribirlogerbdcgi");
		exit(1);
	}


	if (strcmp(cgl_Env->request_method, "POST") == 0) {

		cgl_html_header();

		cgl_html_begin("Configuraci&oacute;n Gestor Comunicaciones - Datalogger");


		cgl_put_heading(2, "Configuraci&oacute;n Gestor Comunicaciones - Datalogger");

		if (cgl_initformdata() == -1) {
			cgl_perror(stderr, "escribirlogerbdcgi");
			exit(1);
		}

		strcpy(Param,cgl_getvalue("NumPar"));	

		if(DEBUG){	printf("debugCGI Parametros %s\n<BR>",Param);}	

/***********************************************************/
/* Lectura-Escritura BBDD SAC */
/***********************************************************/

	short ihw,iRead,frecuencia;

	memset((char *)&BdConf,0,sizeof(BdConf));

	if(CompilarAxis){
		if(setenv("SAIHBD",SAIHBD_PATH,1) !=0){
			printf(stderr,"\n\tReadLogerBd:No es posible SET Variable Entorno SAIHBD");exit(1);}
	}
	if(DEBUG){	printf("debugCGI %s\n<BR>",(char *)getenv("SAIHBD"));}

	if( (char *)getenv("SAIHBD") ==NULL){
		printf(stderr,"\n\tReadLogerBd:Variable Entorno SAIHBD NO SET");}

	if(iRead=ReadLogerBd(&BdConf)!=0){	/*Leer Objeto B.D  BDCONF */
		printf("\n\tReadLogerBd:Error=%d",iRead);
		exit(1);}

	fflush(stdout);	

/***********************************************************/
/* Presentacion datos leidos para configurar */
/***********************************************************/

		if(Param[0]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %s",LisPar[0],BdConf.remconf.name);
			printf("<BR><BR>");
			valor=cgl_getvalue(LisPar[0]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[0],valor);
			if(DEBUG){	printf("debugCGI Param0 %s %s\n<BR>",Param,valor);fflush(stdout);}
			for(i=0;i<12;i++)
				BdConf.remconf.name[i]=valor[i];

			fflush(stdout);		
		}
		if(Param[1]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %s",LisPar[1],BdConf.remconf.desc);
			printf("<BR><BR>");
			valor=cgl_getvalue(LisPar[1]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[1],valor);
			if(DEBUG){	printf("debugCGI Param1 %s %s\n<BR>",Param,valor);fflush(stdout);}
			for(i=0;i<32;i++)
				BdConf.remconf.desc[i]=valor[i];

			fflush(stdout);		
		}
		if(Param[2]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %s",LisPar[2],BdConf.remconf.ipnameFTP);
			printf("<BR><BR>");
			valor=cgl_getvalue(LisPar[2]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[2],valor);
			if(DEBUG){	printf("debugCGI Param2 %s %s\n<BR>",Param,valor);fflush(stdout);}
			for(i=0;i<16;i++)
				BdConf.remconf.ipnameFTP[i]=valor[i];

			fflush(stdout);		
		}
		if(Param[3]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %s",LisPar[3],BdConf.remconf.usuario);
			printf("<BR><BR>");
			valor=cgl_getvalue(LisPar[3]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[3],valor);
			if(DEBUG){	printf("debugCGI Param3 %s %s\n<BR>",Param,valor);fflush(stdout);}
			for(i=0;i<16;i++)
				BdConf.remconf.usuario[i]=valor[i];

			fflush(stdout);		
		}

		if(Param[4]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %s",LisPar[4],BdConf.remconf.contrasenia);
			printf("<BR><BR>");
			valor=cgl_getvalue(LisPar[4]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[4],valor);
			if(DEBUG){	printf("debugCGI Param3 %s %s\n<BR>",Param,valor);fflush(stdout);}
			for(i=0;i<16;i++)
				BdConf.remconf.contrasenia[i]=valor[i];

			fflush(stdout);		
		}
		if(Param[5]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %s",LisPar[5],BdConf.remconf.directorio);
			printf("<BR><BR>");
			valor=cgl_getvalue(LisPar[5]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[5],valor);
			if(DEBUG){	printf("debugCGI Param3 %s %s\n<BR>",Param,valor);fflush(stdout);}
			for(i=0;i<16;i++)
				BdConf.remconf.directorio[i]=valor[i];

			fflush(stdout);		
		}
		if(Param[6]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %ld",LisPar[6],BdConf.remconf.frecuencia);
			printf("<BR><BR>");
			fflush(stdout);
			valor=cgl_getvalue(LisPar[6]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[6],valor);
			fflush(stdout);
			sscanf(valor,"%hd",&frecuencia);
			if(DEBUG){	printf("<br>debugCGI Param4 %s %hd\n<BR>",Param,frecuencia);fflush(stdout);}

				BdConf.remconf.frecuencia=frecuencia;

			fflush(stdout);		
		}
		if(Param[7]=='1'){

			printf("<BR><BR>");
			printf("\n\n\tValor Anterior %s: %ld",LisPar[7],BdConf.remconf.ihw);
			printf("<BR><BR>");
			fflush(stdout);
			valor=cgl_getvalue(LisPar[7]);
			printf("\n\n\tValor Nuevo %s: %s",LisPar[7],valor);
			fflush(stdout);
			sscanf(valor,"%hd",&ihw);
			if(DEBUG){	printf("<br>debugCGI Param4 %s %hd\n<BR>",Param,ihw);fflush(stdout);}

				BdConf.remconf.ihw=ihw;

			fflush(stdout);		
		}




			if(iRead=WriteLogerBd(BdConf)!=0){	/*Escribir Objeto B.D  BDCONF*/
				printf("\n\tWriteLogerBd:Error=%d",iRead);
				exit(1);}



		fflush(stdout);

		cgl_freeformdata();

		cgl_html_end();

/*********************************************************************************/

	}

	return 0;
}

/* END */
