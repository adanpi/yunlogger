/* 		modbus_tcp.h

   By P.Costigan email: phil@pcscada.com.au http://pcscada.com.au
 
   These library of functions are designed to enable a program send and
   receive data from a device that communicates using the Modbus tcp protocol.
 
   Copyright (C) 2000 Philip Costigan  P.C. SCADA LINK PTY. LTD.
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                    




   The functions included here have been derived from the
   Modicon Modbus Protocol and Modbus TCP Protocol Reference Guides
   which can be obtained from Schneider at www.schneiderautomation.com.
 
   This code has its origins with
   paul@pmcrae.freeserve.co.uk (http://www.pmcrae.freeserve.co.uk)
   who wrote a small program to read 100 registers from a modbus slave.
 
   I have used his code as a catalist to produce this more functional set
   of functions. Thanks paul.


 */

#define NUMIPAXIS "127.0.0.1"
#define ISBIGENDIAN 0	//IsBigEndian=1 en Axis
#define DEBUG	// descomentar para ver bytes comunicaciones
#define MODBUS_TCP_PORT 502
#define TIMEOUT 30      /* Tiempo de vida de los socket establecidos si no hay transferencia de datos*/

#ifndef MODBUSTCP_H
#define MODBUSTCP_H

#define MAX_DATA_LENGTH 500
#define MAX_QUERY_LENGTH 512
#define MAX_RESPONSE_LENGTH 1024
#define PRESET_QUERY_SIZE 512

#define FALSE 0 
#define TRUE 1



/***********************************************************************

	 Note: All functions used for sending or receiving data via
	       modbus return these return values.


	Returns:	string_length if OK
			0 if failed
			Less than 0 for exception errors

***********************************************************************/

#define COMMS_FAILURE 0
#define ILLEGAL_FUNCTION -1
#define ILLEGAL_DATA_ADDRESS -2
#define ILLEGAL_DATA_VALUE -3
#define SLAVE_DEVICE_FAILURE -4
#define ACKNOWLEDGE -5
#define SLAVE_DEVICE_BUSY -6
#define NEGATIVE_ACKNOWLEDGE -7
#define MEMORY_PARITY_ERROR -8

#define SOCKET_FAILURE -11



/************************************************************************

	read_coil_status_tcp()

	reads the boolean status of coils and sets the array elements
	in the destination to TRUE or FALSE.

*************************************************************************/

int leer_estados_digitales_modbus( int slave, int start_addr, int count,
				int *dest, int dest_size, int fd );





/************************************************************************

	read_input_status_tcp()

	same as read_coil_status but reads the slaves input table.

************************************************************************/

int read_input_status_tcp( int slave, int start_addr, int count,
				int *dest, int dest_size, int fd );





/***********************************************************************

	read_holding_registers_tcp()

	Read the holding registers in a slave and put the data into
	an array.

************************************************************************/

#define MAX_READ_REGS 240

int leer_registros_modbus( int slave, int start_addr, int count, 
			  	int *dest, int dest_size, int fd );





/***********************************************************************

	read_input_registers_tcp()

	Read the inputg registers in a slave and put the data into
	an array.

***********************************************************************/

#define MAX_INPUT_REGS 240

int read_input_registers_tcp( int slave, int start_addr, int count,
				int *dest, int dest_size, int fd );







/************************************************************************

	force_single_coil()

	turn on or off a single coil on the slave device.

************************************************************************/

int establecer_estado_digital( int slave, int addr, int state, int fd );






/*************************************************************************

	preset_single_register_tcp()

	sets a value in one holding register in the slave device.

*************************************************************************/

int escribir_registro_modbus( int slave, int reg_addr, int value, int fd );






/*************************************************************************

	set_multiple_coils_tcp()

	Takes an array of ints and sets or resets the coils on a slave
	appropriatly.

**************************************************************************/

#define MAX_WRITE_COILS 800

int establecer_estados_digitales( int slave, int start_addr, 
				int coil_count, int *data, int fd );


int establecer_estados_digitales_idata( int slave, int start_addr, 
				int coil_count, unsigned char *data, int fd );



/*************************************************************************

	preset_multiple_registers_tcp()

	copy the values in an array to an array on the slave.

*************************************************************************/

#define MAX_WRITE_REGS 240

int escribir_registros_modbus( int slave, int start_addr,
					int reg_count, int *data, int fd );









/*************************************************************************

	set_up_tcp

	This function sets up a tcp socket for modbus/tcp communications

**************************************************************************/

int set_up_tcp( char *ip_address );






/************************************************************************
*************************************************************************
*************************************************************************

   THE FOLLOWING SECTION IS DEDICATED TO MODBUS TCP SLAVE.

*************************************************************************
*************************************************************************
************************************************************************/




/************************************************************************

	get_slave_query()

	Find out what a master is trying to ask this slave device.

************************************************************************/

int get_slave_query( unsigned char *byte1, unsigned char *byte2, int *slave_addr, int *query, int *start_addr,
                int *point_count, int *data, int sfd );

/************************************************************************

	set_up_tcp_slave()

	This function sets up a tcp socket for modbus/tcp slave
	communications.

************************************************************************/

int set_up_tcp_slave( void );

void EnvioModBusIn(unsigned long segjulact,unsigned short NumSen,unsigned short Estado);
void EnvioModBus( void );
void CopiaBdRemotaModBus( void );











#endif  /* MODBUSTCP_H */





