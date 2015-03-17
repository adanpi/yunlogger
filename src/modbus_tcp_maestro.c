

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
#include "modbus_tcp.h"





/*********************************************************************

	modbus_response( response_data_array, query_array )

   Function to the correct response is returned and check for correctness.

   Returns:	string_length if OK
		0 if failed
		Less than 0 for exception errors

	Note: All functions used for sending or receiving data via
	      modbus return these return values.

**********************************************************************/

int modbus_response( unsigned char *data, unsigned char *query, int fd ,int rtu)
{
	int response_length;


	/* local declaration */
	int receive_response( unsigned char *received_string, int sfd);



	response_length = receive_response( data, fd );
	if( response_length > 0 && rtu==0)
	{

		/********** check for exception response *****/

		if( response_length && data[ 7 ] != query [ 7 ] )
		{

			/* return the exception value as a -ve number */
			response_length = 0 - data[ 8 ];
#ifdef DEBUG
	fprintf( stderr, "Excepcion %d.\n",response_length);
#endif
		}
	}
	else if(response_length > 0 && rtu==1){

		/********** check for exception response *****/

		if( response_length && data[ 0 ] != query [ 0 ] )
		{

			/* return the exception value as a -ve number */
			response_length = 0 - data[ 1 ];
#ifdef DEBUG
	fprintf( stderr, "Excepcion %d.\n",response_length);
#endif
		}
	}

#ifdef DEBUG
	fprintf( stderr, "\n modbus_response %d bytes.\n",response_length);
#endif

	return( response_length );
}








/***********************************************************************

	The following functions construct the required query into
	a modbus query packet.

***********************************************************************/

#define REQUEST_QUERY_SIZE 12	/* the following packets require          */
#define CHECKSUM_SIZE 2		/* 6 unsigned chars for the packet plus   */
				/* 2 for the checksum.                    */

void build_request_packet( int slave, int function, int start_addr,
			   int count, unsigned char *packet , int rtu)
{
	int i=0;

	if(rtu !=1)
		for( i = 0; i < 5 ; i++ ) packet[ i ] = 0;
	if(rtu !=1)
		packet[ i++ ] = 6;
	packet[ i++ ] = slave;
	packet[ i++ ] = function;
	//start_addr -= 1;
	packet[ i++ ] = start_addr >> 8;
	packet[ i++ ] = start_addr & 0x00ff;
	packet[ i++ ] = count >> 8;
	packet[ i ] = count &0x00ff;
	
	if(rtu==1){
		Mb_calcul_crc(packet,6, 0);
	}
}






/************************************************************************

	read_IO_status

	read_coil_stat_query and read_coil_stat_response interigate
	a modbus slave to get coil status. An array of coils shall be
	set to TRUE or FALSE according to the response from the slave.

*************************************************************************/

int read_IO_status( int function, int slave, int start_addr, int count,
			    int *dest, int dest_size, int sfd )
{
	/* local declaration */
	int read_IO_stat_response( int *dest, int dest_size, int coil_count,
					unsigned char *query, int fd );

	int status;


	unsigned char packet[ REQUEST_QUERY_SIZE + CHECKSUM_SIZE ];
	build_request_packet( slave, function, start_addr, count, packet ,0);

	if( send_query( sfd, packet, REQUEST_QUERY_SIZE ) > -1 )
	{
		status = read_IO_stat_response( dest, dest_size,
						count, packet, sfd );
	}
	else
	{
		status = SOCKET_FAILURE;
	}



	return( status );
}



/************************************************************************

	read_coil_status_tcp

	reads the boolean status of coils and sets the array elements
	in the destination to TRUE or FALSE

*************************************************************************/

int leer_estados_digitales_modbus( int slave, int start_addr, int count,
				int *dest, int dest_size, int sfd )
{
	int function = 0x01;
	int status;

	status = read_IO_status( function, slave, start_addr, count,
						dest, dest_size, sfd );

	return( status );
}





/************************************************************************

	read_input_status_tcp

	same as read_coil_status but reads the slaves input table.

************************************************************************/

int read_input_status_tcp( int slave, int start_addr, int count,
				int *dest, int dest_size, int sfd )
{
	int function = 0x02;	/* Function: Read Input Status */
	int status;

	status = read_IO_status( function, slave, start_addr, count,
						 dest, dest_size, sfd );



	return( status );
}




/**************************************************************************

	read_IO_stat_response

	this function does the work of setting array elements to TRUE
	or FALSE.

**************************************************************************/

