#include <fcntl.h>	/* File control definitions */
#include <stdio.h>	/* Standard input/output */
#include <string.h>
#include <termio.h>	/* POSIX terminal control definitions */
#include <sys/time.h>	/* Time structures for select() */
#include <unistd.h>	/* POSIX Symbolic Constants */
#include <errno.h>	/* Error definitions */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "modbus.h"
#include "modbus_tcp.h"

/*#define DEBUG*/	/* uncomment to see the data sent and received */


/************************************************************************************
		Mb_test_crc : check the crc of a packet
 *************************************************************************************
input :
-------
trame  : packet with is crc
n      : lenght of the packet without tht crc
                              ^^^^^^^
answer :
--------
1 = crc fealure
0 = crc ok
 ************************************************************************************/
int Mb_test_crc(byte trame[],int n)
{
	unsigned int crc,i,j,carry_flag,a;
	crc=0xffff;
	for (i=0;i<n;i++)
	{
		crc=crc^trame[i];
		for (j=0;j<8;j++)
		{
			a=crc;
			carry_flag=a&0x0001;
			crc=crc>>1;
			if (carry_flag==1)
				crc=crc^0xa001;
		}
	}
	if (Mb_verbose)
		printf("test crc rtu %0x %0x\n",(crc&255),(crc>>8));
	if ((trame[n+1]!=(crc>>8)) || (trame[n]!=(crc&255)))
		return 1;
	else
		return 0;
}

int Mb_test_crc_tcp(byte trame[],int n)
{
	unsigned int crc,i,j,carry_flag,a;
	crc=0xffff;
	for (i=MODBUS_TCP_LON+0;i<MODBUS_TCP_LON+n;i++)
	{
		crc=crc^trame[i];
		for (j=0;j<8;j++)
		{
			a=crc;
			carry_flag=a&0x0001;
			crc=crc>>1;
			if (carry_flag==1)
				crc=crc^0xa001;
		}
	}
	if (Mb_verbose)
		printf("test crc tcp %0x %0x\n",(crc&255),(crc>>8));
	if ((trame[MODBUS_TCP_LON+n+1]!=(crc>>8)) || (trame[MODBUS_TCP_LON+n]!=(crc&255)))
		return 1;
	else
		return 0;
}

/************************************************************************************
		Mb_calcul_crc : compute the crc of a packet and put it at the end
 *************************************************************************************
input :
-------
trame  : packet with is crc
n      : lenght of the packet without tht crc
                              ^^^^^^^
answer :
--------
crc
 ************************************************************************************/
int Mb_calcul_crc(byte trame[],int n, short byte_ini)
{
	unsigned int crc,i,j,carry_flag,a;
	crc=0xffff;
	for (i=byte_ini+0;i<byte_ini+n;i++)
	{
		crc=crc^trame[i];
		for (j=0;j<8;j++)
		{
			a=crc;
			carry_flag=a&0x0001;
			crc=crc>>1;
			if (carry_flag==1)
				crc=crc^0xa001;
		}
	}
	trame[byte_ini+n+1]=crc>>8;
	trame[byte_ini+n]=crc&255;
	return crc;
}
/************************************************************************************
		Mb_close_device : Close the device
 *************************************************************************************
input :
-------
Mb_device : device descriptor

no output
 ************************************************************************************/
void Mb_close_device(int Mb_device)
{
	if (tcsetattr (Mb_device,TCSANOW,&saved_tty_parameters) < 0)
		perror("Can't restore terminal parameters ");
	close(Mb_device);
}

/************************************************************************************
		Mb_open_device : open the device
 *************************************************************************************
input :
-------
Mbc_port   : string with the device to open (/dev/ttyS0, /dev/ttyS1,...)
Mbc_speed  : speed (baudrate)
Mbc_parity : 0=don't use parity, 1=use parity EVEN, -1 use parity ODD
Mbc_bit_l  : number of data bits : 7 or 8 	USE EVERY TIME 8 DATA BITS
Mbc_bit_s  : number of stop bits : 1 or 2    ^^^^^^^^^^^^^^^^^^^^^^^^^^

answer  :
---------
device descriptor
 ************************************************************************************/
