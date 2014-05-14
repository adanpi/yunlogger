/************************************************************************************/
#define SIZEBDCONF 12000
#define NUMSENANA 32		// Numero Maximo de señales Analogicas
#define NUMSENDIG 16		// Numero Maximo de señales Digitales
#define NUMHISTIN 4000		// Numero Maximo de Registros Incidencias Digitales ;
#define SEGPQM 600		// Seg por QM (para SAICA 10 minutos=600sg)
#define SEGPDIA 86400		// Seg por DIA
#define NUMHISTQM 4464		//(SEGPDIA/SEGPQM)*31 Numero Maximo de Registros Quinceminutales ; Maximo un Mes
#define SEGJULCTELOGER 631152000	// Seg desde 1970 hasta 1990
#define SEGJULCTESAC 315532800	// Seg desde 1970 hasta 1980
#define SEGJULCTE 315532800	// Seg desde 1970 hasta 1980
#define BYTEQM	213		// Num Max de Byte de un QM
#define BYTEIN	513		// Num Max de Byte de un mensaje de Incidencias
#define NUMSENANASAICA 18		// Numero Max sen Analogicas Para SAICA
#define NUMSENGRAY 0		// Numero Max Sen Grays  
#define NUMSENRS 0		// Numero max Sen RS232 
#define NUMVALCONT 0		// 4 Contadores por 3 Cincomins = 6 Valores
#define NUMSENCONT 0		// 4 Señales Contadores
#define VALMAXIND 3999		// 3999 Valor Max del Indice de Incidencias = 4000 Incidencias
#define NUMINALM 4000		// s = 4000 Incidencias
#define MINBYTEMSG 10		// Valor Minimo de MSG en Recepcion ?????????????
#define NUMDIASREC 10		// Numero de dias a recuperar en envios FTP
#define TIMEOUTFTP 60
#define TIMELOGOUTFTP 100
#define PASIVO ""	// si es FTP pasivo "-s"

typedef struct {	// Configuracion Remota
	char name[12];
	char desc[42];
	char ipnameFTP[16];
	char usuario[16];
	char contrasenia[16];
	char directorio[16];
	short frecuencia;
	short ihw;
	}REMCONF; 

typedef struct {	// Configuracion Señales Analogicas
	char tag[NUMSENANA][22];
	char uni[NUMSENANA][12];
	char desc[NUMSENANA][42];
	float fcm[NUMSENANA];
	float fca[NUMSENANA];
	}ANACONF; 

typedef struct {	// Configuracion Señales Digitales
	char tag[NUMSENDIG][22];
	char desc[NUMSENDIG][42];
	char etiqueta0[NUMSENDIG][22];
	char etiqueta1[NUMSENDIG][22];
//	int Estado[NUMSENDIG];
	}DIGCONF; 

typedef struct {		// Configuracion Registro Quinceminutal
	short Status;
	unsigned long SegJul;
	unsigned long SegJulPer;
	unsigned short SegJulCinc[3];
	unsigned short ValorCont[NUMVALCONT];
	unsigned short ValorAna[NUMSENANA];
	unsigned short ValorGray[NUMSENGRAY];
	unsigned short ValorRs[NUMSENRS];
	float	FlValorAna[NUMSENANA];
	float	FlValorCont[NUMVALCONT];
	short NumCont;
	short NumAna;
	short NumGray;
	short NumRs;
	unsigned char BufferQm[BYTEQM];
	unsigned short lBufferQm;
	char Flag[NUMSENANA];		// Flag SAICA
	}QM; 

typedef struct {			// Configuracion Registros Historicos Digitales
	unsigned long SegJulIn[NUMHISTIN];
	unsigned short NumSen[NUMHISTIN];
	unsigned short Estado[NUMHISTIN];
	unsigned char BufferIn[NUMHISTIN][5];
	short IndAct;
	short IndUltIn;			// Indice Ultima Incidencia
	short NumInAlm;			// Num Incidencias almacenadas por la remota
	short ValMaxInd;		// Valor Maximo del Indice 0f 9f || 0f a0
	short FlagIn;			// Flag Recuperacion Incidencias
	}IN;

typedef struct {			// Configuracion Base de Datos
	REMCONF remconf;
	ANACONF anaconf;
	DIGCONF digconf;
	}BDCONF; 

typedef struct {			// Configuracion RS232, TTY,
	}TTY;
typedef struct {			// Configuracion General;
	unsigned long segjulhis;
	unsigned long IndHisAna;	// Indice Ultimo QM historico recuperado
	unsigned long IndActAna;	// Indice QM actual recuperado
	unsigned long IndHisCon;	// Indice Ultimo CM historico recuperado
	unsigned long UltEnvFtp;	// Fecha ultimo envio FTP correcto
	unsigned long IndHisDig;
	unsigned short SigQm;		// firma definicion tabla QM
//	unsigned short SigCm;		// firma definicion tabla CM
	unsigned short SigDig;		// firma definicion tabla DIG
	unsigned long NumComLoger[3];
	unsigned long NumComRed[3];
	unsigned long HorasFun[5];	// Horas Funcionamiento
	short MsgQmAct;			// Mensajes Activos
	short MsgTmAct;
	short MsgInAct;
	short MsgDsAct;
	short MsgXyAct[8];
	char aux[10][32];
	long iaux[10][10];
	}GN;



#define A1 0
#define A2 1
#define A3 2
#define A4 3
#define A5 4
#define C1 32
#define R1 32
#define G1 32

#define D1 0

