			/*Clave semaforos sincronizacion logeripc*/
#define LOGERIPC_SEMKEY 291157

#define MAX_BS 65536	/*Numero Max de bytes por Trasmision*/
#define NB1 512
#define NB2 1024	/*Numero max bytes por mensaje*/
#define NSEN 84		/*Numero max de senales Analogicas por Remota*/
#define NME 102		/*Numero max de Remotas en recepcion RTD */
#define NREM 513	/*Numero Maximo de remotas del Saih*/

#define CONN 1		/*Llamada con solo conexion del socket*/
#define CLOS 2		/*Llamada con socket conectado y close del socket*/
#define CNCL 3		/*Llamada con conexion y close del socket*/

#define IND_LMSG 0
#define IND_CICLICO 2
#define IND_PROTO 4
#define IND_ORIGEN 5
#define IND_DESTINO 7
#define IND_MSG 9
#define IND_EST 10
#define IND_INFO 12
#define IND_JUL 12
#define IND_CHK 16
#define IND_FIN 18

#define PROTO_LOGER 0x1	/*TIPO PROTOCOLO LOGER  */
#define PROTO_104 0x2	/*TIPO PROTOCOLO 104  */
#define MSG_QM 0x50	/*Identificacion Mensaje LOGER Quinceminutal*/
#define LMSG_QM 18	/*Longuitud msg qm Send 18 byte*/
#define MSG_IN 0x60	/*Identificacion Mensaje LOGER Incidencias*/
#define LMSG_IN 20	/*Lonquitud Msg INCID*/

#define NB_INI 10

#define IND_CODE 4
#define IND_NBYTE 5
#define CODE_F1 0x5A    /*Identificacion Mensaje Historicos*/
#define CODE_F2 0x5B    /*Identificacion Mensaje Datos en Tiempo Real*/
#define CODE_F3 0x5C    /*Identificacion Mensaje Transmision Bases de Datos*/
#define CODE_F4 0x5D
#define CODE_FH 0x13    /*Ident. Mensaje Hora HP - WNT */



typedef struct {
	int tipo;
	char remota[5];
	int inter; 
	int sen;
	} CLNTHIS_SEN;

typedef struct {
	char remota[NME][5];
	short numlog[NME];
	}CLNTRTD_SEND; 

typedef struct {
	char remota[NME][5];
	short numlog[NME];
	short numhw[NME];
	short zona[NME];
	char activa[NME][2];
	float valor[NME][84];
	unsigned long dvalor[NME][3];
	long srv_mtime[NME];
	long julsaih;
	}CLNTRTD_RECV; 

typedef struct {
	char remota[5];
	short numlog;
	}CLNTBD_SEND; 

	 
















typedef struct {
	short numlog;
	char name[12];
	char des_rem[62];
        char numhw[5];
        char zona[5];
	char des_sen_ana[NSEN][34];
	char tag_sen_ana[NSEN][14];
	char uni_sen_ana[NSEN][8];
	char lim_sen_ana[NSEN][8][12];
	char fco_sen_ana[NSEN][2][12];
	char des_sen_dig[NSEN-4][34];
	char tag_sen_dig[NSEN-4][14];
	char can_sen_dig[NSEN-4][14];
	char eon_sen_dig[NSEN-4][14];
	char eof_sen_dig[NSEN-4][14];
	}CLNTBD_RECV;

typedef struct {
	short numlog[NREM];
	long clnt_mtime[NREM]; 
	}REM_BD; 

typedef struct {
	char remota[NREM][5];
	short numlog[NREM];
	short numhw[NREM];
	short zona[NREM];
	char activa[NREM][2];
	float valor[NREM][84];
	unsigned long dvalor[NREM][3];
	long srv_mtime[NREM];
	long julsaih;
	}REM_SAIH;


	/*Codigos y subcodigos de mensajes*/
#define LOGERIPC_ACK 0			/*ACK general*/
#define LOGERIPC_NACK 1			/*NACK general*/
#define LOGERIPC_FRAME_ERROR 1		/*Error en formato*/
#define LOGERIPC_CHECKSUM_ERROR 2		/*Error en checksum*/
#define LOGERIPC_CODE_ERROR 3		/*Codigo no existente*/
#define LOGERIPC_IPC_ERROR 4		/*Error en operacion*/
#define LOGERIPC_USER_ERROR 5		/*Fallo en autorizacion*/
#define LOGERIPC_BLOCK_ERROR 6		/*Operacion actualmente bloqueada*/
#define LOGERIPC_INFO_ERROR 7		/*Informacion incongruente*/
#define LOGERIPC_MSG_ 11			/*Envio de mensajes*/
#define LOGERIPC_RTD 21			/*Operaciones sobre rtdpaq*/
