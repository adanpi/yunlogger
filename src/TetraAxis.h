/************************************************************************************/
#define NREMOTAS 1000		//Numero Maximo de Remotas
#define SIZEBDCONF 12000
#define NUMSENANA 64		// Numero Maximo de señales Analogicas
#define NUMSENDIG 80		// Numero Maximo de señales Digitales
#define NUMHISTQM 2976		// Numero Maximo de Registros Quinceminutales ; Maximo un Mes
#define NUMHISTIN 4000		// Numero Maximo de Registros Incidencias Digitales ;
#define SEGPQM 600		// Seg por QM
#define SEGPDIA 86400;		// Seg por DIA
#define SEGJULCTE 315532800	// Seg desde 1970 hasta 1980
#define BYTEQM	213		// Num Max de Byte de un QM
#define BYTEIN	513		// Num Max de Byte de un mensaje de Incidencias
#define NUMSENANA1 16		// Numero Max sen Analogicas
#define NUMSENGRAY 28		// Numero Max Sen Grays 
#define NUMSENRS 16		// Numero max Sen RS232
#define NUMVALCONT 12		// 4 Contadores por 3 Cincomins = 12 Valores
#define NUMSENCONT 4		// 4 Señales Contadores
#define VALMAXIND 3999		// 3999 Valor Max del Indice de Incidencias = 4000 Incidencias
#define NUMINALM 4000		// s = 4000 Incidencias
#define MINBYTEMSG 10		// Valor Minimo de MSG en Recepcion ?????????????


typedef struct {	// Configuracion Remota
	char name[12];
	char desc[42];
	char ipname[16];
	char ipname1[16];
	char ipname2[16];
	short ihw;
	unsigned long segjulfun;
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
	}DIGCONF; 

typedef struct {		// Configuracion Registro Quinceminutal
	short Status;
	unsigned long SegJul;
	unsigned long SegJulPer;
	unsigned short SegJulCinc[3];
	unsigned short ValorCont[NUMVALCONT];
	unsigned short ValorAna[NUMSENANA1];
	unsigned short ValorGray[NUMSENGRAY];
	unsigned short ValorRs[NUMSENRS];
	short NumCont;
	short NumAna;
	short NumGray;
	short NumRs;
	unsigned char BufferQm[BYTEQM];
	unsigned short lBufferQm;
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
	unsigned long NumComSac[3];
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

typedef struct {		// Configuracion Remota
	char name[5];
	char desc[42];
	char ip[16];
	char ip1[16];
	char ip2[16];
	short ihw;
	unsigned long segjulfun;
	GN gn;
	}REMOTAS[NREMOTAS]; 


#define A1 0
#define A2 1
#define A3 2
#define A4 3
#define A5 4
#define G1 16
#define G2 17
#define G3 18
#define G4 19
#define G5 20
#define R1 44
#define R2 45
#define C1 60
#define C2 61
#define C3 62
#define C4 63
#