int read_IO_stat_response( int *dest, int dest_size, int coil_count,
				unsigned char *query, int fd )
{

	unsigned char data[ MAX_RESPONSE_LENGTH ];
	int raw_response_length;
	int temp, i, bit, dest_pos = 0;
	int coils_processed = 0;

	raw_response_length = modbus_response( data, query, fd,0 );


	if( raw_response_length > 0 )
	{
		for( i = 0; i < ( data[8] ) && i < dest_size; i++ )
		{
			/* shift reg hi_byte to temp */
			temp = data[ 9 + i ] ;
			for( bit = 0x01; bit & 0xff &&
				coils_processed < coil_count; )
			{
				if( temp & bit )
				{
					dest[ dest_pos ] = TRUE;
				}
				else
				{
					dest[ dest_pos ] = FALSE;
				}
				coils_processed++;
				dest_pos++;
				bit = bit << 1;
			}
		}
	}

	return( raw_response_length );
}





/************************************************************************

	read_registers

	read the data from a modbus slave and put that data into an array.

************************************************************************/

int read_registers( int function, int slave, int start_addr, int count,
			  int *dest, int dest_size, int sfd, int rtu_over_tcp)
{
	/* local declaration */
	int read_reg_response( int *dest, int dest_size,
					unsigned char *query, int fd ,int rtu);

	int status;


	unsigned char packet[ REQUEST_QUERY_SIZE + CHECKSUM_SIZE ];
	build_request_packet( slave, function, start_addr, count, packet , rtu_over_tcp);

	int bytes_a_enviar=REQUEST_QUERY_SIZE;

	if(rtu_over_tcp)
		bytes_a_enviar=REQUEST_QUERY_SIZE-4;

	if( send_query( sfd, packet, bytes_a_enviar ) > -1 )
	{
		status = read_reg_response( dest, dest_size, packet, sfd ,rtu_over_tcp);
	}
	else
	{
		status = SOCKET_FAILURE;
	}

	return( status );

}



/************************************************************************

	read_holding_registers

	Read the holding registers in a slave and put the data into
	an array.

*************************************************************************/

int leer_registros_modbus( int slave, int start_addr, int count,
				int *dest, int dest_size, int sfd ,int rtu_over_tcp)
{
	int function = 0x03;    /* Function: Read Holding Registers */
	int status;

	if( count > MAX_READ_REGS )
	{
		count = MAX_READ_REGS;
#ifdef DEBUG
		fprintf( stderr, "Too many registers requested.\n" );
#endif
	}

	status = read_registers( function, slave, start_addr, count,
						dest, dest_size, sfd ,rtu_over_tcp);

#ifdef DEBUG
	fprintf( stderr, "\n leer_registros_modbus %d bytes.\n",status);
#endif

	// para RTU over TCP ignoramos errores anteriores
	if(rtu_over_tcp==1){
		
	}
	return( status);
}





/************************************************************************

	read_input_registers

	Read the inputg registers in a slave and put the data into
	an array.

*************************************************************************/

int read_input_registers_tcp( int slave, int start_addr, int count,
				int *dest, int dest_size, int sfd ,int rtu_over_tcp)
{
	int function = 0x04;	/* Function: Read Input Reqisters */
	int status;

	if( count > MAX_INPUT_REGS )
	{
		count =  MAX_INPUT_REGS;
#ifdef DEBUG
		fprintf( stderr, "Too many input registers requested.\n" );
#endif
	}

	status = read_registers( function, slave, start_addr, count,dest, dest_size, sfd,rtu_over_tcp );

	return( status );
}





/************************************************************************

	read_reg_response

	reads the response data from a slave and puts the data into an
	array.

************************************************************************/

int read_reg_response( int *dest, int dest_size, unsigned char *query, int fd ,int rtu)
{

	unsigned char data[ MAX_RESPONSE_LENGTH ];
	int raw_response_length;
	int temp,i;



	raw_response_length = modbus_response( data, query, fd ,rtu);
	if( raw_response_length > 0 )
		raw_response_length -= 2;


	if( raw_response_length > 0 && rtu==0)
	{
		for( i = 0;
			i < ( data[8] * 2 ) && i < (raw_response_length / 2);
									i++ )
		{
			/* shift reg hi_byte to temp */
			temp = data[ 9 + i *2 ] << 8;
			/* OR with lo_byte           */
			temp = temp | data[ 10 + i * 2 ];

			dest[i] = temp;
		}
	}else if ( raw_response_length > 0 && rtu==1){
		for( i = 0;i < ( data[2] * 2 ) ;i++ )
		{
			/* shift reg hi_byte to temp */
			temp = data[ 3 + i *2 ] << 8;
			/* OR with lo_byte           */
			temp = temp | data[ 4 + i * 2 ];

			dest[i] = temp;
		}
	}
#ifdef DEBUG
	fprintf( stderr, "\n read_reg_response %d bytes.\n",raw_response_length);
#endif
	return( raw_response_length );
}










