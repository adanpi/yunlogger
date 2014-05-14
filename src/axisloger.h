//----------------- DEFINICION RS232 -------------------------------------
#
#define NBW 40			//numero bytes escritura
#define NBR 2048		//numero bytes lectura
#define NCConf 20		//numero strings configuracion tty

#define COM1 "/dev/ttyS4"
//#define PTS "/dev/pts/0"
//#define Baudios "9600"
#define Baudios "115200"
#define Flow "none"
#define TimeOut "+1"
#define Verbose "1"

#define DEBUG 0			//imprimir informacion depuracion
#define FirmaIN 1		//comprobar firma entrada mensajes
#define PAUSA 0			//pausa entre mensajes para depuracion

#define NUM_MENSAJES_TABLAS 6
#define NUMBYTES_MENSAJE_TABLAS 985
#define LOGER_FIN_FLAG 0x2E	// caracter 0x2E (.) marca el final de los flag en el string
#define TIPO_LOGGER 3350	//tipo datalogger CR1000 valor 3350
//#define TIPO_LOGGER 3350	//tipo datalogger CR1000 valor 3350
//#define TIPO_LOGGER 600	//tipo datalogger CR200 valor 600 

typedef struct{
int bytesleidos;
char bufrecv[NBR];
}RX;

//----------------- DEFINICION PROTOCOLO DATALOGER ----------------------------
#define SAIHIPC_SEMKEY 291157	// Clave semaforos sincronizacion saihipc
//----------------------------------------------------------------------------
#define MAX_BS 65536		// Numero Max de bytes por Trasmision
#define NB 256			// Numero Max Byte por Mensaje
#define NB1 512			// Numero Max bytes por mensaje
#define NB2 1024		// Numero Max bytes por mensaje
#define NSEN 84			// Numero Max de senales Analogicas por Remota
#define NME 102			// Numero Max de Remotas en recepcion RTD 
#define NREM 513		// Numero Maximo de remotas del Saih
//-------------------------------------------------------------------------------------
#define CONN 1			// Llamada con solo conexion del socket
#define CLOS 2			// Llamada con socket conectado y close del socket
#define CNCL 3			// Llamada con conexion y close del socket
//------------------------------------------------------------------------------------
#define IND_PROTO 4		// IND PROTOCOLO 
#define PROTO_LOGER 0x1		// TIPO PROTOCOLO LOGER  
#define PROTO_104 0x2		// TIPO PROTOCOLO 104  
//--------------------------------------------------------------------------------------
#define LOGER_MSG_QM 0x09	// Red Segundaria Identificacion Mensaje LOGER Quinceminutal
#define LOGER_MSG_TM 0x17	// Red Segundaria Identificacion Mensaje LOGER Time
#define LOGER_MSG_RESP_QM 0x89	// Red Segundaria Identificacion Mensaje LOGER Respuesta Quinceminutal
#define LOGER_MSG_RESP_OK_QM 0x00	// Red Segundaria Identificacion Mensaje LOGER Respuesta Quinceminutal
#define LOGER_MSG_RESP_TM 0x97	// Red Segundaria Identificacion Mensaje LOGER Respuesta Time
#define LOGER_MSG_RESP_OK_TM 0x00	// Red Segundaria Identificacion Mensaje LOGER Respuesta Time
#define LOGER_MSG_IN 0x09	// Red Segundaria Identificacion Mensaje LOGER Incidencias
#define LOGER_MSG_DT 0x1D	// Red Segundaria Identificacion Mensaje LOGER Definicion Tabla
#define LOGER_MSG_DT_NUM 0x0e	// Red Segundaria Identificacion Mensaje LOGER Definicion Tabla por Numero
#define LOGER_LMSGCL_QM 40	// Longuitud MSG QM en el Envio del Cliente
#define LOGER_LMSGSR_QM 512	// Longuitud Max MSG QM en la Recepcion del Cliente
#define LOGER_LMSGCL_IN 40	// Lonquitud MSG INCID en Cliente
#define LOGER_LMSGSR_IN 512	// Longuitud Max MSG INCID en la Recepcion del Cliente
#define LOGER_LMSGCL_TM 40	// Lonquitud MSG TIME en Cliente
#define LOGER_MSG_SECCODE 0x00	// Segurity Code Mensajes Loger
#define LOGER_MSG_TIPO_QM 0x05	// Tipo de adquisicion datos Loger QM o CM
#define LOGER_MSG_TIPO_DIG 0x04	// Tipo de adquisicion datos Loger Dig
//Adquisicion Dig tambien tipo 05 (recolectar ultimos N registros)
//#define LOGER_MSG_TIPO_DIG 0x05	// Tipo de adquisicion datos Loger Dig
#define LOGER_MSG_TIPO_HIS_QM 0x06	// Tipo de adquisicion datos Loger QM o CM Historicos
#define LOGER_MSG_NUMTABLA1_A 0x00	// Analogicas
#define LOGER_MSG_NUMTABLA2_A 0x02
#define LOGER_MSG_SIGTABLA1_A 0x89
#define LOGER_MSG_SIGTABLA2_A 0x78
#define LOGER_MSG_NUMTABLA1_C 0x00	// Contadores
#define LOGER_MSG_NUMTABLA2_C 0x04
#define LOGER_MSG_SIGTABLA1_C 0x25
#define LOGER_MSG_SIGTABLA2_C 0x63
#define LOGER_MSG_NUMTABLA1_D 0x00	// Digitales
#define LOGER_MSG_NUMTABLA2_D 0x03
#define LOGER_MSG_SIGTABLA1_D 0xE7
#define LOGER_MSG_SIGTABLA2_D 0x21
#define LOGER_MSG_STATUS_IRREC -1	// Status Mensaje QM Irrecuperable protocolo Sac
#define LOGER_INT_MARCAR_IRREC 973	// Intervalo llamada a MarcarIrrecuperables (x 2 = Tiempo en segundos)
//------------------------------------------------------------------------------------------
#define LOGER_IND_INI_QM 0	// Indices BufSend PROTO LOGER MSG QM
#define LOGER_IND_MSG_QM 9
#define LOGER_IND_SECCODE_QM 11
#define LOGER_IND_TIPO_QM 13
#define LOGER_IND_NUMTABLA_QM 14
#define LOGER_IND_SIGTABLA_QM 16
#define LOGER_IND_P1_QM 18
#define LOGER_IND_P2_QM 22

