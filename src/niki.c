/*	AUTOR:@M.B@
*	DATE:01-01-95
*	FILE:niki
*	PRODUCT:saih
*	S.O:unix
*/

#include <stdio.h>
#include <string.h>


main(argc,argv)
int argc;
char *argv[];
{
        unsigned char buffer[12];
	unsigned char aux[4][9];
	char *num;
        unsigned long l; long ll;
	unsigned int uno;
	int i,j,k;

	uno=128;
	if (argc != 3) exit(1);
	num=buffer;
	if(argv[1][1]=='x'){
	    sscanf(argv[2],"%ld",&l);
	    move(num,&l,4);
	    printf("\n    NUM=");
	    for(i=0;i<4;i++)
		printf("%02.2x ",num[i]);
	    printf("  %ld\n",l);
	    for(j=0;j<4;j++){
		printf("    ");
		for(k=0;k<8;k++){
		   if(num[j] & (uno >> k))
			memset(aux[j]+k,1,1);
		   else 
			memset(aux[j]+k,0,1);
		printf("%x ",aux[j][k]);
	    	}
	    }
	}
	else if(argv[1][1]=='d'){
	    sscanf(argv[2],"%x",&l);
	    printf("\n    NUM=%x  %ld\n",l,l);
	    move(num,&l,4);
	    for(j=0;j<4;j++){
		printf("    ");
		for(k=0;k<8;k++){
		   if(num[j] & (uno >> k))
			memset(aux[j]+k,1,1);
		   else 
			memset(aux[j]+k,0,1);
		printf("%x ",aux[j][k]);
	    	}
	    }
	 }
	 printf("\n");
}

move(p1,p2,l)
char *p1,*p2;int l;
{
    long laux;
    int *j;

    if (!p1) p1=&laux;
    memcpy(p1,p2,l);
    /*return((l == 4)  ? laux : *((short *)laux));*/
    return(0);
}