int Mb_open_device(char Mbc_port[20], int Mbc_speed, int Mbc_parity, int Mbc_bit_l, int Mbc_bit_s)
{
	int fd;

	/* open port */
	fd = open(Mbc_port,O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY) ;
	if(fd<0)
	{
		perror("Open device failure\n") ;
		exit(-1) ;
	}

	/* save olds settings port */
	if (tcgetattr (fd,&saved_tty_parameters) < 0)
	{
		perror("Can't get terminal parameters ");
		return -1 ;
	}

	/* settings port */
	bzero(&Mb_tio,sizeof(&Mb_tio));

	switch (Mbc_speed)
	{
	case 0:
		Mb_tio.c_cflag = B0;
		break;
	case 50:
		Mb_tio.c_cflag = B50;
		break;
	case 75:
		Mb_tio.c_cflag = B75;
		break;
	case 110:
		Mb_tio.c_cflag = B110;
		break;
	case 134:
		Mb_tio.c_cflag = B134;
		break;
	case 150:
		Mb_tio.c_cflag = B150;
		break;
	case 200:
		Mb_tio.c_cflag = B200;
		break;
	case 300:
		Mb_tio.c_cflag = B300;
		break;
	case 600:
		Mb_tio.c_cflag = B600;
		break;
	case 1200:
		Mb_tio.c_cflag = B1200;
		break;
	case 1800:
		Mb_tio.c_cflag = B1800;
		break;
	case 2400:
		Mb_tio.c_cflag = B2400;
		break;
	case 4800:
		Mb_tio.c_cflag = B4800;
		break;
	case 9600:
		Mb_tio.c_cflag = B9600;
		break;
	case 19200:
		Mb_tio.c_cflag = B19200;
		break;
	case 38400:
		Mb_tio.c_cflag = B38400;
		break;
	case 57600:
		Mb_tio.c_cflag = B57600;
		break;
	case 115200:
		Mb_tio.c_cflag = B115200;
		break;
	case 230400:
		Mb_tio.c_cflag = B230400;
		break;
	default:
		Mb_tio.c_cflag = B9600;
	}
	switch (Mbc_bit_l)
	{
	case 7:
		Mb_tio.c_cflag = Mb_tio.c_cflag | CS7;
		break;
	case 8:
	default:
		Mb_tio.c_cflag = Mb_tio.c_cflag | CS8;
		break;
	}
	switch (Mbc_parity)
	{
	case 1:
		Mb_tio.c_cflag = Mb_tio.c_cflag | PARENB;
		Mb_tio.c_iflag = ICRNL;
		break;
	case -1:
	Mb_tio.c_cflag = Mb_tio.c_cflag | PARENB | PARODD;
	Mb_tio.c_iflag = ICRNL;
	break;
	case 0:
	default:
		Mb_tio.c_iflag = IGNPAR | ICRNL;
		break;
	}

	if (Mbc_bit_s==2)
		Mb_tio.c_cflag = Mb_tio.c_cflag | CSTOPB;

	Mb_tio.c_cflag = Mb_tio.c_cflag | CLOCAL | CREAD;
	Mb_tio.c_oflag = 0;
	Mb_tio.c_lflag = 0; /*ICANON;*/
	Mb_tio.c_cc[VMIN]=1;
	Mb_tio.c_cc[VTIME]=0;

	/* clean port */
	tcflush(fd, TCIFLUSH);

	fcntl(fd, F_SETFL, FASYNC);
	/* activate the settings port */
	if (tcsetattr(fd,TCSANOW,&Mb_tio) <0)
	{
		perror("Can't set terminal parameters ");
		return -1 ;
	}

	/* clean I & O device */
	tcflush(fd,TCIOFLUSH);

	if (Mb_verbose)
	{
		printf("setting ok:\n");
		printf("device        %s\n",Mbc_port);
		printf("speed         %d\n",Mbc_speed);
		printf("data bits     %d\n",Mbc_bit_l);
		printf("stop bits     %d\n",Mbc_bit_s);
		printf("parity        %d\n",Mbc_parity);
	}
	return fd ;
}

/************************************************************************************
		Mb_rcv_print : print a character
This function can be use with slave or master to print a character when it receive one
 *************************************************************************************
input :
-------
c : character

no output
 ************************************************************************************/
void Mb_rcv_print(unsigned char c)
{
	printf("-> receiving byte :0x%x %d \n",c,c);
}

/************************************************************************************
		Mb_snd_print : print a character
This function can be use with slave or master to print a character when it send one
 *************************************************************************************
input :
-------
c : character

no output
 ************************************************************************************/
