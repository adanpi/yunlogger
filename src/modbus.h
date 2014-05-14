/*
#ifndef __modbus__

#define __modbus__ 1
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>

#define VERSION "1.0"
#define MODBUS_TCP_LON 6
//#define DEBUG 
#define BBDDMODBUSLON 5012	// Tamaño BBDD ModBus ( 4992/2= 2496 Valores Word Analogicas + 20*8=160 Estados Bit Digitales)
//#define BBDDMODBUSLON 73727	// Tamaño BBDD ModBus ( 65535 Registros + 65535/8 Estados Bit Digitales)
#define POSDIGMODBUS 4992	// Posicion Comienzo Estado Digitales
//#define POSDIGMODBUS 65535	// Posicion Comienzo Estado Digitales

#define ESCLAVO 1	// Numero esclavo modbus

#define PUERTOSERIE "/dev/ttyS0"
#define VELOCIDAD 9600
#define PARIDAD 0
#define BITDATOS 8
#define BITSTOP 1

#define REFRESCO_HORA 100	// refresco modbus hora en decimas de segundo
#define TIMEOUT_MBS_RTU 3	// timeout segundos entre recepcion caracteres rtu esclavo

#define ISBIGENDIAN 1	//IsBigEndian=1 en Axis

struct termios saved_tty_parameters;			/* old serial port setting (restored on close) */
struct termios Mb_tio;					/* new serail port setting */

pthread_t Mbs_thread_rtu,Mbs_thread_tcp,Mbs_thread_hora;

int Mb_verbose;										/* print debug informations */
int Mb_status;											/* stat of the software : This number is free, it's use with function #07 */


unsigned short *Mbs_data;								/* slave modbus data */
int Mbs_pid;											/* PID of the slave thread */

typedef unsigned char byte;						/* create byte type */

/* master structure */
typedef struct {
   int device;											/* modbus device (serial port: /dev/ttyS0 ...) */
   int slave; 											/* number of the slave to call*/
   int function; 										/* modbus function to emit*/
   int address;										/* slave address */
   int length;											/* data length */
   int timeout;										/* timeout in ms */
} Mbm_trame;

/*pointer functions */
void (*Mb_ptr_rcv_data) ();						/* run when receive a char in master or slave */
void (*Mb_ptr_snd_data) ();						/* run when send a char  in master or slave */
void (*Mb_ptr_end_slve) ();						/* run when slave finish to send response trame */

/* master main function :
- trame informations
- data in
- data out
- pointer function called when master send a data on serial port (can be NULL if not use)
- pointer function called when master receive a data on serial port (can be NULL if not use)*/
int Mb_master(Mbm_trame, int [] , int [], void*, void*);

/* slave main function (start slave thread) :
- device
- slave number
- pointer function called when slave send a data on serial port (can be NULL if not use)
- pointer function called when slave receive a data on serial port (can be NULL if not use)
- pointer function called when slave finish to send data on serial port (can be NULL if not use)*/
void Mb_slave_rtu(int, int, void*, void*, void*);

void Mb_slave_tcp(int, int);

void Mb_slave_hora();

void Mb_slave_rtu_stop(void);											/* stop slave thread */

void Mb_slave_tcp_stop(void);

/* commun functions */
int Mb_open_device(char [], int , int , int ,int );		/* open device and configure it */
void Mb_close_device();												/* close device and restore old parmeters */
void Mb_close_socket(int sfd);
int Mb_test_crc(unsigned char[] ,int);						/* check crc16 */
int Mb_test_crc_tcp(unsigned char[] ,int);
int Mb_calcul_crc(unsigned char[] ,int, short);					/* compute crc16 */

void Mb_rcv_print(unsigned char);				/* print a char (can be call by master or slave with Mb_ptr_rcv_data)*/
void Mb_snd_print(unsigned char);				/* print a char (can be call by master or slave with Mb_ptr_rcv_data)*/
char *Mb_version(void);								/* return libmodbus version */
void ImprimirBits(unsigned char);
void yield(void);
void Mb_slave_init(void);
void Mb_slave_stop(void);
void Mb_slave_join_threads(void);
void Float2Bytes(int isBigEndian,float anotherFloat,unsigned char bytes[sizeof(float)]);
void Mb_slave_recibido_char(unsigned char *c);
void Mb_slave_fin_envio(unsigned char *c);
