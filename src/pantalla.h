/*
 * pantalla.h
 *
 *  Created on: 17-jun-2009
 *      Author: adan
 *
 *      Define posiciones y tamaños en la bbdd modbus
 *      los tamaños son en word (2 bytes)
 */

#define NUMSENANAMB 22
#define NUMSENANAMBDATOS 8

	// Nombre
#define POS_MB_BDCONF_REMCONF_NAME 1
#define SIZE_MB_BDCONF_REMCONF_NAME 6
	// Descripcion
#define POS_MB_BDCONF_REMCONF_DESC (POS_MB_BDCONF_REMCONF_NAME + SIZE_MB_BDCONF_REMCONF_NAME)	// 7
#define SIZE_MB_BDCONF_REMCONF_DESC 21
	// Numero hardware
#define POS_MB_BDCONF_REMCONF_HW (POS_MB_BDCONF_REMCONF_DESC + SIZE_MB_BDCONF_REMCONF_DESC)	// 28
#define SIZE_MB_BDCONF_REMCONF_HW 1
	// Direccion IP
#define POS_MB_BDCONF_REMCONF_IP (POS_MB_BDCONF_REMCONF_HW + SIZE_MB_BDCONF_REMCONF_HW)	// 29
#define SIZE_MB_BDCONF_REMCONF_IP 8
	// Direccion IP1
#define POS_MB_BDCONF_REMCONF_IP1 (POS_MB_BDCONF_REMCONF_IP + SIZE_MB_BDCONF_REMCONF_IP)	// 37
#define SIZE_MB_BDCONF_REMCONF_IP1 8
	// Direccion IP2
#define POS_MB_BDCONF_REMCONF_IP2 (POS_MB_BDCONF_REMCONF_IP1 + SIZE_MB_BDCONF_REMCONF_IP1)	// 45
#define SIZE_MB_BDCONF_REMCONF_IP2 8

	// Direccion Tag Analogica 1
#define POS_MB_BDCONF_ANACONF_TAG1 (POS_MB_BDCONF_REMCONF_IP2 + SIZE_MB_BDCONF_REMCONF_IP2)	// 53
#define SIZE_MB_BDCONF_ANACONF_TAG1 11
	// Direccion Descripcion Analogica 1
#define POS_MB_BDCONF_ANACONF_DESC1 (POS_MB_BDCONF_ANACONF_TAG1 + SIZE_MB_BDCONF_ANACONF_TAG1)	// 64
#define SIZE_MB_BDCONF_ANACONF_DESC1 21
	// Direccion Factor Multiplicativo 1
#define POS_MB_BDCONF_ANACONF_FCM1 (POS_MB_BDCONF_ANACONF_DESC1 + SIZE_MB_BDCONF_ANACONF_DESC1)	// 85
#define SIZE_MB_BDCONF_ANACONF_FCM1 2
	// Direccion Factor Aditivo 1
#define POS_MB_BDCONF_ANACONF_FCA1 (POS_MB_BDCONF_ANACONF_FCM1 + SIZE_MB_BDCONF_ANACONF_FCM1)	// 87
#define SIZE_MB_BDCONF_ANACONF_FCA1 2
	// Direccion Tabla 1
#define POS_MB_BDCONF_ANACONF_TABLA1 (POS_MB_BDCONF_ANACONF_FCA1 + SIZE_MB_BDCONF_ANACONF_FCA1)	// 89
#define SIZE_MB_BDCONF_ANACONF_TABLA1 2
	// Direccion Formula 1
#define POS_MB_BDCONF_ANACONF_FORMULA1 (POS_MB_BDCONF_ANACONF_TABLA1 + SIZE_MB_BDCONF_ANACONF_TABLA1)	// 91
#define SIZE_MB_BDCONF_ANACONF_FORMULA1 2
	// Direccion NUMANA 1
#define POS_MB_BDCONF_ANACONF_NUMANA1 (POS_MB_BDCONF_ANACONF_FORMULA1 + SIZE_MB_BDCONF_ANACONF_FORMULA1)	// 93
	// Tamaño de NumAna + NumCont + ....