void Mb_snd_print(unsigned char c)
{
	printf("<- sending byte :0x%x %d \n",c,c);
}

char *Mb_version(void)
{
	return VERSION;
}

int set_up_tcp_slave( void )
{

	int sfd;
	struct sockaddr_in client;
	int bind_stat;
	int listen_stat;




	client.sin_family = AF_INET;
	client.sin_port = htons( MODBUS_TCP_PORT );
	client.sin_addr.s_addr = INADDR_ANY;

	sfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if( sfd >= 0 )
	{

		bind_stat = bind( sfd, &client, sizeof( client ) );
		if( bind_stat < 0 )
		{
			//plc_log_errmsg( 0,"bind failed %s\n",
					//			strerror( errno) );
			close( sfd );
			sfd = -1;
			//exit( bind_stat );
		}
	}

	if( sfd >= 0 )
	{
		listen_stat = listen( sfd, 2 );
		if( listen_stat != 0 )
		{
			//plc_log_errmsg( "listen FAILED with %s\n",
			//			strerror( errno ) );
			close( sfd );
			sfd = -2;
			if( listen_stat > 0 ) sfd = -3;
			//exit(listen_stat);
		}


/*		if( listen_stat == 0 )
		{
			accept_stat = accept( sfd, NULL, NULL );



			if( accept_stat < 0 )
			{
				//plc_log_errmsg( 0, "\nConnect - error %d\n",
						//                              connect_stat );

				close( sfd );
				sfd = -4;
				//exit( accept_stat );
			}
		}*/
	}
	return( sfd );
}

void Mb_close_socket(int sfd){
	close(sfd);
}

/***********************************************************************

	receive_response( array_for_data )

   Function to monitor for the reply from the modbus slave.
   This function blocks for timeout seconds if there is no reply.

   Returns:	Total number of characters received.
 ***********************************************************************/

int receive_response(  unsigned char *received_string, int sfd )
{

	int rxchar = -1;
	int data_avail = FALSE;
	int bytes_received = 0;
	int read_stat;

	int timeout = TIMEOUT;			/* TIMEOUT second */
	int char_interval_timeout = 10000; 	/* 10 milliseconds. */

	//int max_fds = 32;
	int max_fds =sfd+1;	/* Solo escuchamos en un socket*/
	fd_set rfds;

	struct timeval tv;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	FD_ZERO( &rfds );
	FD_SET( sfd, &rfds );

	if (Mb_verbose)
		fprintf( stderr, "\n\tEscuchando en socket: %d (%d seg)\n",sfd,timeout);

#ifdef DEBUG
	fprintf( stderr, "Recibido:\n");
#endif

	/* wait for a response */
	data_avail = select( max_fds, &rfds, NULL, NULL, &tv );


	if( !data_avail )
	{
		bytes_received = 0;
#ifdef DEBUG
		fprintf( stderr, "Comms time out\n" );
#endif
	}

	tv.tv_sec = 0;
	tv.tv_usec = char_interval_timeout;

	FD_ZERO( &rfds );
	FD_SET( sfd, &rfds );

	while( data_avail )
	{

		/* if no character at the buffer wait char_interval_timeout */
		/* before accepting end of response			    */

		if( select( max_fds, &rfds, NULL, NULL, &tv) > 0 )
		{
			read_stat = recv( sfd, &rxchar, 1, 0);

			if( read_stat < 0 )
			{
				bytes_received = SOCKET_FAILURE;
				data_avail = FALSE;
			}
			else
			{

				rxchar = rxchar & 0xFF;
				received_string[ bytes_received ++ ] = rxchar;
			}


			if( bytes_received >= MAX_RESPONSE_LENGTH )
			{
				bytes_received = SOCKET_FAILURE;
				data_avail = FALSE;
			}
#ifdef DEBUG
			/* display the hex code of each character received */
			fprintf( stderr, "<%02X>", rxchar );
#endif


		}
		else
		{
			data_avail = FALSE;
		}

	}

#ifdef DEBUG
	fprintf( stderr, "Fin recepcion respuesta, recibidos %d bytes\n" ,bytes_received);
#endif

	if( bytes_received > 7 )
	{
		bytes_received -= 7;
	}


	return( bytes_received );
}

