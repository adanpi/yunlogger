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
#define PROTO_SAC 0x1		// TIPO PROTOCOLO SAC  
#define PROTO_104 0x2		// TIPO PROTOCOLO 104  
//--------------------------------------------------------------------------------------
#define SAC_MSG_QM 0x19		// Red Segundaria Identificacion Mensaje SAC Quinceminutal
#define SAC_MSG_IN 0x1A		// Red Segundaria Identificacion Mensaje SAC Incidencias
#define SAC_MSG_TM 0x25		// Red Segundaria Identificacion Mensaje SAC TIME
#define SAC_MSG_DS 0x26		// Red Segundaria Identificacion Mensaje SAC SEND Dimension Señeles Base de Datos
#define SAC_LMSGCL_QM 20	// Longuitud MSG QM en el Envio del Cliente
#define SAC_LMSGSR_QM 212	// Longuitud Max MSG QM en la Recepcion del Cliente
#define SAC_LMSGCL_IN 18	// Lonquitud MSG INCID en Cliente
#define SAC_LMSGSR_IN 512	// Longuitud Max MSG INCID en la Recepcion del Cliente
#define SAC_LMSGCL_TM 28	// Lonquitud MSG TIME en Cliente
#define SAC_LMSGCL_DS 28	// Lonquitud MSG Dimension Señales Base de datos
//------------------------------------------------------------------------------------------
#define SAC_IND_INI_QM 0	// Indices BufSend PROTO SAC MSG QM
#define SAC_IND_MSG_QM 4
#define SAC_IND_WORD_QM 5
#define SAC_IND_CIC_QM 6
#define SAC_IND_RUTA_QM 8
#define SAC_IND_EST_QM 9
#define SAC_IND_INFO_QM 12
#define SAC_IND_JUL_QM 12
#define SAC_IND_CHKCL_QM 16
#define SAC_IND_FINCL_QM 18
#
#define SAC_IND_STATUS_QM 16	// Indices PROTO SAC Solo en IpcServer MSG QM
#define SAC_IND_JULPER_QM 18
#define SAC_IND_SEGC1_QM 22
#define SAC_IND_SEGC2_QM 24
#define SAC_IND_SEGC3_QM 26
#define SAC_IND_NUMCONT_QM 28
#define SAC_IND_NUMANA_QM 54
#define SAC_IND_NUMGRAY_QM 88
#define SAC_IND_NUMRS_QM 146
#
#define SAC_IND_CHKSR_QM 208
#define SAC_IND_FINSR_QM 210
#
#define SAC_P0_QM 0x80		// Definicion Protocolo Cabecera y Fin de Mensaje PROTO SAC
#define SAC_P1_QM 0x01
#define SAC_P2_QM 0x02
#define SAC_P3_QM 0x03
#define SAC_P4_QM 0x19
#
#define SAC_FIN_QM 0xa5		// Cliente y Server 
#
#define NB_INI 16
#
//------------------------------- GEN -----------------------------------------------
#
#define SAC_IND_INI 0		// Indices BufSend PROTO SAC
#define SAC_IND_MSG 4
#define SAC_IND_WORD 5
#define SAC_IND_CIC 6
#define SAC_IND_RUTA 8
#define SAC_IND_EST 9
#define SAC_IND_INFO 12
#
#define SAC_P0 0x80		// Definicion Protocolo Cabecera y Fin de Mensaje PROTO SAC
#define SAC_P1 0x01
#define SAC_P2 0x02
#define SAC_P3 0x03
#define SAC_P4 0x19
#define SAC_FIN 0xa5		// Cliente y Server
#
//------------------------------- TIME --------------------------------------------
#
#define SAC_P5_TM 0x08		// Num Byte en Word
#define SAC_IND_EST1_TM 22	// Num Estacion
#define SAC_IND_CHK_TM 24
#define SAC_IND_FIN_TM 26
//------------------------------- INCID ------------------------------------------------------
#
#define SAC_P14_IN -1		// Indice Actual
#define SAC_P15_IN -1		// Indice Actual
#
#define SAC_P5_IN 0x03		// Num Byte en Word (Desde inclusive Ruta hasta Sin CHK)
#define SAC_P55_IN 0x05		// Num Byte en Word : 5*2 Byte 
#define SAC_IND_IAC_IN 12
#define SAC_IND_CHK_IN 14
#define SAC_IND_FIN_IN 16
#define SAC_IND_CHKSR_IN 18
#define SAC_IND_FINSR_IN 20
#
#define SAC_IND_NIN_IN 14
//------------------------------------ ------------------------------------------------------
//------------------------------------ ------------------------------------------------------
#define SACBUFFER_IND_INI_QM 0	// Indices BufSend PROTO LOGER MSG QM
#define SACBUFFER_IND_MSG 4
#define SACBUFFER_IND_EST 9
#define SACBUFFER_IND_INFO 12
#define SACBUFFER_IND_CONT 28	// Inicio Contadores
#define SACBUFFER_IND_WORD 5
#define SACBUFFER_FIN 0xa5		// Cliente y Server
#define SACBUFFER_CINC1 0x01	// Periodo Concominutal 300 seg
#define SACBUFFER_CINC2 0x2C
#define SACBUFFER_P0 0x80		// Definicion Protocolo Cabecera y Fin de Mensaje PROTO LOGER
#define SACBUFFER_P1 0x01
#define SACBUFFER_P2 0x02
#define SACBUFFER_P3 0x03
#define SACBUFFER_P4 0xE7
//#define SEGJULCTESAC 315532800	// Seg desde 1970 hasta 1980
//------------------------------------ ------------------------------------------------------
//
//short CrearBufferQm( QM *qm,short ihw,unsigned char *BufferQm);	// Funcion que crea bytes QM protocolo SAC
short CrearBufferQm_v2(short ihw );	// Funcion que crea bytes QM protocolo SAC
//------------------------------------ ------------------------------------------------------