#define SIZE_MB_BDCONF_ANACONF_NUMANA1 6

	// Direccion Unidades Analogica 1
#define POS_MB_BDCONF_ANACONF_UNI1 (POS_MB_BDCONF_ANACONF_NUMANA1 + SIZE_MB_BDCONF_ANACONF_NUMANA1)	// 99
#define SIZE_MB_BDCONF_ANACONF_UNI1 6

	// Total tamaño configuracion una analogica:
#define SIZE_MB_BDCONF_ANACONF (POS_MB_BDCONF_ANACONF_UNI1+SIZE_MB_BDCONF_ANACONF_UNI1 - POS_MB_BDCONF_ANACONF_TAG1)	// 105 -53 = 52

	// Direccion Tag Digital 1
#define POS_MB_BDCONF_DIGCONF_TAG1 (POS_MB_BDCONF_ANACONF_TAG1 + NUMSENANAMB*SIZE_MB_BDCONF_ANACONF)	// 53 + 22*52 = 1197
#define SIZE_MB_BDCONF_DIGCONF_TAG1 11
	// Direccion Desc Digital 1
#define POS_MB_BDCONF_DIGCONF_DESC1 (POS_MB_BDCONF_DIGCONF_TAG1 + SIZE_MB_BDCONF_DIGCONF_TAG1)	// 1208
#define SIZE_MB_BDCONF_DIGCONF_DESC1 21
	// Direccion Etiqueta0 Digital 1
#define POS_MB_BDCONF_DIGCONF_ETI01 (POS_MB_BDCONF_DIGCONF_DESC1 + SIZE_MB_BDCONF_DIGCONF_DESC1)	// 1229
#define SIZE_MB_BDCONF_DIGCONF_ETI01 11
	// Direccion Etiqueta1 Digital 1
#define POS_MB_BDCONF_DIGCONF_ETI11 (POS_MB_BDCONF_DIGCONF_ETI01 + SIZE_MB_BDCONF_DIGCONF_ETI01)	// 1240
#define SIZE_MB_BDCONF_DIGCONF_ETI11 11
	// Direccion  NumDig Digital 1 + MAscaras +....
#define POS_MB_BDCONF_DIGCONF_NUMDIG1 (POS_MB_BDCONF_DIGCONF_ETI11 + SIZE_MB_BDCONF_DIGCONF_ETI11)	// 1251
	// Tamaño de NumDig + MAscaras + ....
	// configuracion digitales invertidas
#define SIZE_MB_BDCONF_DIGCONF_NUMDIG1 5

	// Total tamaño configuracion una digital:
#define SIZE_MB_BDCONF_DIGCONF (POS_MB_BDCONF_DIGCONF_NUMDIG1+SIZE_MB_BDCONF_DIGCONF_NUMDIG1 - POS_MB_BDCONF_DIGCONF_TAG1)	// 1256 - 1197 = 59

// Dejamos libres hasta la 4000 y empezamos con los valores minutales y quinceminutales

	// Direccion Minutal 1 Fecha
#define POS_MB_ANAMIN_FECHA 4000	// 4000
#define SIZE_MB_ANAMIN_FECHA 2
	// Direccion Ultimo Minutal  Valor Analogica 1 en Cuentas
#define POS_MB_ANAMIN_ANA1 (POS_MB_ANAMIN_FECHA + SIZE_MB_ANAMIN_FECHA)	// 4002
#define SIZE_MB_ANAMIN_ANA1 1
	// Direccion Ultimo Minutal  Valor Analogica 1 en Valor de Ingenieria
#define POS_MB_ANAMIN_ING_ANA1 (POS_MB_ANAMIN_ANA1 + SIZE_MB_ANAMIN_ANA1*NUMSENANAMBDATOS)	// 4010
#define SIZE_MB_ANAMIN_ING_ANA1 2

	// Tamaño total Ultimo Minutal
#define SIZE_MB_ANAMIN (SIZE_MB_ANAMIN_FECHA+(SIZE_MB_ANAMIN_ANA1+SIZE_MB_ANAMIN_ING_ANA1)*NUMSENANAMBDATOS)	// 2+(1+2)*8 = 26

	// Ultimo QM
