/*
*	S.A.C.
*	FILE:		seg_juliano.c
*	AUTHOR:		F.L.S.@M.B@
*	DATE:		13-12-98
*	REVISION:	1.1
*	PRODUCT:	COMPAQ
*	SUBJECTS:	Calculo del segundo juliano correspondiente a una
*			estructura time_t y viceversa
*	O.S.:		UNIX	
*	CUSTOMER:	S.A.C.
*/

#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#define SEGPDIA 86400		/*Segundos en un dia*/
#define dif7080 (long)315532800	/*Segundos de 1 Enero 1970 a 1 Enero 1980*/
#define NDIANO	365		/*Dias en un ano*/
#define bisiesto(i) (0+i)%4		/*Test ano bisiesto*/
#define NDENE 31		/*Dias en cada mes*/
#define NDFEB 28
#define NDMAR 31
#define NDABR 30
#define NDMAY 31
#define NDJUN 30
#define NDJUL 31
#define NDAGO 31
#define NDSEP 30
#define NDOCT 31
#define NDNOV 30
#define NDDIC 31
static int mes[]={NDENE,NDFEB,NDMAR,NDABR,NDMAY,NDJUN,NDJUL,NDAGO,
			NDSEP,NDOCT,NDNOV,NDDIC};

/*	segtojul

	Calcula el segundo juliano correspondiente a una estructura
	time_t (ver <time.h>).

	FORMA DE LLAMADA C:

		sjul=segtojul(nsegs)

		long	sjul			segundos julianos calculados
		time_t	*nsegs			estructura time_t a calcular.
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

	Rutina que calcula el numero de segundos julianos solares transcurridos
	desde el 1 de Enero de 1980 a las 00:00:00 horas.
	Utiliza la rutina time que devuelve el numero de segundos transcurridos
	desde el 1 de Enero de 1970 a las 00:00:00 horas en una longitud del
	meridiano de Greenwich. A este valor le resta el numero de segundos
	transcurridos entre Enero de 1970 y Enero de 1980 y obtiene el numero
	de segundos julianos buscado. No es necesario hacer ninguna conversion
	de zona horal ya que estamos trabajando con segundos absolutos.
*/

long segtojul(nsegs)
time_t *nsegs;
{
	time_t clk;
	clk= (nsegs == NULL) ? time(NULL) : *nsegs;
	return (clk-dif7080);
}

/*	jultoseg

	Calcula la estructura time_t (ver <time.h>) correspondiente a un
	numero de segundos julianos.


	FORMA DE LLAMADA C:

		nsegs=jultoseg(sjul)

		time_t	nsegs			estructura time_t calculada
		long	*sjul			segundos julianos a calcular
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

	Rutina que calcula la estructura time_t corrspondiente a un numero de
	segundos julianos solares transcurridos desde el 1 de Enero de 1990 a
	las 00:00:00 horas.
*/

time_t jultoseg(sjul)
long *sjul;
{
	time_t jul1;
	jul1= (sjul == NULL) ? segtojul(NULL) : *sjul;
	return (jul1+dif7080);
}

/*	jultodia

	Calcula el numero de dia juliano correspondiente a un numero de
	segundos julianos.


	FORMA DE LLAMADA C:

		ndia=jultodia(sjul)

		long	ndia			numero de dia juliano
		long	*sjul			segundos julianos a calcular
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

	Rutina que calcula el dia juliano correspondiente a un numero de
	segundos julianos solares transcurridos desde el 1 de Enero de 1980 a
	las 00:00:00 horas. Como dia juliano se entiende el numero de dia a
	partir del 1 de Enero de 1980.
*/

long jultodia(sjul)
long *sjul;
{
	long jul1;
	jul1= (sjul == NULL) ? segtojul(NULL) : *sjul;
	return (jul1/SEGPDIA);
}

/*	diatojul

	Calcula el segundo juliano de comienzo del dia juliano
	correspondiente.

	FORMA DE LLAMADA C:

		sjul=diatojul(ndia)

		long	sjul			segundos julianos a calcular
		long	*ndia			numero de dia juliano
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

	Rutina que calcula el  segundo juliano de comienzo correspondiente
	a un dia juliano. Como dia juliano se entiende el numero de dia a
	partir del 1 de Enero de 1980.
*/

long diatojul(ndia)
long *ndia;
{
	long dia1;
	dia1= (ndia == NULL) ? jultodia(NULL) : *ndia;
	return (dia1*SEGPDIA);
}

/*	diatofech

	Calcula una estructura fecha (dd/mm/aa) correspondiente a un dia
	juliano.

	FORMA DE LLAMADA C:

		fech=diatofech(ndia)

		char	*fech			Puntero a fecha calculada
		long	*ndia			numero de dia juliano
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

*/

char *diatofech(ndia)
long *ndia;
{
	int nano,nmes;
	static char fech[9];
	long dia1;
	dia1= (ndia == NULL) ? jultodia(NULL) : *ndia;
	for (nano=80;;nano++)
	{
		mes[1]= (bisiesto(nano+1900)) ? NDFEB : NDFEB+1;
		for (nmes=0;nmes < 12;nmes++)
		{
			if (dia1 >= mes[nmes])
				dia1=dia1-mes[nmes];
			else
			{
				sprintf (fech,"%2.2ld/%2.2d/%2.2d",
				++dia1,++nmes,nano);
				return(fech);
			}
		}
	}
}

/*	fechtodia

	Calcula el dia juliano correspondiente a una estructura
	fecha (dd/mm/aa).

	FORMA DE LLAMADA C:

		ndia=diatofech(fech)

		long	ndia			numero de dia juliano
		char	*fech			Puntero a fecha
		
	DESCRIPCION:

*/

