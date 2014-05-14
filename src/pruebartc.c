#include <time.h>
/*#**************************************************************************
*#                                                            
*# FUNCTION NAME: main
*#                                                            
*# DESCRIPTION  : check flags and calls requested functions
*#                                                            
*#**************************************************************************/
int main(int argc, char *argv[])
{
	int arg = 1;
	struct tm NewTim;

	NewTim.tm_year = 105;
	NewTim.tm_mon = 4;
	NewTim.tm_mday = 19;
	NewTim.tm_hour = 12;
	NewTim.tm_min = 15;
	NewTim.tm_sec = 01;
	
	if(argc==1){
		printf("Nueva fecha: \n");
		arg = set_time(NewTim);
	}
	else	
	{
		printf("Argumentos no validos: %s\n",argv[arg]);
	}


	return 0;
}
