/*
*	S.A.C.
*	FILE:		timer.h
*	AUTHOR:		F.L.S.
*	DATE:		05-01-90
*	REVISION:	1.1
*	PRODUCT:	LIBPAQ
*	SUBJECTS:	Definiciones rutinas de temporizacion
*	O.S.:		UNIX	
*	CUSTOMER:	S.A.C.
*/

#define TIM_NTIM 50	/*Numero maximo mark-times simultaneos*/
#define TIM_FL 0	/*Flag sistema control accesos memoria compartida*/

struct tim_str {
	long ticks;
	int fl_id;
	int fl_num;
};