long fechtodia(fech)
char fech[];
{
	int nano,nmes;
	long ndia;
	int nano1,nmes1,ndia1;
	if (strlen(fech) != 8) return (-1);
	if (sscanf (fech,"%2d/%2d/%2d",&ndia1,&nmes1,&nano1) != 3)
		return(-1);
	if (nano1 < 0) return(-1);
	nano1+= (nano1 < 80) ? 2000 : 1900;
	ndia=0;
	for (nano=1980;nano < nano1;nano++)
		ndia+= (bisiesto(nano)) ? NDIANO : NDIANO+1;
	if ((nmes1 <=0) || (nmes1 > 12)) return(-1);
	--nmes1;
	mes[1]= (bisiesto(nano)) ? NDFEB : NDFEB+1;
	for (nmes=0;nmes < nmes1;nmes++)
		ndia+= mes[nmes];
	if ((ndia1 <=0) || (ndia1 > mes[nmes])) return(-1);
	ndia+=ndia1;--ndia;
	return(ndia);
}

/*	horatoseg

	Calcula el  numero de segundos correspondientes a una hora
	ascii.

	FORMA DE LLAMADA C:

		seg=horatoseg(hora)

		long	seg			segundos de retorno
		char	*hora			Puntero a hora
		
	DESCRIPCION:

*/

long horatoseg(hora)
char hora[];
{
	long seg;
	int h,m,s;
	if ((sscanf(hora,"%2d:%2d:%2d",&h,&m,&s) != 3) || (h < 0) || (h > 23) ||
	(m < 0) || (m > 59) || (s < 0) || (s > 59)) return(-1);
	return(h*3600+m*60+s);
}

/*	segtohora

	Calcula estructura ascii hora correspondiente a un segundo
	dentro de un dia.

	FORMA DE LLAMADA C:

		hora=segtohora(seg)

		char	*hora			hora de retorno
		long	*seg			segundo del dia
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

*/

char *segtohora(seg)
long *seg;
{
	static char hora[10];
	long nseg;
	int h,m,s;
	nseg= (seg == NULL) ? jultoseg(NULL)%SEGPDIA : *seg;
	if ((nseg < 0) || (nseg >= SEGPDIA)) return(NULL);
	h=nseg/3600;nseg=nseg%3600;
	m=nseg/60;nseg;s=nseg%60;
	sprintf (hora,"%2.2d:%2.2d:%2.2d",h,m,s);
	return(hora);
}

/*	fechora

	Calcula el dia y la fecha correspondiente a un segundo juliano.

	FORMA DE LLAMADA C:

		fechora(sjul,fech,hora)

		long	*sjul			segundo juliano
						Si se pasa un puntero a NULL
						se calcula segun la hora
						ctual del reloj.
		char	*fech			Puntero a fecha
		char	*hora			Puntero a hora
		
	DESCRIPCION:

*/

void fechora(sjul,fech,hora)
long *sjul;
char fech[];
char hora[];
{
	long njul,nseg,ndia;
	njul = (sjul == NULL) ? segtojul(NULL) : *sjul;
	ndia=jultodia(&njul);
	nseg=njul%SEGPDIA;
	strcpy(fech,diatofech(&ndia));
	strcpy(hora,segtohora(&nseg));
}

/*	jultomes

	Calcula el numero de mes juliano correspondiente a un numero de
	segundos julianos.


	FORMA DE LLAMADA C:

		nmes=jultomes(sjul)

		long	nmes			numero de mes juliano
		long	*sjul			segundos julianos a calcular
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

	Rutina que calcula el mes juliano correspondiente a un numero de
	segundos julianos solares transcurridos desde el 1 de Enero de 1980 a
	las 00:00:00 horas. Como mes juliano se entiende el numero de mes a
	partir del 1 de Enero de 1980.
*/

long jultomes(sjul)
long *sjul;
{
	long jul1,jul2;
	int iano,imes;

	jul1= (sjul == NULL) ? segtojul(NULL) : *sjul;
	jul2=0;
	for (iano=80;;iano++)
	{
		mes[1]= (bisiesto(iano+1900)) ? NDFEB : NDFEB+1;
		for (imes=0;imes < 12;imes++)
		{
			jul2+=mes[imes]*SEGPDIA;
			if (jul2 >= jul1) break;
		}
		if (jul2 >= jul1) break;
	}
	return ((iano-80)*12+imes);
}

/*	mestojul

	Calcula el segundo juliano de comienzo del mes juliano
	correspondiente.

	FORMA DE LLAMADA C:

		sjul=mestojul(nmes)

		long	sjul			segundos julianos a calcular
		long	*nmes			numero de mes juliano
						Si se pasa un puntero a NULL
						se calcula segun la hora
						actual del reloj.
		
	DESCRIPCION:

	Rutina que calcula el segundo juliano de comienzo correspondiente
	a un mes juliano. Como dia juliano se entiende el numero de mes a
	partir del 1 de Enero de 1980.
*/

long mestojul(nmes)
long *nmes;
{
	long nmes1;
	long sjul;
	int iano,imes;

	nmes1= (nmes == NULL) ? jultomes(NULL) : *nmes;
	sjul=0;
	for (iano=80;iano-80 < nmes1/12;iano++)
	{
		mes[1]= (bisiesto(iano+1900)) ? NDFEB : NDFEB+1;
		for (imes=0;imes < 12;imes++) sjul+=mes[imes]*SEGPDIA;
	}
	mes[1]= (bisiesto(iano+1900)) ? NDFEB : NDFEB+1;
	for (imes=0;imes < nmes1%12;imes++) sjul+=mes[imes]*SEGPDIA;
	return (sjul);
}
