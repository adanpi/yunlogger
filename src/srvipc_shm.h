#define SHM_DATA_HIS 0
#define NSHMBUF 10	/* Tamano del buffer de datos */
#define NDAT 1440 	/* 15 Dias por 96 qm's */
#define SEGPQM 600

typedef struct {
	char remota[5];
	long seg[2][2];
	int sen[2];
	float num[2][NDAT];
	}DATA_HIS;

typedef struct {
	int flag[NSHMBUF+1];
	int pin;
	DATA_HIS data_his[NSHMBUF+1];
	}SHM_DATA;

typedef struct {
	int pin;
	DATA_HIS data_his[NSHMBUF+1];
	}SHM_DATA_1;

int shm_open();
int sem_open();
int shm_kill();
int sem_kill();
int io_in_shm_data();
int io_out_shm_data();