/***********************************************************************

	preset_response

	Gets the raw data from the input stream.

***********************************************************************/

int preset_response( unsigned char *query, int fd )
{
	unsigned char data[ MAX_RESPONSE_LENGTH ];
	int raw_response_length;

	raw_response_length = modbus_response( data, query, fd ,0);

	return( raw_response_length );
}






/*************************************************************************

	set_single

	sends a value to a register in a slave.

**************************************************************************/

int set_single( int function, int slave, int addr, int value, int fd ,int rtu)
{

	int status;
	int i=0;

	unsigned char packet[ REQUEST_QUERY_SIZE ];

	if(rtu !=1)
		for( i = 0; i < 5 ; i++ ) packet[ i ] = 0;
	if(rtu !=1)
		packet[ i++ ] = 6;

//	for( i = 0; i < 5; i++ ) packet[ i ] = 0;
//	packet[ i++ ] = 6;
	packet[ i++ ] = slave;
	packet[ i++ ] = function;
	//addr -= 1;
	packet[ i++ ] = addr >> 8;
	packet[ i++ ] = addr & 0x00FF;
	packet[ i++ ] = value >> 8;
	packet[ i++ ] = value & 0x00FF;

	if(rtu==1){
		Mb_calcul_crc(packet,6, 0);
		i=i+2;
	}

	if( send_query( fd, packet, i ) > -1 )
	{
		status = preset_response( packet, fd );
	}
	else
	{
		status = SOCKET_FAILURE;
	}

	return( status );
}






/*************************************************************************

	force_single_coil

	turn on or off a single coil on the slave device

*************************************************************************/

int establecer_estado_digital( int slave, int coil_addr, int state, int fd ,int rtu)
{
	int function = 0x05;
	int status;

	if( state ) state = 0xFF00;
	coil_addr++;
	status = set_single( function, slave, coil_addr, state, fd ,rtu);

	return( status );
}





/*************************************************************************

	preset_single_register

	sets a value in one holding register in the slave device

*************************************************************************/

int escribir_registro_modbus(int slave, int reg_addr, int value, int fd , int rtu)
{
	int function = 0x06;
	int status;

	status = set_single( function, slave, reg_addr, value, fd ,rtu);

	return( status );
}





/************************************************************************

	set_multiple_coils

	Takes an array of ints and sets or resets the coils on a slave
	appropriatly.

*************************************************************************/


int establecer_estados_digitales( int slave, int start_addr, int coil_count,
				int *data, int fd )
{
	int byte_count;
	int i, bit, packet_size, data_legth;
	int coil_check = 0;
	int data_array_pos = 0;
	int status;

	unsigned char packet[ PRESET_QUERY_SIZE ];

	if( coil_count > MAX_WRITE_COILS )
	{
		coil_count = MAX_WRITE_COILS;
#ifdef DEBUG
		fprintf( stderr, "Writing to too many coils.\n" );
#endif
	}
	for( packet_size = 0; packet_size < 5; packet_size++ )
		packet[ packet_size ] = 0;
	packet[ packet_size++ ] = 6;
	packet[ packet_size++ ] = slave;
	packet[ packet_size++ ] = 0x0F;
	//start_addr -= 1;
	packet[ packet_size++ ] = start_addr >> 8;
	packet[ packet_size++ ] = start_addr & 0x00FF;
	packet[ packet_size++ ] = coil_count >> 8;
	packet[ packet_size++ ] = coil_count & 0x00FF;
	byte_count = (coil_count / 8) + 1;
	packet[ packet_size ] = byte_count;

	bit = 0x01;

	for( i = 0; i < byte_count; i++)
	{
		packet[ ++packet_size ] = 0;

		while( bit & 0xFF && coil_check++ < coil_count )
		{
			if( data[ data_array_pos++ ] )
			{
				packet[ packet_size ] |= bit;
			}
			else
			{
				packet[ packet_size ] &=~ bit;
			}
			bit = bit << 1;
		}
		bit = 0x01;
	}

	data_legth = packet_size - 5;
	packet[ 4 ] = data_legth >> 8;
	packet[ 5 ] = data_legth & 0x00FF;


	if( send_query( fd, packet, ++packet_size ) > -1 )
	{
		status = preset_response( packet, fd );
	}
	else
	{
		status = SOCKET_FAILURE;
	}

	return( status );
}

/************************************************************************

	set_multiple_coils

	Takes an array of ints and sets or resets the coils on a slave
	appropriatly.

*************************************************************************/