#
#define LOGER_IND_STATUS_QM 11	// Indices Respuesta Loger
#define LOGER_IND_NUMREG_QM 14
#define LOGER_IND_SEGJUL_QM 20
//#define LOGER_IND_ANAINI_QM 28	//CR200
#define LOGER_IND_ANAINI_QM 32	//CR1000
//#define LOGER_IND_DIGINI 24	//CR200
#define LOGER_IND_DIGINI 28	//CR1000

#define LOGER_IND_ENVIO_TM 13
#define LOGER_IND_RESP_TM 12

#define LOGER_IND_SEGC3_QM 26
#define LOGER_IND_NUMCONT_QM 28
//#define LOGER_IND_NUMANA_QM 24	//CR200
#define LOGER_IND_NUMANA_QM 28	//CR1000
#define LOGER_IND_NUMGRAY_QM 88
#define LOGER_IND_NUMRS_QM 146
#
#define LOGER_IND_CHKSR_QM 208
#define LOGER_IND_FINSR_QM 210
#
#define LOGER_P0_QM 0xBD		// Definicion Protocolo Cabecera y Fin de Mensaje PROTO LOGER
#define LOGER_P1_QM 0xA0
#define LOGER_P2_QM 0x01
#define LOGER_P3_QM 0x4F
#define LOGER_P4_QM 0xFE
#define LOGER_P5_QM 0x10
#define LOGER_P6_QM 0x01
#define LOGER_P7_QM 0x0F
#define LOGER_P8_QM 0xFE

#
#define LOGER_FIN_QM 0xBD		// Cliente y Server 
#
#define NB_INI 16
#
//------------------------------- GEN -----------------------------------------------
#
#define LOGER_IND_INI 0		// Indices BufSend PROTO LOGER
#define LOGER_IND_MSG 4
#define LOGER_IND_WORD 5
#define LOGER_IND_CIC 6
#define LOGER_IND_RUTA 8
#define LOGER_IND_EST 9
#define LOGER_IND_INFO 12
#define LOGER_IND_CONT 28	// Inicio Contadores
#
#define LOGER_P0 0x80		// Definicion Protocolo Cabecera y Fin de Mensaje PROTO LOGER
#define LOGER_P1 0x01
#define LOGER_P2 0x02
#define LOGER_P3 0x03
#define LOGER_P4 0xE7
#define LOGER_WORD 0x30		// Numero Word mensaje 
#define LOGER_CINC1 0x01	// Periodo Concominutal 300 seg
#define LOGER_CINC2 0x2C
#define LOGER_FIN 0xa5		// Cliente y Server
#
//------------------------------- TIME --------------------------------------------
#
#define LOGER_P5_TM 0x08	// Num Byte en Word
#define LOGER_IND_EST1_TM 22	// Num Estacion
#define LOGER_IND_CHK_TM 24
#define LOGER_IND_FIN_TM 26
//------------------------------- INCID ------------------------------------------------------
#
#define LOGER_P14_IN -1		// Indice Actual
#define LOGER_P15_IN -1		// Indice Actual
#
#define LOGER_P5_IN 0x03		// Num Byte en Word (Desde inclusive Ruta hasta Sin CHK)
#define LOGER_P55_IN 0x05		// Num Byte en Word : 5*2 Byte 
#define LOGER_IND_IAC_IN 12
#define LOGER_IND_CHK_IN 14
#define LOGER_IND_FIN_IN 16
#define LOGER_IND_CHKSR_IN 18
#define LOGER_IND_FINSR_IN 20
#
#define LOGER_IND_NIN_IN 14
//------------------------------------ ------------------------------------------------------