int get_slave_query( unsigned char *byte1, unsigned char *byte2, int *slave_addr, int *query, int *start_addr,
		int *point_count, int *data, int sfd )
{

	unsigned char raw_data[ MAX_RESPONSE_LENGTH ];
	int response_length;
	int i,indice=6,NumReg=0;


	response_length = receive_response( raw_data, sfd );


	if( response_length > 0 )
	{
		*byte1=raw_data[ 0 ];
		*byte2=raw_data[ 1 ];
		*slave_addr = raw_data[ indice++ ];
		*query = raw_data[ indice++ ];
		*start_addr = raw_data[ indice++ ] << 8;
		*start_addr = *start_addr | raw_data[ indice++ ];
		if(*query==5){
			data[0]=raw_data[ indice ]  + raw_data[ indice+1 ];
			return 1;
		}
		if(*query==15){
			*point_count = raw_data[ indice++ ] << 8;
			*point_count = *point_count | raw_data[ indice++ ];
			NumReg=raw_data[indice++];
			for( i = 0;	i < NumReg;	i++ )
					data[i]=raw_data[ indice +i];

			return NumReg;
		}
		if( (*query!=6 ) ){
			*point_count = raw_data[ indice++ ] << 8;
			*point_count = *point_count | raw_data[ indice++ ];
		}

		if ( (*query==3 ) || (*query==4 ) ){
			return 1;
		}
		if( (*query==16 ) ){
			//NumReg=raw_data[12]/2;		// un unico byte, por tanto el maximo numero de registros a establecer es de 254/2=127
			NumReg=*point_count;			// de esta manera no limitamos el numero de registros a 127
			indice++;
		}else{
			NumReg=(raw_data[5]-4)/2;
		}
		for( i = 0;
		i < NumReg;
		i++ )
		{
			data[i]=(raw_data[ indice ] << 8) + raw_data[ indice+1 ];

#ifdef DEBUG
			/* display the hex code of each character received */
			fprintf( stderr, "\n [%d] <%02X> <%02X>", data[i], raw_data[ indice ],raw_data[ indice+1 ] );
#endif
			indice=indice+2;
		}
#ifdef DEBUG
			/* Numero de registros */
			fprintf( stderr, "\n NumReg: %d \t indice: %d\n", NumReg, indice );
#endif


	}
	tcflush( sfd, TCIFLUSH );
	return( NumReg );

}

/***********************************************************************

   send_query( file_descriptor, query_string, query_length )

Function to send a query out to a modbus slave.
************************************************************************/

int send_query( int sfd, unsigned char *query, size_t string_length )
{
	int write_stat;

#ifdef DEBUG
	int i;
#endif


#ifdef DEBUG
/* Print to stderr the hex value of each character that is about to be */
/* sent to the modbus slave.					       */

	for( i = 0; i < string_length; i++ )
	{
		fprintf( stderr, "[%02X]", query[ i ] );
	}
	fprintf( stderr, "\n" );
#endif

	tcflush( sfd, TCIOFLUSH );	/* flush the input & output streams */

	write_stat = send( sfd, query, string_length, 0);

	tcflush( sfd, TCIFLUSH );	/* maybe not neccesary */

	return( write_stat );
}

void ImprimirBits(unsigned char byte){
	int j;
	for (j=7;j>=0;j--)
		fprintf(stderr,"%d",byte>>j & 1);
	fprintf(stderr,"\n");
}

/* se entrega al planificador de manera probabilistica */
void yield(void)
{
	if (rand()%2)
		sched_yield();
}

/* Funcion que toma un float y lo despieza en sus bytes para almacenar en bbdd modbus (2 posiciones)
 * isBigEndian: para axis = 1 para PC =0 (littleEndian)
 * devuelve los bytes
 */

void Float2Bytes(int isBigEndian,float anotherFloat,unsigned char bytes[sizeof(float)]){

	float aFloat = anotherFloat; // Could be a double instead
	//unsigned char bytes[sizeof(float)];
	unsigned char *byte = (unsigned char *) &aFloat;
	int i=0;
	if(isBigEndian)
	{
		for (i=0; i<sizeof(float); i++)
		{
			bytes[i] = *byte;
			byte++;
		}
	}
	else
	{
		for (i=sizeof(float)-1; i>=0; i--)
		{
			bytes[i] = *byte;
			byte++;
		}
	}
}

