/*
*	S.A.C.
*	FILE:		
*	AUTHOR:		F.L.S.@M.B@
*	DATE:		08-01-90
*	REVISION:	1.8
*	PRODUCT:	LIBPAQ
*	SUBJECTS:	Timer services
*	O.S.:		UNIX	
*	CUSTOMER:	S.A.C.
*/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "timer.h"
#include "flags.h"

/*	settr

	FORMA DE LLAMADA C:

		istat=settr(fl_id,fl_num,itim,iunit)

		int	istat			status de salida
		int	fl_id			identificador de grupo de flags
		int	fl_num			flag dentro del grupo
		long	itim			numero de unidades de tiempo
		int	iunit			unidad de tiempo
							1 : ticks
		
	DESCRIPCION:

		Rutina encarga de activar el servicio timer del sistema.
		con un tiempo especificado.
		NOTA: Esta rutina clear el flag correspondiente.
		Retorna:
			istat=0		operacion OK
			istat<0		error segun errno cambiado de signo
*/

int shmid=0;		/*id memoria compartida*/
int flgid=0;		/*id flag control acceso a memoria compartida*/
struct tim_str *tim_data=NULL;	/*Acceso a tabla de mark-time*/

settr(fl_id,fl_num,itim,iunit)
int	fl_id,fl_num,iunit;
long	itim;
{
	int i;

	/*Primera entrada:
		allocate shared memory and attach
		get flag control de acceso*/

	if (!tim_data)
		if (((shmid=shmget(333,TIM_NTIM*sizeof(struct tim_str)
		,0666)) == -1) || ((int)(tim_data=shmat(shmid,0,0)) == -1)
		 || ((flgid=initef(SYSTEM_FL,0,NULL)) < 0)) return(-errno);

	/*Esperar a que tengamos libre el flag de acceso y borrarlo*/

	if ((waitef(flgid,TIM_FL) < 0) || (clearef(fl_id,fl_num) < 0))
	{
		setef(flgid,TIM_FL);	/*Permitir otros accesos*/
		return(-errno);
	}

	/*Buscar primer elemento libre en la lista e insertar*/

	for (i=0;i<TIM_NTIM;i++)
		if (!tim_data[i].ticks)
		{
			tim_data[i].fl_id=fl_id;
			tim_data[i].fl_num=fl_num;
			tim_data[i].ticks=itim;
			setef(flgid,TIM_FL);	/*Permitir otros accesos*/
			return(0);
		}
	setef(flgid,TIM_FL);	/*Permitir otros accesos*/
	return(-1);	/*No quedan huecos libres*/
}

/*	cantr

	FORMA DE LLAMADA C:

		istat=cantr(fl_id,fl_num)

		int	istat			status de salida
		int	fl_id			identificador de grupo de flags
		int	fl_num			flag dentro del grupo
		
	DESCRIPCION:

		Rutina encarga de delogertivar el servicio timer del sistema.
		NOTA: Esta rutina clear el flag correspondiente.
		Retorna:
			istat=0		operacion OK
			istat<0		error segun errno cambiado de signo
*/

cantr(fl_id,fl_num)
int	fl_id,fl_num;
{
	int i;

	/*Esperar a que tengamos libre el flag de acceso y borrarlo*/

	if ((waitef(flgid,TIM_FL) < 0) || (clearef(fl_id,fl_num) < 0))
	{
		setef(flgid,TIM_FL);	/*Permitir otros accesos*/
		return(-errno);
	}

	/*Buscar el elemento correspondiente en la lista y borrar*/
	for (i=0;i<TIM_NTIM;i++)
		if ((tim_data[i].fl_id == fl_id) &&
			(tim_data[i].fl_num == fl_num))
		{
			tim_data[i].ticks=0;
			setef(flgid,TIM_FL);	/*Permitir otros accesos*/
			return(0);
		}
	setef(flgid,TIM_FL);	/*Permitir otros accesos*/
	return(-1);	/*No existe el elemento*/
}

/*	readtr

	FORMA DE LLAMADA C:

		istat=readtr(fl_id,fl_num)

		int	istat			status de salida
		int	fl_id			identificador de grupo de flags
		int	fl_num			flag dentro del grupo
		
	DESCRIPCION:

		Rutina encarga de leer el servicio timer del sistema.
		Retorna:
			istat>=0	operacion OK, decimas de segundo
					que faltan por cumplirse
			istat<0		error segun errno cambiado de signo
*/

long readtr(fl_id,fl_num)
int	fl_id,fl_num;
{
	int i;

	/*Buscar el elemento correspondiente en la lista y leer*/
	for (i=0;i<TIM_NTIM;i++) if ((tim_data[i].fl_id == fl_id) &&
		(tim_data[i].fl_num == fl_num)) return(tim_data[i].ticks);
	return(-1);	/*No existe el elemento*/
}

/*	timwait

	FORMA DE LLAMADA C:

		istat=timw(ticks)

		int	istat			status de salida
		int	ticks			tiempo a esperar
		
	DESCRIPCION:

		Rutina para esperar el numero de decimas de segundo indicado.
		Retorna:
			istat=0		operacion OK
			istat<0		error segun errno cambiado de signo
*/

void handnull() {;}

timw(ticks)
int ticks;
{
	static struct itimerval tvalue = {0,0,0,0};
	int oldpr;
	if (!ticks) return(0);
	oldpr=rtprio(0,127);
	if (signal(SIGALRM,handnull) == SIG_ERR) {rtprio(0,oldpr);return(0);}
	tvalue.it_value.tv_sec=ticks/10;
	tvalue.it_value.tv_usec=(ticks%10)*100000;
	tvalue.it_interval.tv_sec=0;
	tvalue.it_interval.tv_usec=0;
	if (setitimer(ITIMER_REAL,&tvalue,0) == -1)
		{signal(SIGALRM,SIG_IGN);return(-errno);}
	pause();
	signal(SIGALRM,SIG_IGN);
	rtprio(0,oldpr);
	return(0);
}


void handnull_np() {;}

timw_np(ticks)
int ticks;
{
	static struct itimerval tvalue = {0,0,0,0};
	int oldpr;

	if (!ticks) return(0);
	if (signal(SIGALRM,handnull_np) == SIG_ERR) {return(0);}
	tvalue.it_value.tv_sec=ticks/10;
	tvalue.it_value.tv_usec=(ticks%10)*100000;
	tvalue.it_interval.tv_sec=0;
	tvalue.it_interval.tv_usec=0;
	if (setitimer(ITIMER_REAL,&tvalue,0) == -1)
		{signal(SIGALRM,SIG_IGN);return(-errno);}
	pause();
	signal(SIGALRM,SIG_IGN);
	return(0);
}
