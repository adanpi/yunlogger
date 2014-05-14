#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
//#include <math.h>

#include <memory.h>
#include <errno.h>

//#include "cgl.h"
//#include "cgi.h"
#include "logersaihbd.h"
#define NUMEY 5		//numero de etiquetas Y

BDCONF BdConf;
QM qm[NUMHISTQM];
//QM QmHis[NUMHISTQM];
GN gn;



int
main(int argc, char **argv)
{

	int	i,j;
	int indice;
	int intervalo;
	int ing=0;						 // 1=ingenieria, 0=cuentas
	int	ParNum,ParNumAux,NumAna,NumDat;
	float min,max;
	float	cuentas[576],valor;		// 576 es el maximo numero de datos = 96 horas
	long	segjulqm, segjulini,segjulfin;
	char	*Aux,LisSen[4][10],LisPer[4][10],titulo[100];
	char sen;
	int num, tipo, anio, mes, dia, hora ,mins;
	struct tm *newtime;



	if (argc != 10)
		{
		printf("\nUso:\t datoscgi2 tipoSenial #Senial #horas ingenieria #anio #mes #dia #hora #min\n");
		return(1);
		}


		sscanf(argv[1], "%c", &sen);

		sscanf(argv[2], "%d", &num);

		NumAna = num;


		if(sen == 'G') NumAna=NumAna+G1-1;	// Si es Gray
		if(sen == 'A') NumAna=NumAna-1;		// Si es Analogica
		if(sen == 'R') NumAna=NumAna+R1-1;	// Si es RS232
		if(sen == 'C') NumAna=NumAna+C1-1;	// Si es Contador

		//Titulo del grafico
		if(i=ReadLogerBd(&BdConf)!=0){			// Leer Objeto B.D  BDCONF
			printf("\n\tReadAxisBd:Error=%d",i);
			exit(1);}
		strcpy(titulo,BdConf.anaconf.tag[NumAna]);
		strcat(titulo," - ");
		strcat(titulo,BdConf.anaconf.desc[NumAna]);
		//strcat(titulo," - ");
		//strcat(titulo,BdConf.remconf.desc);

		//segun tipo de grafico diferente intervalo de datos: tipo==numero de horas

		sscanf(argv[3], "%d", &tipo);
		NumDat=tipo*4;

		// Si son valores de ingenieria o cuentas (1=ingenieria, 0=cuentas)
		sscanf(argv[4], "%d", &ing);

		//Lectura datos, primero parsear fecha inicial
		segjulqm=time(NULL);
                newtime=localtime(&segjulqm);
		sscanf(argv[5], "%d", &anio);
                newtime->tm_year=anio-1900;
		sscanf(argv[6], "%d", &mes);
		newtime->tm_mon=mes-1;
		sscanf(argv[7], "%d", &dia);
                newtime->tm_mday=dia;
		sscanf(argv[8], "%d", &hora);
		newtime->tm_hour=hora;
		sscanf(argv[9], "%d", &mins);
		newtime->tm_min=mins;
		newtime->tm_sec=0;
                segjulqm=mktime(newtime);

		segjulini = segjulqm;

		//Lectura datos QM

//		memset((char *)&qm,0,sizeof(QM));
		if( (i=ReadLogerQmHis(&qm)) != 0 )
		{			// Leemos todos los QMs

			printf("\n\tReadAxisQmHist:Error=%d",i);
			exit(1);
		}
		if( (i=ReadLogerGn(&gn)) != 0 )
		{			// Leemos todos los QMs

			printf("\n\tReadAxisGn:Error=%d",i);
			exit(1);
		}

		if(segjulini > gn.segjulhis)
		{
			printf("\n\tError de fechas segjulini (%ld) > gn.segjulhis (%ld)",segjulini,gn.segjulhis);
			exit(1);
		}

		indice = NUMHISTQM +5000;
		for(i = 0;i<NUMHISTQM;i++)
		{
//printf("\n%d - %d - %d",i,qm[i].SegJul,segjulini);
			if(qm[i].SegJul == segjulini)
			{
				indice = i;
				break;
			}
		}
//printf("\n\nindice: %d\n",indice);
		if(indice == NUMHISTQM +5000)
		{
			printf("\n\tError de fechas");
			exit(1);
		}


		switch(sen)
		{
			case 'A':
			valor=qm[indice].ValorAna[NumAna];
			break;

			case 'G':
			valor=qm[indice].ValorGray[NumAna-G1];
			break;

			case 'R':
			valor=qm[indice].ValorRs[NumAna-R1];
			break;

			case 'C':
			//valor=qm[indice].ValorCont[NumAna-C1];
			break;
		}

		if(ing==1)
		{
			//valor=BdConf.anaconf.fca[NumAna] + valor*BdConf.anaconf.fcm[NumAna];
			valor=qm[indice].FlValorAna[NumAna];
		}
		min=max=valor;


		for(j=0;j<=NumDat;j++)
		{
			if((indice+j) >= NUMHISTQM) indice = -j;
			if(qm[indice+j].SegJul < segjulini)
			{
//printf("\n\t%d - %d < %d",indice+j,qm[indice+j].SegJul, segjulini);
				cuentas[j]=-999999;
				continue;
			}

			switch(sen)
			{
				case 'A':
				valor=qm[indice+j].ValorAna[NumAna];
				break;

				case 'G':
				valor=qm[indice+j].ValorGray[NumAna-G1];
				break;

				case 'R':
				valor=qm[indice+j].ValorRs[NumAna-R1];
				break;

				case 'C':
				//valor=qm[indice+j].ValorCont[NumAna-C1];
				break;
			}
			segjulqm=segjulqm+SEGPQM;
			//si es valor ingenieria se hace la conversion
			if(ing==1)
			{
				//valor=BdConf.anaconf.fca[NumAna] + valor*BdConf.anaconf.fcm[NumAna];
				valor=qm[indice+j].FlValorAna[NumAna];
				//printf(" %3.2f",qm[indice+j].FlValorAna[NumAna]);
			}
			if(valor>max){max=valor;}
			if(valor<min){min=valor;}
			cuentas[j]=valor;
		}

		if(min==max)
			{
			if(min==0){max=5;min=0;}
			if(min!=0){min=max -1;max=max+1;}
			if(min<0){min=0;}
			}
		else
			{
//			max=(float)ceil(max + (0.1*(double)(max-min)) );
//			min=(float)floor(min - (0.1*(double)(max-min)) );
			max=1+(int)(max + (0.1*(double)(max-min)) );
			min=(int)(min - (0.1*(double)(max-min)) );
			}

			intervalo = (int)((NumDat+1)/12);
			if(NumDat>11 && intervalo<2) intervalo = 2;


/*********************************************************************************/
/*	Codigo del applet y parametros						 */
/*********************************************************************************/
			printf("{\n\"elements\": [\n\t{\n\"type\": \"scatter_line\",\n\"colour\": \"#DB1750\",\n\"width\": 3,\n\"values\": [");
			printf("\n{\"x\": %d ,\"y\": %3.2f}",segjulini,cuentas[0]);
			segjulfin=segjulini;
			for(j=1;j<NumDat;j++)
			{
				segjulfin=segjulfin+SEGPQM;
				if(cuentas[j]==-999999) 
					printf("\n,{\"x\": %d ,\"y\": null}",segjulfin);
				else 
					printf("\n,{\"x\": %d ,\"y\": %3.2f}",segjulfin,cuentas[j]);
			}
		if(ing ==1)
			printf("],\n\"text\": \"%s\",\n\"font-size\": 12 ,\n\"dot-style\": {\"type\": \"hollow-dot\", \"dot-size\": 3, \"halo-size\": 2, \"tip\": \"#date:d M y H:i#<br>Value: #val#\" }\n",BdConf.anaconf.uni[NumAna]);
		else
			printf("],\n\"text\": \"cuentas\",\n\"font-size\": 12 ,\n\"dot-style\": {\"type\": \"hollow-dot\", \"dot-size\": 3, \"halo-size\": 2, \"tip\": \"#date:d M y H:i#<br>Value: #val#\" }\n");
		printf("}],\n\"title\": {\n\"text\":\"%s\",\n\"style\":\"{font-size: 15px; color: #DB1750}\"\n}",titulo);
		printf(",\n\"x_axis\":{\n\"min\":%d,\n\"max\":%d,\"steps\": %d,\n\"labels\": { \"text\": \"#date:d M y H:i#\",\"steps\": %d,\"visible-steps\": 2,\"rotate\": 40}\n}",segjulini,segjulfin,SEGPQM,(NumDat/10)*SEGPQM);
		//if(ing ==1) printf(",\n\"y_legend\":{\n\"text\":\"(%s)\",\n\"style\":\"{font-size: 12px; color: #464625}\"\n}",BdConf.anaconf.uni[NumAna]);
		printf(",\n\"y_axis\":{\n\"min\":\"%f\",\n\"max\":\"%f\"\n}\n}\n",min,max);

/*
		if(ing ==1) printf("\n&y_legend=%s (%s),12,#464625&",BdConf.anaconf.desc[NumAna],BdConf.anaconf.uni[NumAna]);
		else  printf("\n&y_legend=%s (cuentas),12,#464625&",BdConf.anaconf.desc[NumAna]);

		printf("\n&x_legend=Quinceminutal,12,#464625&");

		printf("\n&x_axis_steps=%d&",intervalo);
		printf("\n&x_offset=false&");

		printf("\n&y_ticks=5,10,%d&",NUMEY);
		printf("\n&y_min=%f&",min);

		printf("\n&line_hollow=2,0xCC3399,%s - %s - %02d/%02d/%d - %d horas,10,3&",BdConf.anaconf.tag[NumAna],BdConf.anaconf.desc[NumAna],dia,mes,anio,tipo);

		printf("\n&x_labels=");
		segjulqm=segjulini;
	        newtime=localtime(&segjulqm);
		printf("%02d:%02d",newtime->tm_hour,newtime->tm_min);

		for(j=1;j<=NumDat;j++)
		{
			segjulqm=segjulqm+900;
	                newtime=localtime(&segjulqm);
			printf(",%02d:%02d",newtime->tm_hour,newtime->tm_min);
		}
		printf("&");

		if(cuentas[0]==-999999) printf("\n&values=");
		else printf("\n&values=%3.2f",cuentas[0]);
		for(j=1;j<=NumDat;j++)
		{
			if(cuentas[j]==-999999) printf("");
			else printf(",%3.2f",cuentas[j]);
		}
		printf("&");


		printf("\n&x_label_style=13,0x0,0,%d&",intervalo);
		printf("\n&y_label_style=13,0x0,0,1&");
*/

////////		printf("\n<param name =\"ETIQUETASY\" value =\"");
////////		for(j=0;j<=NUMEY;j++){
////////			if(j>0){printf(",");}
////////			printf("%d",min+j*(min+max)/NUMEY);
////////		}
////////		printf("\">");

////////		printf("\n<applet code=\"GraficoVal.GrafValSemana.class\" archive=\"../GraficoVal.zip\" width=725 height=500>");
////////		printf("\n<param name =\"IDESTACION\" value =\"%s\">",cgl_getvalue("hora"));
////////		printf("\n<param name =\"IDPARAMETRO\" value =\"%s\">",cgl_getvalue("min"));
////////		printf("\n<param name =\"FECHAHORA\" value =\"%s/%s/%s\">",cgl_getvalue("mes"),cgl_getvalue("dia"),cgl_getvalue////////("anio"));

////////		printf("\n<param name =\"LONEJEX\" value =\"550\">");
////////		printf("\n<param name =\"MAXVALORX\" value =\"%d\">",NumDat);
////////		printf("\n<param name =\"LONEJEY\" value =\"350\">");
////////		printf("\n<param name =\"ENTEROX\" value =\"FALSE\">");
////////		printf("\n<param name =\"ENTEROY\" value =\"FALSE\">");

////////		printf("\n<param name =\"POSEJEX\" value =\"0\">");
////////		printf("\n<param name =\"POSEJEY\" value =\"0\">");

////////		printf("\n<param name =\"ETIQUETASX\" value =\"");

//	}

	return 0;
}

/* END */
