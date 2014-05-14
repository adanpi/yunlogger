#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <netdb.h>
#include "ipcserver.h"
#include "logersaihbd.h"
#include "axisloger.h"


int main(int argc, char *argv[])
{
	int offset=0,i;
	if (argc !=2){
		printf("\n\tUso deftablas char (char ASCII)\n");
	}
	if(sscanf(argv[1],"%c",&offset)!=1){
		printf("\n\tError parseando parametro char\n");
		exit(0);}
	if( (i=DefinicionTablas(offset)) >= 0 ){
		printf("\n\t\t OK Actualiza Tablas: %d\n",i);
	}
	else
		printf("\n\t\t Fallo Actualiza Tablas: error %d \n",i);

	return(0);
}