int establecer_estados_digitales_idata( int slave, int start_addr, int coil_count,
				unsigned char *data, int fd )
{
	int byte_count;
	int i, bit, packet_size, data_legth;
	int status;

	unsigned char packet[ PRESET_QUERY_SIZE ];

	if( coil_count > MAX_WRITE_COILS )
	{
		coil_count = MAX_WRITE_COILS;
#ifdef DEBUG
		fprintf( stderr, "Writing to too many coils.\n" );
#endif
	}
	for( packet_size = 0; packet_size < 5; packet_size++ )
		packet[ packet_size ] = 0;
	packet[ packet_size++ ] = 6;
	packet[ packet_size++ ] = slave;
	packet[ packet_size++ ] = 0x0F;
	//start_addr -= 1;
	packet[ packet_size++ ] = start_addr >> 8;
	packet[ packet_size++ ] = start_addr & 0x00FF;
	packet[ packet_size++ ] = coil_count >> 8;
	packet[ packet_size++ ] = coil_count & 0x00FF;
	byte_count = (coil_count / 8) + 1;
	packet[ packet_size ] = byte_count;

	bit = 0x01;

	for( i = 0; i < byte_count; i++)
	{
		packet[ ++packet_size ] = data[i];
	}

	data_legth = packet_size - 5;
	packet[ 4 ] = data_legth >> 8;
	packet[ 5 ] = data_legth & 0x00FF;


	if( send_query( fd, packet, ++packet_size ) > -1 )
	{
		status = preset_response( packet, fd );
	}
	else
	{
		status = SOCKET_FAILURE;
	}

	return( status );
}





/*************************************************************************

	preset_multiple_registers

	copy the values in an array to an array on the slave.

***************************************************************************/

int escribir_registros_modbus( int slave, int start_addr,
				    int reg_count, int *data, int fd )
{
	int byte_count, i, packet_size, data_legth;
	int status;

	unsigned char packet[ PRESET_QUERY_SIZE ];

	if( reg_count > MAX_WRITE_REGS )
	{
		reg_count = MAX_WRITE_REGS;
#ifdef DEBUG
		fprintf( stderr, "Trying to write to too many registers.\n" );
#endif
	}

	for( packet_size = 0; packet_size < 5; packet_size++ )
		packet[ packet_size ] = 0;
	packet[ packet_size++ ] = 6;
	packet[ packet_size++ ] = slave;
	packet[ packet_size++ ] = 0x10;
	//start_addr -= 1;
	packet[ packet_size++ ] = start_addr >> 8;
	packet[ packet_size++ ] = start_addr & 0x00FF;
	packet[ packet_size++ ] = reg_count >> 8;
	packet[ packet_size++ ] = reg_count & 0x00FF;
	byte_count = reg_count * 2;
	packet[ packet_size ] = byte_count;

	for( i = 0; i < reg_count; i++ )
	{
		packet[ ++packet_size ] = data[ i ] >> 8;
		packet[ ++packet_size ] = data[ i ] & 0x00FF;
	}

	data_legth = packet_size - 5;
	packet[ 4 ] = data_legth >> 8;
	packet[ 5 ] = data_legth & 0x00FF;

	if( send_query( fd, packet, ++packet_size ) > -1 )
	{
		status = preset_response( packet, fd );
	}
	else
	{
		status = SOCKET_FAILURE;
	}

	return( status );
}











int set_up_tcp( char *ip_address )
{
	int sfd;
	struct sockaddr_in server;
	int connect_stat;

	sfd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	server.sin_family = AF_INET;
	server.sin_port = htons( MODBUS_TCP_PORT );
	server.sin_addr.s_addr = inet_addr(ip_address);

	if( sfd >= 0 )
	{
		connect_stat = connect( sfd, (struct sockaddr *)&server,
						sizeof(struct sockaddr_in) );

		if( connect_stat < 0 )
		{
			//plc_log_errmsg( 0, "\nConnect - error %d\n",
			//				connect_stat );
			close( sfd );
			sfd = -1;
			// exit( connect_stat );
		}
	}

	return( sfd );
}

int set_up_tcp_port( char *ip_address , int port)
{
	int sfd;
	struct sockaddr_in server;
	int connect_stat;

	sfd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	server.sin_family = AF_INET;
	server.sin_port = htons( port );
	server.sin_addr.s_addr = inet_addr(ip_address);

	if( sfd >= 0 )
	{
		connect_stat = connect( sfd, (struct sockaddr *)&server,
						sizeof(struct sockaddr_in) );

		if( connect_stat < 0 )
		{
			fprintf( stderr, "\n error conexion %s [%d]\n",ip_address,port  );
			close( sfd );
			sfd = -1;
			// exit( connect_stat );
		}
	}

	return( sfd );
}


























