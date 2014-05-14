#include <stdio.h>

main(argc,argv)
int argc;
char *argv[];
{
        long seg;
        char fecha[9],hora[9];

        if ((argc < 2) || (argc > 3)) exit(1);

	if(argc==2){
	    if(sscanf(argv[1],"%ld",&seg)){
		hisdate(0,&seg,fecha,hora);
		printf("\n\tFECHA=%s %s  %ld\n",fecha,hora,seg);}
	    }
	else if(argc==3){
	    hisdate(1,&seg,argv[1],argv[2]);
            printf ("\n\tFECHA=%s %s  JUL=%ld\n",argv[1],argv[2],seg);
	}
}