#define POS_MB_ANAQM_FECHA (POS_MB_ANAMIN_FECHA+SIZE_MB_ANAMIN)		//4026
#define SIZE_MB_ANAQM_FECHA 20	//fecha en texto
	// Direccion Ultimo QM  Valor Analogica 1 en Cuentas
#define POS_MB_ANAQM_ANA1 (POS_MB_ANAQM_FECHA + SIZE_MB_ANAQM_FECHA)	// 4046
#define SIZE_MB_ANAQM_ANA1 1
	// Direccion Ultimo QM  Valor Analogica 1 en Valor de Ingenieria
#define POS_MB_ANAQM_ING_ANA1 (POS_MB_ANAQM_ANA1 + SIZE_MB_ANAQM_ANA1*NUMSENANAMBDATOS)	// 4054
#define SIZE_MB_ANAQM_ING_ANA1 2

	// Tamaño total Ultimo QM
#define SIZE_MB_ANAQM (SIZE_MB_ANAQM_FECHA+(SIZE_MB_ANAQM_ANA1+SIZE_MB_ANAQM_ING_ANA1)*NUMSENANAMBDATOS)	// 20+(1+2)*8 = 44

	// Ultimo Calculada num1 (caudal)
#define POS_MB_CALCQM_CALC1 (POS_MB_ANAQM_FECHA+SIZE_MB_ANAQM)		//4026+44=4070

	// Contadores empezamos en 4100
#define POS_MB_CONT_NUMMIN 4099		// numero minutal 0-14	
#define POS_MB_CONT1_MIN 4100		// valor minutal contador 1
#define POS_MB_CONT2_MIN 4101
#define POS_MB_CONT1_ACU 4102		// valor acumulado hasta el minutal contador 1
#define POS_MB_CONT2_ACU 4103
#define POS_MB_CONT1_CINC1_QM 4104	// valor cincominutal primero del quinceminutal del contador 1
#define POS_MB_CONT1_CINC2_QM 4105
#define POS_MB_CONT1_CINC3_QM 4106
#define POS_MB_CONT2_CINC1_QM 4107
#define POS_MB_CONT2_CINC2_QM 4108
#define POS_MB_CONT2_CINC3_QM 4109
#define POS_MB_CONT1_QM_ING 4110	// valor acumulado QM en valor ingenieria
#define POS_MB_CONT2_QM_ING 4111

	// Estadistica comunicaciones en 4200
#define POS_MB_GPRS_IP1 4200
#define POS_MB_GPRS_IP2 4201
#define POS_MB_GPRS_IP3 4202
#define POS_MB_GPRS_IP4 4203
#define POS_MB_GPRS_TOT 4204
#define POS_MB_GPRS_OK 4205
#define POS_MB_GPRS_EFI 4206

#define POS_MB_TETRA_IP1 4210
#define POS_MB_TETRA_IP2 4211
#define POS_MB_TETRA_IP3 4212
#define POS_MB_TETRA_IP4 4213
#define POS_MB_TETRA_TOT 4214
#define POS_MB_TETRA_OK 4215
#define POS_MB_TETRA_EFI 4216




	// Incidencias
#define POS_FECHA_IN1 4500	// fecha incidencia 1 (ascii)
#define SIZE_MB_IN_FECHA 20	//fecha en texto
#define POS_SEN_IN1 4520	// señal
#define POS_ESTADO_IN1 4521	// estado
#define SIZE_IN_MB 22
#define NUM_IN_MB 10
#define SIZE_IN_MB_TOT (SIZE_IN_MB*NUM_IN_MB)	// tamaño total incidencias modbus = 22*10 = 220

	// Reloj
#define POS_MB_SEG 4973
#define POS_MB_MIN 4974
#define POS_MB_HORA 4975
#define POS_MB_DIA 4976
#define POS_MB_MES 4977
#define POS_MB_ANIO 4978

	// Digitales out
#define POS_MB_DIG_IN1 0
#define POS_MB_DIG_OUT1 128


//BDCONF BdConf;
