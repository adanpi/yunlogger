/*!**************************************************************************
*! Revision 1.3  01/03/05 Adan Pineiro@M.Bibudis
*! Saih Linux Axis RS232 Remota Loger DataLoger 
*!
*!                                                            
*! FILE NAME  : hwtestserial.c
*!                                                            
*! DESCRIPTION: Tests the serial port(s) and other I/O
*!
*!---------------------------------------------------------------------------
*! HISTORY                                                    
*!
*! $Log: hwtestserial.c,v $
*! Revision 1.23  2002/11/06 12:11:58  magnusmn
*! Adjustments to the -loopback option.
*! * The read call won't hang if there is no connection between rx and tx. An error is returned instead.
*! * Added a rxtx option to -loopback so that rx to tx loopback can be tested ignoring the flow controll signals.
*!
*! Revision 1.22  2002/11/04 18:10:57  johana
*! Added -flow option to set flow control: none, x, -x, rtscts, -rtscts.
*! -rx commands support '+' in front of time meaning that the reception
*! is really only time seconds, and not stop rx after idle in time seconds.
*! Added -dnbnoctty that opens the port with O_RDWR|O_NONBLOCK|O_NOCTTY
*!
*! Revision 1.21  2002/10/09 13:35:34  johana
*! Added getinfospeedtest and changed CONFIG_ETRAX100_ to CONFIG_ETRAX_.
*!
*! Revision 1.20  2001/12/14 17:24:00  martinnn
*! Added cache-control http headers to prevent test results to be cached.
*!
*! Revision 1.19  2001/10/03 13:20:14  johana
*! Added specialbaud command (-sb baudbase/divisor) to set non standard
*! baudrates.
*! elinux supports using the prescaler: 3125000/divisor or extern baudrate/1
*! if it's run on an ETRAX100LX.
*!
*! Revision 1.18  2001/05/29 15:37:24  sebsjo
*! Made them compile with uC-libc Beta2.
*!
*! Revision 1.17  2001/03/13 08:43:44  johana
*! Grr.. finally fixed ETRAXPARIO_TEST stuff
*!
*! Revision 1.16  2001/02/28 16:32:59  johana
*! Oups, fixed ETRAXPARIO_TEST stuff.
*!
*! Revision 1.15  2001/02/28 15:38:55  johana
*! Added -rxhex command and handle break (SIGINT)
*!
*! Revision 1.14  2001/02/28 15:37:12  johana
*! Compile with Linux 2.4 even though some stuff are missing.
*!
*! Revision 1.13  2001/02/07 08:52:22  johana
*! Added -break and -txhex commands.
*!
*! Revision 1.12  2001/01/16 11:54:07  johana
*! Added print_cflag_fields()
*!
*! Revision 1.11  2000/11/01 13:48:57  johana
*! Added rx485 and tx485 commands
*!
*! Revision 1.10  2000/10/17 17:52:22  johana
*! Cleaned up, removed warnings. Added -rxdbg option that shows pin status
*! and allows terminal input as well.
*!
*! Revision 1.9  2000/07/05 10:53:45  tobiasa
*! flush fd and small fix in printout
*!
*! Revision 1.8  2000/07/05 07:35:41  tobiasa
*! added -loopback <times> option for doing serial loopback test.
*!
*! Revision 1.7  2000/05/25 16:30:11  johana
*! Added -raw, -male and -female commands
*!
*! Revision 1.6  2000/04/26 14:25:33  johana
*! Use text/plain and skip the HTML tags
*!
*! Revision 1.5  2000/04/26 10:49:09  johana
*! Detect if executed as CGI and deal with it
*!
*! Revision 1.4  2000/03/27 17:42:48  johana
*! Started on serial port production test (check in before the move)
*!
*! Revision 1.3  2000/02/02 16:01:41  johana
*! Parallell port control as well - time to change name?...
*!
*! Revision 1.2  2000/01/24 18:38:58  johana
*! Added following features:
*!   -v level      How much debug info we should print
*!   -d device     What device to use, e.g. /dev/ttyS0
*!   -dnb device   Open device with O_NONBLOCK
*!   -rx time      rx-mode in time seconds
*!   -txtest size  Send size byes of testpattern
*!   -w time      Wait time: T seconds, Tms Tus
*!   -nohup        Don't hang up (RTS/DTR) when closing.
*!
*! Revision 1.1  2000/01/20 18:17:34  johana
*! Serial port test routines
*!
*! Revision 1.1  2000/01/17 17:54:37  johana
*! initial version
*!
*!
*!---------------------------------------------------------------------------
*!
*! (C) Copyright 2000, Axis Communications AB, LUND, SWEDEN
*!
*! $Id: hwtestserial.c,v 1.23 2002/11/06 12:11:58 magnusmn Exp $
*!
*!**************************************************************************/

/********************** INCLUDE FILES SECTION ******************************/

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>

#include <sys/time.h>
#include <sys/types.h>
#include <linux/serial.h>
#include "serutil.h"

#include "serutil.c"

#include "axisloger.h"
#include "ipcserver.h"

#include <sys/ioctl.h>

#ifdef __CRIS__
#include <linux/version.h>
#if LINUX_VERSION_CODE >= 0x020400
/* New kernel has the special files in asm */
#include <linux/types.h>
#include <asm/etraxgpio.h>
#include <asm/svinto.h>
/*#include <asm/etraxpario.h>*/
#warning "#### TODO: Fix ETRAXPARIO ####"
#define ETRAXPARIO_TEST
#ifndef TIOCSERSETRS485
#warning "#### TODO: WARNING: No RS485 support in kernel ####"
#endif

#else
/* Old elinux has the special files in linux */
#include <linux/etraxgpio.h>
#include <asm/svinto.h>
#include <linux/etraxpario.h>
#undef ETRAXPARIO_TEST
#endif
#else
/* Not __CRIS__ */
#define ETRAXPARIO_TEST
#endif



#define VERSION "1.0"
// #define DEBUG
#ifdef DEBUG
#ifdef __CRIS__
/* for glibc debugging */
typedef void (*bozo)(char *buf);
extern bozo console_print_etrax; /* from crt0.o */
#define D(x) 
#else
#define console_print_etrax(x) { printf( "%s", x); fflush(NULL); }
#define D(x) x
#endif
#else
#define console_print_etrax(x) 
#define D(x) 
#endif

/********************** DOCUMENTATION SECTION ******************************/
/*
Productio test loopback testing of Bundy (2490 and Developer board)
Anders Sjö and Johan Adolfsson

com1   = ser0 /dev/ttyS0
com2   = ser2 /dev/ttyS2
debug  = ser1 /dev/ttyS1
com422 = ser3 /dev/ttyS3

UNPLUGGED
-----------------------------
com1,2
------
com1-DTR 	    -> com1-CD 
com2-CD_OUT 	-> com1-RXD  * NOTE CD on COM2 set to output
com1-TXD	-> 
com1-RTS	-> com1-DSR
		-> com1-CTS
		-> com1-RI

com2-DTR	-> com2-CD
com2-RI_OUT	-> com2-RXD    * NOTE RI on COM2 set to output
com2-TXD	->
com2-RTS	-> com2-DSR
		-> com2-CTS
		-> com2-RI

Debug
-----
	-> RXD
	-> CTS
TXD	->
RTS	->

com422
------
TX-A	-> RX-A
TX+B	-> RX+B


PLUGGED
------------------------------
COM1,2
----
	-> CD
TXD	-> RXD
DTR	-> RI
	-> DSR
CTS 	-> RTS
CD_OUT	->
RI_OUT	->

DEBUG
-----
TXD 	-> RXD
CTS	-> RTS

COM422
------
RX-A	-> fixtur
RX+B	-> fixtur
TX-A	->
TX+B	->

When changing outputs, all inputs should be checked so that shortcuts are 
detected. 

*/


/********************** CONSTANT AND MACRO SECTION *************************/




#ifndef __CRIS__
/* Have these values in host mode as well to be able to test program better */
#define R_PAR0_CTRL_DATA (IO_TYPECAST_UDWORD 0xb0000040)
#define R_PAR0_CTRL_DATA__peri_int__BITNR 24
#define R_PAR0_CTRL_DATA__peri_int__WIDTH 1
#define R_PAR0_CTRL_DATA__peri_int__ack 1
#define R_PAR0_CTRL_DATA__peri_int__nop 0
#define R_PAR0_CTRL_DATA__oe__BITNR 20
#define R_PAR0_CTRL_DATA__oe__WIDTH 1
#define R_PAR0_CTRL_DATA__oe__enable 1
#define R_PAR0_CTRL_DATA__oe__disable 0
#define R_PAR0_CTRL_DATA__seli__BITNR 19
#define R_PAR0_CTRL_DATA__seli__WIDTH 1
#define R_PAR0_CTRL_DATA__seli__active 1
#define R_PAR0_CTRL_DATA__seli__inactive 0
#define R_PAR0_CTRL_DATA__autofd__BITNR 18
#define R_PAR0_CTRL_DATA__autofd__WIDTH 1
#define R_PAR0_CTRL_DATA__autofd__active 1
#define R_PAR0_CTRL_DATA__autofd__inactive 0
#define R_PAR0_CTRL_DATA__strb__BITNR 17
#define R_PAR0_CTRL_DATA__strb__WIDTH 1
#define R_PAR0_CTRL_DATA__strb__active 1
#define R_PAR0_CTRL_DATA__strb__inactive 0
#define R_PAR0_CTRL_DATA__init__BITNR 16
#define R_PAR0_CTRL_DATA__init__WIDTH 1
#define R_PAR0_CTRL_DATA__init__active 1
#define R_PAR0_CTRL_DATA__init__inactive 0
#define R_PAR0_CTRL_DATA__ecp_cmd__BITNR 8
#define R_PAR0_CTRL_DATA__ecp_cmd__WIDTH 1
#define R_PAR0_CTRL_DATA__ecp_cmd__command 1
#define R_PAR0_CTRL_DATA__ecp_cmd__data 0
#define R_PAR0_CTRL_DATA__data__BITNR 0
#define R_PAR0_CTRL_DATA__data__WIDTH 8


#define R_PAR0_STATUS_DATA (IO_TYPECAST_RO_UDWORD 0xb0000040)
#define R_PAR0_STATUS_DATA__mode__BITNR 29
#define R_PAR0_STATUS_DATA__mode__WIDTH 3
#define R_PAR0_STATUS_DATA__mode__manual 0
#define R_PAR0_STATUS_DATA__mode__centronics 1
#define R_PAR0_STATUS_DATA__mode__fastbyte 2
#define R_PAR0_STATUS_DATA__mode__nibble 3
#define R_PAR0_STATUS_DATA__mode__byte 4
#define R_PAR0_STATUS_DATA__mode__ecp_fwd 5
#define R_PAR0_STATUS_DATA__mode__ecp_rev 6
#define R_PAR0_STATUS_DATA__mode__off 7
#define R_PAR0_STATUS_DATA__perr__BITNR 28
#define R_PAR0_STATUS_DATA__perr__WIDTH 1
#define R_PAR0_STATUS_DATA__perr__active 1
#define R_PAR0_STATUS_DATA__perr__inactive 0
#define R_PAR0_STATUS_DATA__ack__BITNR 27
#define R_PAR0_STATUS_DATA__ack__WIDTH 1
#define R_PAR0_STATUS_DATA__ack__active 0
#define R_PAR0_STATUS_DATA__ack__inactive 1
#define R_PAR0_STATUS_DATA__busy__BITNR 26
#define R_PAR0_STATUS_DATA__busy__WIDTH 1
#define R_PAR0_STATUS_DATA__busy__active 1
#define R_PAR0_STATUS_DATA__busy__inactive 0
#define R_PAR0_STATUS_DATA__fault__BITNR 25
#define R_PAR0_STATUS_DATA__fault__WIDTH 1
#define R_PAR0_STATUS_DATA__fault__active 0
#define R_PAR0_STATUS_DATA__fault__inactive 1
#define R_PAR0_STATUS_DATA__sel__BITNR 24
#define R_PAR0_STATUS_DATA__sel__WIDTH 1
#define R_PAR0_STATUS_DATA__sel__active 1
#define R_PAR0_STATUS_DATA__sel__inactive 0
#define R_PAR0_STATUS_DATA__tr_rdy__BITNR 17
#define R_PAR0_STATUS_DATA__tr_rdy__WIDTH 1
#define R_PAR0_STATUS_DATA__tr_rdy__ready 1
#define R_PAR0_STATUS_DATA__tr_rdy__busy 0
#define R_PAR0_STATUS_DATA__dav__BITNR 16
#define R_PAR0_STATUS_DATA__dav__WIDTH 1
#define R_PAR0_STATUS_DATA__dav__data 1
#define R_PAR0_STATUS_DATA__dav__nodata 0
#define R_PAR0_STATUS_DATA__ecp_cmd__BITNR 8
#define R_PAR0_STATUS_DATA__ecp_cmd__WIDTH 1
#define R_PAR0_STATUS_DATA__ecp_cmd__command 1
#define R_PAR0_STATUS_DATA__ecp_cmd__data 0
#define R_PAR0_STATUS_DATA__data__BITNR 0
#define R_PAR0_STATUS_DATA__data__WIDTH 8


#define R_PAR0_CONFIG (IO_TYPECAST_UDWORD 0xb0000044)
#define R_PAR0_CONFIG__ioe__BITNR 25
#define R_PAR0_CONFIG__ioe__WIDTH 1
#define R_PAR0_CONFIG__ioe__inv 1
#define R_PAR0_CONFIG__ioe__noninv 0
#define R_PAR0_CONFIG__iseli__BITNR 24
#define R_PAR0_CONFIG__iseli__WIDTH 1
#define R_PAR0_CONFIG__iseli__inv 1
#define R_PAR0_CONFIG__iseli__noninv 0
#define R_PAR0_CONFIG__iautofd__BITNR 23
#define R_PAR0_CONFIG__iautofd__WIDTH 1
#define R_PAR0_CONFIG__iautofd__inv 1
#define R_PAR0_CONFIG__iautofd__noninv 0
#define R_PAR0_CONFIG__istrb__BITNR 22
#define R_PAR0_CONFIG__istrb__WIDTH 1
#define R_PAR0_CONFIG__istrb__inv 1
#define R_PAR0_CONFIG__istrb__noninv 0
#define R_PAR0_CONFIG__iinit__BITNR 21
#define R_PAR0_CONFIG__iinit__WIDTH 1
#define R_PAR0_CONFIG__iinit__inv 1
#define R_PAR0_CONFIG__iinit__noninv 0
#define R_PAR0_CONFIG__iperr__BITNR 20
#define R_PAR0_CONFIG__iperr__WIDTH 1
#define R_PAR0_CONFIG__iperr__inv 1
#define R_PAR0_CONFIG__iperr__noninv 0
#define R_PAR0_CONFIG__iack__BITNR 19
#define R_PAR0_CONFIG__iack__WIDTH 1
#define R_PAR0_CONFIG__iack__inv 1
#define R_PAR0_CONFIG__iack__noninv 0
#define R_PAR0_CONFIG__ibusy__BITNR 18
#define R_PAR0_CONFIG__ibusy__WIDTH 1
#define R_PAR0_CONFIG__ibusy__inv 1
#define R_PAR0_CONFIG__ibusy__noninv 0
#define R_PAR0_CONFIG__ifault__BITNR 17
#define R_PAR0_CONFIG__ifault__WIDTH 1
#define R_PAR0_CONFIG__ifault__inv 1
#define R_PAR0_CONFIG__ifault__noninv 0
#define R_PAR0_CONFIG__isel__BITNR 16
#define R_PAR0_CONFIG__isel__WIDTH 1
#define R_PAR0_CONFIG__isel__inv 1
#define R_PAR0_CONFIG__isel__noninv 0
#define R_PAR0_CONFIG__dma__BITNR 9
#define R_PAR0_CONFIG__dma__WIDTH 1
#define R_PAR0_CONFIG__dma__enable 1
#define R_PAR0_CONFIG__dma__disable 0
#define R_PAR0_CONFIG__rle_in__BITNR 8
#define R_PAR0_CONFIG__rle_in__WIDTH 1
#define R_PAR0_CONFIG__rle_in__enable 1
#define R_PAR0_CONFIG__rle_in__disable 0
#define R_PAR0_CONFIG__rle_out__BITNR 7
#define R_PAR0_CONFIG__rle_out__WIDTH 1
#define R_PAR0_CONFIG__rle_out__enable 1
#define R_PAR0_CONFIG__rle_out__disable 0
#define R_PAR0_CONFIG__enable__BITNR 6
#define R_PAR0_CONFIG__enable__WIDTH 1
#define R_PAR0_CONFIG__enable__on 1
#define R_PAR0_CONFIG__enable__reset 0
#define R_PAR0_CONFIG__force__BITNR 5
#define R_PAR0_CONFIG__force__WIDTH 1
#define R_PAR0_CONFIG__force__on 1
#define R_PAR0_CONFIG__force__off 0
#define R_PAR0_CONFIG__ign_ack__BITNR 4
#define R_PAR0_CONFIG__ign_ack__WIDTH 1
#define R_PAR0_CONFIG__ign_ack__ignore 1
#define R_PAR0_CONFIG__ign_ack__wait 0
#define R_PAR0_CONFIG__oe_ack__BITNR 3
#define R_PAR0_CONFIG__oe_ack__WIDTH 1
#define R_PAR0_CONFIG__oe_ack__wait_oe 1
#define R_PAR0_CONFIG__oe_ack__dont_wait 0
#define R_PAR0_CONFIG__mode__BITNR 0
#define R_PAR0_CONFIG__mode__WIDTH 3
#define R_PAR0_CONFIG__mode__manual 0
#define R_PAR0_CONFIG__mode__centronics 1
#define R_PAR0_CONFIG__mode__fastbyte 2
#define R_PAR0_CONFIG__mode__nibble 3
#define R_PAR0_CONFIG__mode__byte 4
#define R_PAR0_CONFIG__mode__ecp_fwd 5
#define R_PAR0_CONFIG__mode__ecp_rev 6
#define R_PAR0_CONFIG__mode__off 7

#define R_PAR0_DELAY (IO_TYPECAST_UDWORD 0xb0000048)
#define R_PAR0_DELAY__hold__BITNR 16
#define R_PAR0_DELAY__hold__WIDTH 5
#define R_PAR0_DELAY__strobe__BITNR 8
#define R_PAR0_DELAY__strobe__WIDTH 5
#define R_PAR0_DELAY__setup__BITNR 0
#define R_PAR0_DELAY__setup__WIDTH 5

#endif

#ifdef ETRAXPARIO_TEST
#define PARIO_GET_STRUCT 424242
#define PARIO_SET_STRUCT 424242

struct etrax_pario
{
  unsigned long ctrl_data;
  unsigned long status_data;
  unsigned long config;
  unsigned long delay;
/* Commands */
  int assign_ctrl;
  int assign_config;
  int assign_delay;
};
#endif

static const char *usage_start = "\nUsage: hwtestserial device [options]\n\n"
"Test serial ports.\n";


#define DEVICE "/dev/ttyS0"

#ifndef MIN
#define MIN(a,b) (a<b)?a:b
#endif

/* If we should use kernal routines or raw mem access for parX I/O */
#define KERNEL_PARX_IO 1 

struct Tcontext
{
  int fd;
  const char *dev;
  int arg;
  int argc;
  char **argv;
};


#define IO_ASSIGN(var, reg, field, value) var = ((var & \
 ~( ( ( 1 << reg##__##field##__WIDTH ) - 1 ) << reg##__##field##__BITNR )) |\
  ( reg##__##field##__##value << reg##__##field##__BITNR ) )

/*  var = (var & ~IO_MASK(reg, field)) | IO_STATE(reg, field, value)*/

enum {
  IO_DIR_IN = 1,
  IO_DIR_OUT = 2,
  IO_DIR_BIPUT = 3
};

  
struct par_bit
{
  const char name[8];
  int bitnr;
  int width;
  int direction; /* 1=in, 2= out, 3 = biput */
};

#define IO_MASK_BITNR_WIDTH(bitnr, width) \
 ( ( ( 1 << (width) ) - 1 ) << (bitnr) )

#define IO_FIELD_BITNR(bitnr, val) ((val) << (bitnr))

#define PAR_CTRL(name, field) {name,  R_PAR0_CTRL_DATA##__##field##__BITNR,\
 R_PAR0_CTRL_DATA##__##field##__WIDTH, IO_DIR_OUT }
#define PAR_CTRLDATA(name, bit) {name,  R_PAR0_CTRL_DATA##__##data##__BITNR + bit, 1, IO_DIR_OUT }

#define PAR_CONFIG(name, field) {name,  R_PAR0_CONFIG##__##field##__BITNR,\
 R_PAR0_CONFIG##__##field##__WIDTH, IO_DIR_OUT }

const struct par_bit par_config_bits[]={
  PAR_CONFIG("ioe", ioe),
  PAR_CONFIG("iseli", iseli),
  PAR_CONFIG("iautofd", iautofd),
  PAR_CONFIG("istrobe", istrb),
  PAR_CONFIG("iinit", iinit),
  PAR_CONFIG("iperr", iperr),
  PAR_CONFIG("iack", iack),
  PAR_CONFIG("ibusy", ibusy),
  PAR_CONFIG("ifault", ifault),
  PAR_CONFIG("isel", isel),
  PAR_CONFIG("dma", dma),
  PAR_CONFIG("rle_in", rle_in),
  PAR_CONFIG("rle_out", rle_out),
  PAR_CONFIG("enable", enable),
  PAR_CONFIG("force", force),
  PAR_CONFIG("ign_ack", ign_ack),
  PAR_CONFIG("oe_ack", oe_ack),
  PAR_CONFIG("mode", mode)
};
#define NR_PAR_CONFIG_BITS (sizeof(par_config_bits)/sizeof(struct par_bit))


const struct par_bit par_ctrl_bits[]={
  PAR_CTRL("oe",    oe ),
  PAR_CTRL("seli",  seli),
  PAR_CTRL("autofd", autofd),    
  PAR_CTRL("strobe", strb),
  PAR_CTRL("init", init),
  PAR_CTRL("d", data),

  PAR_CTRLDATA("d7", 7),
  PAR_CTRLDATA("d6", 6),
  PAR_CTRLDATA("d5", 5),
  PAR_CTRLDATA("d4", 4),
  PAR_CTRLDATA("d3", 3),
  PAR_CTRLDATA("d2", 2),
  PAR_CTRLDATA("d1", 1),
  PAR_CTRLDATA("d0", 0)
};
#define NR_PAR_CTRL_BITS (sizeof(par_ctrl_bits)/sizeof(struct par_bit))

#define PAR_STATUS(name, field) {name,  R_PAR0_STATUS_DATA##__##field##__BITNR, R_PAR0_STATUS_DATA##__##field##__WIDTH, IO_DIR_OUT }
#define PAR_STATUSDATA(name, bit) {name,  R_PAR0_STATUS_DATA##__##data##__BITNR + bit, 1, IO_DIR_OUT }
const struct par_bit par_status_bits[]={
  PAR_STATUS("mode", mode),
  PAR_STATUS("perr",    perr ),
  PAR_STATUS("ack",  ack),
  PAR_STATUS("busy", busy),    
  PAR_STATUS("fault", fault),
  PAR_STATUS("sel", sel),
  PAR_STATUS("tr_rdy", tr_rdy),
  PAR_STATUS("dav", dav),
  PAR_STATUS("ecp_cmd", ecp_cmd),
  PAR_STATUS("d", data),

  PAR_STATUSDATA("d7", 7),
  PAR_STATUSDATA("d6", 6),
  PAR_STATUSDATA("d5", 5),
  PAR_STATUSDATA("d4", 4),
  PAR_STATUSDATA("d3", 3),
  PAR_STATUSDATA("d2", 2),
  PAR_STATUSDATA("d1", 1),
  PAR_STATUSDATA("d0", 0)
};
#define NR_PAR_STATUS_BITS (sizeof(par_status_bits)/sizeof(struct par_bit))

char* get_control_state_str(int fd, char *s);
char* port_state_to_string(char *s, unsigned char state);



typedef int Tcmd_func(struct Tcontext *cb, int argc, char* argv[]);
int set_verbose(struct Tcontext *cb, int argc, char* argv[]);
int read_info(struct Tcontext *cb, int argc, char* argv[]);
int set_dev(struct Tcontext *cb, int argc, char* argv[]);
int set_devnb(struct Tcontext *cb, int argc, char* argv[]);
int set_devnbnoctty(struct Tcontext *cb, int argc, char* argv[]);
int set_female(struct Tcontext *cb, int argc, char* argv[]);
int set_male(struct Tcontext *cb, int argc, char* argv[]);
int set_raw(struct Tcontext *cb, int argc, char* argv[]);
int set_ctrlpin(struct Tcontext *cb, int argc, char* argv[]);
int do_nohup(struct Tcontext *cb, int argc, char* argv[]);
int set_PA(struct Tcontext *cb, int argc, char* argv[]);
int set_PB(struct Tcontext *cb, int argc, char* argv[]);
int set_baud(struct Tcontext *cb, int argc, char* argv[]);
int set_specialbaud(struct Tcontext *cb, int argc, char* argv[]);
int set_parity(struct Tcontext *cb, int argc, char* argv[]);
int set_stopbits(struct Tcontext *cb, int argc, char* argv[]);
int set_flow(struct Tcontext *cb, int argc, char* argv[]);
int do_wait(struct Tcontext *cb, int argc, char* argv[]);
int do_send_break(struct Tcontext *cb, int argc, char* argv[]);
int do_tx(struct Tcontext *cb, int argc, char* argv[]);
int do_txhex(struct Tcontext *cb, int argc, char* argv[]);
int do_tx485(struct Tcontext *cb, int argc, char* argv[]);
int do_tx_test(struct Tcontext *cb, int argc, char* argv[]);
int do_rx(struct Tcontext *cb, int argc, char* argv[]);
int do_rxhex(struct Tcontext *cb, int argc, char* argv[]);
int do_rx485(struct Tcontext *cb, int argc, char* argv[]);
int do_rxdbg(struct Tcontext *cb, int argc, char* argv[]);
int do_loopbacktest(struct Tcontext *cb, int argc, char *argv[]);
int set_par0(struct Tcontext *cb, int argc, char* argv[]);
int set_par0hi(struct Tcontext *cb, int argc, char* argv[]);
int set_par0lo(struct Tcontext *cb, int argc, char* argv[]);
int set_par1(struct Tcontext *cb, int argc, char* argv[]);
int set_par1hi(struct Tcontext *cb, int argc, char* argv[]);
int set_par1lo(struct Tcontext *cb, int argc, char* argv[]);

int do_serprodtest(struct Tcontext *cb, int argc, char* argv[]);

int do_getinfospeedtest(struct Tcontext *cb, int argc, char* argv[]); 

unsigned char hexbyte(const char *s);

int do_rxsaih(struct Tcontext *cb, int argc, char* argv[], int print_hex, RX *rx);

int TtyFunc(int argc, char *argv[]);

int UtilTty(int NumArg, char *ConfTty[NCConf]);

struct cmd_struct
{
  const char *param;
  Tcmd_func  *func;
  const char *help;
};



#define set_rts set_ctrlpin
#define set_dtr set_ctrlpin
#define set_ri set_ctrlpin
#define set_cd set_ctrlpin


typedef enum 
{ 
  UNKNOWN,      
  UNCONNECTED, /* No connectors connected */
  UNPLUGGED,   /* Test equipment in UNPLUGGED state */
  PLUGGED      /* Test equipment in PLUGGED state */
} prod_test_state_type;

struct ser_state_type
{
  int fd; /* port descriptor */
  int portnbr; /* ser0..3 */
  unsigned long modem_state; /* TIOCM_xxx by ioctl(TIOCMGET) */
  int rx_state; /* State of RXD signal */
};
#define EXP_STATE(modem, rx) {0, 0, (modem), (rx)}

#define MAX_COM_PORTS   4

const struct ser_state_type exp_state_unconnected = EXP_STATE(0, 1);

struct ser_state_type exp_state[MAX_COM_PORTS];
struct ser_state_type exp_state_com2 = EXP_STATE(0, 1);
struct ser_state_type exp_state_comdebug = EXP_STATE(0, 1);
struct ser_state_type exp_state_com422 = EXP_STATE(0, 1);



/* This is the default "portname" and physical port assignment for "Bundy" 
 */
int com1 = 0;
int com2 = 2;
int comdebug = 1;
int com422 = 3;



struct prod_test_struct
{
  prod_test_state_type mode;
  struct ser_state_type state[MAX_COM_PORTS];
};


struct prod_test_struct prod_test;



const struct cmd_struct cmd_table[] = {
{"-v", set_verbose,"  -v level      How much debug info we should print\n"},
{"-d", set_dev,    "  -d device     What device to use, e.g. /dev/ttyS0\n"},
{"-dnb", set_devnb,"  -dnb device   Open device with O_NONBLOCK\n"},
{"-dnbnoctty", set_devnbnoctty,"  -dnb device   Open device with O_NONBLOCK\n"},
{"-raw", set_raw,  "  -raw          Set mode to raw\n" },
{"-female", set_female, 
                   "  -female       Set ttyS2 in female direction (ri,cd out)\n" },
{"-male", set_male, 
                   "  -male       Set ttyS2 in male direction (ri,cd in)\n" },

{"-r", read_info,  "  -r            Print info\n" },
{"-b", set_baud,   "  -b baudrate   Default 115200\n" },
{"-sb", set_specialbaud,   "  -sb baudbase/divisor   To set special baudrate to baudbase/divisor\n" },

{"-p",set_parity,  "  -p parity     Default None (None, Even, Odd)\n"},
{"-s",set_stopbits,"  -s stopbits   Default 1 (1 or 2)\n"},
{"-tx",do_tx,      "  -tx string    String to send\n"},
{"-txhex",do_txhex,"  -txhex data   Hex data (00010203 etc.) to send\n"},
{"-tx485",do_tx485,"  -tx485 string String to send using RS-485 driver\n"},
{"-rx",do_rx,      "  -rx [+]time   rx-mode in time seconds \n"
                   "                (if + the timing is strict, otherwise it's idle time)\n"}, 
{"-rxhex",do_rxhex,"  -rxhex time   rx-mode in time seconds, prints in hex\n"}, 
{"-rx485",do_rx485,"  -rx485 time   RS-485 rx-mode (rts=1 and raw) in time seconds\n"}, 
{"-rxdbg",do_rxdbg,"  -rxdbg time   rx-debug mode in time seconds (shows status)\n"}, 
{"-rxsaih",do_rxsaih,"  -rxsaih tiempo   rxsaih tiempo espera\n"}, 
{"-txtest",do_tx_test, "  -txtest size  Send size byes of testpattern\n"},
{"-flow",set_flow,     "  -flow none|x|-x|rtscts|-rtscts     Set flow control flags\n"},
{"-w",do_wait,      "  -w time      Wait time: T seconds, Tms Tus\n"},
{"-break",do_send_break,"  -break time     Send break in time*(0.25 to 0.5) seconds\n"},
{"-rts",set_rts,   "  -rts 0|1      Set RTS\n"},
{"-dtr",set_dtr,   "  -dtr 0|1      Set DTR\n"},
{"-nohup",do_nohup,"  -nohup        Don't hang up (RTS/DTR) when closing.\n"},
{"", NULL, "For female DSUB:\n" },
{"-ri", set_ri,     "  -ri 0|1       Set RI\n"},
{"-cd", set_cd,     "  -cd 0|1       Set CD\n"},
{"-sertest", do_serprodtest, "  -sertest UNCONNECTED | UNPLUGGED | PLUGGED  Do production test.\n"},
{"-loopback", do_loopbacktest, "  -loopback times rxtx 	Do a serial loopback test. Add 'rxtx'" 
	"if you only want rx -> tx to be tested.\n"   	
	"	Plug scheme: TxD->RxD, RTS->CTS,DSR, DTR->CD,RI\n"},
	
{"-PA", set_PA,     "  -PA bit value  Set PA bit = 'x' gives whole byte.\n"},
{"-PB", set_PB,     "  -PA bit value  Set PA bit = 'x' gives whole byte.\n"},

{"-par0", set_par0, "  -par0 bit value  Set par0 bit = 'x' gives whole byte.\n"},
{"-par0hi", set_par0hi, "  -par0hi          All par0 pins high.\n"},
{"-par0lo", set_par0lo, "  -par0lo          All par0 pins low.\n"},
{"-par1", set_par1, "  -par1 bit value  Set par1 bit = 'x' gives whole byte.\n"},
{"-par1hi", set_par1hi, "  -par1hi          All par1 pins high.\n"},
{"-par1lo", set_par1lo, "  -par1lo          All par1 pins low.\n"},
{"-getinfospeedtest", do_getinfospeedtest,     "  -getinfospeedtest       Test spped of get_modeminfo\n"},

  {NULL, NULL, NULL  }
};


struct Tcontext context;

int verbose_mode = 1;

#define VDBG(mode, x) if (verbose_mode >= mode){ x; }

#define DERROR(x) VDBG(1, x)
#define DWRITE(x) VDBG(2, x)
#define DREAD(x) VDBG(2, x)
  



struct state_str
{
  int state;
  const char *str;
  
};

const struct state_str modem_state_str[]={
  {TNET_COM_MODEMSTATE_CD, "CD"},
  {TNET_COM_MODEMSTATE_RI, "RI"},
  {TNET_COM_MODEMSTATE_DSR, "DSR"},
  {TNET_COM_MODEMSTATE_CTS, "CTS"},
  {TNET_COM_MODEMSTATE_CD_DELTA, "delta CD"},
  {TNET_COM_MODEMSTATE_RI_TRAIL, "trail RI"},
  {TNET_COM_MODEMSTATE_DSR_DELTA, "delta DSR"},
  {TNET_COM_MODEMSTATE_CTS_DELTA, "delta CTS"}
};

const struct state_str flow_state_str[]={

  {TNET_COM_SET_CONTROL_FLOW_XONXOFF, "XONXOFF OUT"},
  {TNET_COM_SET_CONTROL_FLOW_RTSCTS, "RTSCTS OUT" }  ,
  {TNET_COM_SET_CONTROL_BREAK_ON, "BREAK ON"},
  {TNET_COM_SET_CONTROL_BREAK_OFF, "BREAK OFF"},
  {TNET_COM_SET_CONTROL_DTR_ON, "DTR ON"},
  {TNET_COM_SET_CONTROL_DTR_OFF, "DTR OFF"},
  {TNET_COM_SET_CONTROL_RTS_ON, "RTS ON"},
  {TNET_COM_SET_CONTROL_RTS_OFF, "RTS OFF"},
  {TNET_COM_SET_CONTROL_FLOWIN_XOFF, "XONXOFF IN"},
  { TNET_COM_SET_CONTROL_FLOWIN_RTSCTS, "RTSCTS IN"},
  { TNET_COM_SET_CONTROL_FLOWIN_NONE, "NONE"},
  { 0, NULL},
};

/*   ioctl(port_fd, TIOCMGET, &MLines); */
const struct state_str control_state_str[]={
  {TIOCM_DTR, "DTR" },
  {TIOCM_RTS, "RTS"},
  {TIOCM_ST, "ST?" },
  {TIOCM_SR, "SR?" },
  {TIOCM_CTS, "CTS" },
  {TIOCM_CD, "CD" },
  {TIOCM_RI, "RI" },
  {TIOCM_DSR, "DSR" },
  {0, NULL }
};
#define TIOCM_OUTPUT_MASK (TIOCM_RTS | TIOCM_DTR) 
#define TIOCM_INPUT_MASK (TIOCM_CD | TIOCM_RI | TIOCM_CTS | TIOCM_DSR) 

/* FIXME - That asm/termios.h stuff in the kernel needs to be fixed. Until then
 * create 2 dummy structures here in order to make this program compile with
 * uC-libc Beta2.
 */
struct rs485_ctrl {
	unsigned short rts_on_send;
	unsigned short rts_after_sent;
	unsigned int  delay_rts_before_send;
 	unsigned short enabled;
};

struct rs485_wrt {
	unsigned short outc_size;
	unsigned char *outc;
};

/********************** LOCAL VARIABLE SECTION *****************************/

static int useHTML = 0;
RX rx;
//struct RX *tx,*rx;

/********************** FUNCTION DEFINITION SECTION *************************/

void usage(void)
{
  int i = 0;
  
  printf(usage_start);
  while (cmd_table[i].help != NULL)
  {
    printf(cmd_table[i].help);
    i++;
  }
  printf("  -parX ctrl signals: oe seli autofd strobe init \n");
  
}
/*************************************************************************/

void print_ser_state(struct ser_state_type *port_state)
{
  int i = 0;
  printf("Port: %i  fd=%i\n", port_state->portnbr, port_state->fd);
  printf("  RXD: %i\n", port_state->rx_state);
  while (control_state_str[i].str != NULL)
  {
    printf("  %s: %i\n", control_state_str[i].str, 
           (port_state->modem_state & control_state_str[i].state)? 1:0);
    i++;
  }
}


void get_ser_state(struct ser_state_type *port_state)
{
#if defined(__CRIS__) && defined(CRISNOMMU)
  int portnbr = port_state->portnbr;
#endif
  ioctl(port_state->fd, TIOCMGET, &port_state->modem_state);
#ifdef __CRIS__ /* This is UGLY and should be made in kernel driver... */
#ifdef CRISNOMMU
  if (portnbr == 0)
  {
    port_state->rx_state = ((*R_SERIAL0_READ) >> 12) & 1;
  }
  else if (portnbr == 1)
  {
    port_state->rx_state = ((*R_SERIAL1_READ) >> 12) & 1;
  }
  else if (portnbr == 2)
  {
    port_state->rx_state = ((*R_SERIAL2_READ) >> 12) & 1;
  }
  else if (portnbr == 3)
  {
    port_state->rx_state = ((*R_SERIAL3_READ) >> 12) & 1;
  }
#else
  /* TODO: Fix this - for now Assume the normal state */
  port_state->rx_state = 1;
#endif
#else
  /* Assume the normal state */
  port_state->rx_state = 1;
#endif  
}

void serprod_check_state(const char *teststring,
                        const struct ser_state_type *port_state, 
                        const struct ser_state_type *expected_state)
{
  if (port_state->fd > 0)
  {
    if ((port_state->modem_state & TIOCM_INPUT_MASK) != 
        (expected_state->modem_state & TIOCM_INPUT_MASK))
    {
      printf("ERROR: Port %i - State differs in %s\n", port_state->portnbr,
             teststring);
    }
    if ((port_state->rx_state) != 
        (expected_state->rx_state))
    {
      printf("ERROR: RX state differs in %s\n", teststring);
    }
  }
}


void serprod_open_ports(void)
{
  char dev[20];
  int i;
  
  for (i = 0; i < MAX_COM_PORTS; i++)
  {
    sprintf(dev, "/dev/ttyS%i",i);
    prod_test.state[i].fd = open(dev, O_RDWR);
    prod_test.state[i].portnbr = i;
    exp_state[i] = exp_state_unconnected;
    if (prod_test.state[i].fd < 0)
    {
      printf("ERROR: Failed to open port %s (%i)\n", dev, errno);
    }
    else
    {
      get_ser_state(&prod_test.state[i]);
      print_ser_state(&prod_test.state[i]);
    }
  }
}



int do_serprodtest(struct Tcontext *cb, int argc, char* argv[])
{
  int arg = 0;

  if (strcmp(argv[arg], "UNCONNECTED") == 0)
  {
    prod_test.mode = UNCONNECTED;
  }
  else if (strcmp(argv[arg], "UNPLUGGED") == 0)
  {
    prod_test.mode = UNPLUGGED;
  }
  else if (strcmp(argv[arg], "PLUGGED") == 0)
  {
    prod_test.mode = PLUGGED;
  }
  else
  {
    prod_test.mode = UNKNOWN;
    printf("ERROR: Unsupported testmode: %s\n", argv[arg]);
  }
  arg++;
  serprod_open_ports();
  
  switch (prod_test.mode)
  {
   case UNKNOWN:
    /* Do nothing */
    break;
    
   case UNCONNECTED:
    {
      const char *teststr = "Unconnected - idle";
      
      serprod_check_state(teststr, &prod_test.state[com1], 
                          &exp_state_unconnected);
      serprod_check_state(teststr, &prod_test.state[com2], 
                          &exp_state_unconnected);
      serprod_check_state(teststr, &prod_test.state[comdebug], 
                          &exp_state_unconnected);
      serprod_check_state(teststr, &prod_test.state[com422], 
                          &exp_state_unconnected);
      
    }
    
    break;
    
   case UNPLUGGED:
    break;   
   case PLUGGED:
    break;    
  };
  
  return arg;
}


/*************************************************************************/



void print_bits(const struct par_bit *bits, int size, unsigned long in_value)
{
  int i;
  int value;
  int cnt = 0;
  for (i = 0; i < size; i++)
  {
    value = (in_value & IO_MASK_BITNR_WIDTH( bits[i].bitnr,  bits[i].width)) >> 
      bits[i].bitnr;
    
    cnt++;
    if (bits[i].width == 8)
    {
      cnt = -8;
      printf("\n          %s: %i ", bits[i].name, value);
      printf("0x%02X %i%i%i%i %i%i%i%i ", value, 
             (value & 0x80) >>7,
             (value & 0x40) >>6,
             (value & 0x20) >>5,
             (value & 0x10) >>4,
             (value & 0x08) >>3,
             (value & 0x04) >>2,
             (value & 0x02) >>1,
             (value & 0x01) >>0 );
    }
    else
    {
      printf("%s: %i ", bits[i].name, value);
    }
    if (cnt >= 5)
    {
      printf("\n          ");
      cnt = 0;
    }
    
  }
}

#ifdef __CRIS__

void print_par(int port, struct etrax_pario *par)
{
#if 0
  int value = IO_EXTRACT(R_PAR0_STATUS_DATA, data, par->status_data);
#endif  
  printf("par%i: ctrl: 0x%08lX status: 0x%08lX config: 0x%08lX delay:0x%08lX\n",
         port, par->ctrl_data, par->status_data, par->config, par->delay);
  printf("  ctrl  : ");
  print_bits( par_ctrl_bits, NR_PAR_CTRL_BITS, par->ctrl_data);
  printf("\n");
  printf("  status: ");
  print_bits( par_status_bits, NR_PAR_STATUS_BITS, par->status_data);
  printf("\n");
  printf("  config: ");
  print_bits( par_config_bits, NR_PAR_CONFIG_BITS, par->config);
  printf("\n");
  
# if 0  
  printf("par%i: ctrl: oe:%u seli:%d autofd:%d strb:%d init:%d data: 0x%02X (%i)\n",
         port,
         IO_EXTRACT(R_PAR0_CTRL_DATA, oe, par->ctrl_data),
         IO_EXTRACT(R_PAR0_CTRL_DATA, seli, par->ctrl_data),
         IO_EXTRACT(R_PAR0_CTRL_DATA, autofd, par->ctrl_data),
         IO_EXTRACT(R_PAR0_CTRL_DATA, strb, par->ctrl_data),
         IO_EXTRACT(R_PAR0_CTRL_DATA, init, par->ctrl_data),
         IO_EXTRACT(R_PAR0_CTRL_DATA, data, par->ctrl_data));
  
  
  printf("par%i: status: mode: %i perr: %i ack: %i busy: %i fault: %i sel: %i\n"
         "             tr_rdy: %i dav: %i ecp_cmd: %i data: 0x%02X (%i) %i%i%i%i %i%i%i%i\n",
         port,
         IO_EXTRACT(R_PAR0_STATUS_DATA, mode, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, perr, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, ack, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, busy, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, fault, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, sel, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, tr_rdy, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, dav, par->status_data),
         IO_EXTRACT(R_PAR0_STATUS_DATA, ecp_cmd, par->status_data),
         value,
         value,
         (value & 0x80) >>7,
         (value & 0x40) >>6,
         (value & 0x20) >>5,
         (value & 0x10) >>4,
         (value & 0x08) >>3,
         (value & 0x04) >>2,
         (value & 0x02) >>1,
         (value & 0x01) >>0 );
#endif
}
#endif


int set_verbose(struct Tcontext *cb, int argc, char* argv[])
{
  verbose_mode = atoi(argv[0]);  
  return 1;
}

//Open Tty
int set_dev(struct Tcontext *cb , int argc, char* argv[])
{
	if (cb->fd > 0){			// Don't close if stdin (0) 
		close(cb->fd);
	}

	cb->dev = argv[0];
	cb->fd = open(cb->dev, O_RDWR);
	if (cb->fd < 0){
		DERROR(printf("ERROR! Failed to open %s\n", cb->dev));
	}
  
	return 1;
}

int set_devnb(struct Tcontext *cb , int argc, char* argv[])
{
  if (cb->fd > 0) /* Don't close if stdin (0) */
  {
    close(cb->fd);
  }
  cb->dev = argv[0];
  cb->fd = open(cb->dev, O_RDWR | O_NONBLOCK);
  if (cb->fd < 0)
  {
    DERROR(printf("ERROR! Failed to open %s\n", cb->dev));
  }
  return 1;
}

int set_devnbnoctty(struct Tcontext *cb , int argc, char* argv[])
{
  if (cb->fd > 0) /* Don't close if stdin (0) */
  {
    close(cb->fd);
  }
  cb->dev = argv[0];
  cb->fd = open(cb->dev, O_RDWR | O_NONBLOCK | O_NOCTTY);
  if (cb->fd < 0)
  {
    DERROR(printf("ERROR! Failed to open %s\n", cb->dev));
  }
  return 1;
}


int set_raw(struct Tcontext *cb, int argc, char* argv[])
{
  struct termios ti;
  tcgetattr(cb->fd, &ti);
  cfmakeraw(&ti);
  tcsetattr(cb->fd, TCSANOW, &ti);
  return 0;
}

int set_female(struct Tcontext *cb, int argc, char* argv[])
{
#if defined(__CRIS__) && \
    defined(CONFIG_ETRAX_SERIAL_PORT2) && \
    defined(CONFIG_ETRAX_SER2_DTR_RI_DSR_CD_ON_PA)
/* This is only available on Bundy */
  int devfd = open("/dev/gpioa", O_RDWR);
  unsigned char bitmap = 
    (1 << CONFIG_ETRAX_SER2_RI_ON_PA_BIT) |
    (1 << CONFIG_ETRAX_SER2_CD_ON_PA_BIT);
  if (devfd > 0)
  {
    ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_SETOUTPUT), bitmap);
    close( devfd );
  }
  else
  {
    DERROR(perror("ERROR! Failed to open port"));
  }
#else
  printf("ERROR: command -female not available\n");
#endif
  return 0;
}

int set_male(struct Tcontext *cb, int argc, char* argv[])
{
#if defined(__CRIS__) && \
    defined(CONFIG_ETRAX_SERIAL_PORT2) && \
    defined(CONFIG_ETRAX_SER2_DTR_RI_DSR_CD_ON_PA)
/* This is only available on Bundy */
  int devfd = open("/dev/gpioa", O_RDWR);
  unsigned char bitmap = 
    (1 << CONFIG_ETRAX_SER2_RI_ON_PA_BIT) |
    (1 << CONFIG_ETRAX_SER2_CD_ON_PA_BIT);
  if (devfd > 0)
  {
    ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_SETINPUT), bitmap);
    close( devfd );
  }
  else
  {
    DERROR(perror("ERROR! Failed to open port"));
  }
#else
  printf("ERROR: command -male not available\n");
#endif
  return 0;
}



int set_baud(struct Tcontext *cb, int argc, char* argv[])
{
  unsigned long baudrate = atol(argv[0]);
  set_port_Speed(cb->fd, baudrate);
  return 1;
}

int set_specialbaud(struct Tcontext *cb, int argc, char* argv[])
{
  if (argc >= 1)
  {
    unsigned long baudbase = atol(argv[0]);
    unsigned char *divisorp = strchr(argv[0],'/');
    unsigned long divisor = 1;
    unsigned long baudrate;
    struct serial_struct serial;
    
    if (divisorp)
    {
      divisorp++;
      divisor = atol(divisorp);
    }
    if (divisor == 0) /* Just to be safe */
    {
      divisor = 1;
    }
    
    baudrate = baudbase/divisor;
    printf("Using baudbase: %lu and divisor %lu = %lu on fd %i\n", 
           baudbase, divisor, baudrate, cb->fd);
    if (ioctl(cb->fd, TIOCGSERIAL, &serial))
    {
      perror("Error getting serial_struct with TIOCGSERIAL");
      return 1;
    }
    printf("Current setting is: baud_base: %lu divisor: %lu\n", 
	   (unsigned long)serial.baud_base, (unsigned long)serial.custom_divisor);
    
    if (baudrate)
    {
      serial.baud_base = baudbase;
      serial.custom_divisor = divisor;
      serial.flags |= ASYNC_SPD_CUST;
    }
    else
    {
      /* Remove special baudrate */ 
      serial.flags &= ~ASYNC_SPD_CUST;
    }
    
    if (ioctl(cb->fd, TIOCSSERIAL, &serial))
    {
      perror("Error setting serial_struct with TIOCSSERIAL");
    }
    return 1;
  }
  return 0;
  
}


int set_parity(struct Tcontext *cb, int argc, char* argv[])
{
  unsigned char parity;
  char *s = argv[0];
  switch (toupper(s[0]))
  {
   case 'N':
    parity = TNET_COM_PARITY_NONE;
    break;
   case 'E':
    parity = TNET_COM_PARITY_EVEN;
    break;
   case 'O':
    parity = TNET_COM_PARITY_ODD;
    break;
   case 'M':
    parity = TNET_COM_PARITY_MARK;
    break;
   case 'S':
    parity = TNET_COM_PARITY_SPACE;
    break;
   default:
    parity = TNET_COM_PARITY_NONE;
    break;
  }
  set_port_Parity(cb->fd, parity);
  return 1;
}

int set_stopbits(struct Tcontext *cb, int argc, char* argv[])
{
  unsigned char size = atoi(argv[0]);
  set_port_DataSize(cb->fd, size);
  return 1;
}

int set_flow(struct Tcontext *cb, int argc, char* argv[])
{
  char *s = argv[0];
  struct termios port_info;

	printf("\n\tSET_FLOW");

  if (tcgetattr(cb->fd, &port_info) == 0) 
  {

    if (strcmp(s, "none")==0) {
      port_info.c_iflag &= ~IXON;
      port_info.c_iflag &= ~IXOFF;
      port_info.c_iflag &= ~IXANY;
      port_info.c_cflag &= ~CRTSCTS;
      
    } else if (strcmp(s, "-x")==0) {
      port_info.c_iflag &= ~IXON;
      port_info.c_iflag &= ~IXOFF;
      port_info.c_iflag &= ~IXANY;
    } else if (strcmp(s, "x")==0) {
      port_info.c_iflag |= IXON;
      port_info.c_iflag |= IXOFF;
      port_info.c_iflag &= ~IXANY;
    } else if (strcmp(s, "-rtscts")==0) {
	port_info.c_cflag &= ~CRTSCTS;
    } else if (strcmp(s, "rtscts")==0) {
	port_info.c_cflag |= CRTSCTS;
    }else 
    {
      DERROR(printf("ERROR! -flow Don't know option '%s'!\n", s));
    }
    if (tcsetattr(cb->fd, TCSANOW, &port_info) != 0)
    {
      DERROR(printf("ERROR! -flow tcsetattr failed %i %m!\n", errno));
    }
  }
  else 
  {
    DERROR(printf("ERROR! -flow tcgetattr failed %i %m!\n", errno));
  }
  
  return 1;
}

int do_wait(struct Tcontext *cb, int argc, char* argv[])
{
  char *s;
  unsigned long val = strtol(argv[0], &s, 0);
  struct timeval tv;
  unsigned long sec;
  unsigned long usec;
  

	//printf("\n\tDO_WAIT");

	if (s[0] == 'u'){
		sec = val / 1000000;
		usec = val % 1000000;
	}
	else if (s[0] == 'm'){
		sec = val / 1000;
		usec = val % 1000;    
	}
	else{
		sec = val;
		usec = 0;
	}
  
	//Use select to wait sub second resolution, see "man 2 select" 

	tv.tv_sec = sec;
	tv.tv_usec = usec;
	select(0, NULL, NULL, NULL, &tv);
  
	return 1;
}

int do_send_break(struct Tcontext *cb, int argc, char* argv[])
{
  int duration = atoi(argv[0]);
  tcsendbreak(cb->fd, duration);
  return 1;
}

//Funcion Trasmision SAIH
int do_tx(struct Tcontext *cb, int argc, char* argv[])
{
	int i;

	TimeWait(10);
	fflush(NULL);

	//printf("\n\tDO_TX");

/*	for(i=0;i<20;i++)
		printf("\n\tDO_TX:%02x",argv[0][i]);*/

	int len = strlen(argv[0]);

	if(argv[0][4]==LOGER_MSG_QM)
		len=LOGER_LMSGCL_QM;
	else if(argv[0][4]==LOGER_MSG_TM)
		len=LOGER_LMSGCL_TM;
	else if(argv[0][4]==LOGER_MSG_IN)
		len=LOGER_LMSGCL_IN;
	else
		len=40;
	if(DEBUG){printf("\n\tLMSG=%d",len);}

	if (write(cb->fd, argv[0], len) == len){	//Write ok
		DWRITE(printf("\n\tWrite %i bytes ok\n", len));
	}
	else{
		DERROR(printf("\n\tRS:ERROR! Failed to write!\n");
		perror("\n\tRS:ERROR in write"));
	}
	return 1;
}



int do_txhex(struct Tcontext *cb, int argc, char* argv[])
{
  char *hex = argv[0];
  int hexlen = strlen(hex);
  unsigned char *buf = malloc(hexlen/2 + 1);
  int len = 0;


  while (hex[0] && hex[1] )
  {
    buf[len++] = hexbyte(hex);
    hex++;
    hex++;
  }
  if (write(cb->fd, buf, len) == len)
  {
    /* Write ok */
    DWRITE(printf("Wrote %i bytes ok\n", len));
  }
  else
  {
    DERROR(printf("ERROR! Failed to write!\n");
           perror("ERROR in write"));
  }
  free(buf);
  return 1;
}


int do_tx485(struct Tcontext *cb, int argc, char* argv[])
{
#ifdef TIOCSERSETRS485
  int len = strlen(argv[0]);
  struct rs485_ctrl ctrl485;
  struct rs485_wrt   io485;  
  int status;
  memset(&ctrl485, 0, sizeof(ctrl485));
  
  ctrl485.rts_on_send = 0;
  ctrl485.rts_after_sent = 1;
  ctrl485.delay_rts_before_send = 0;
  status = ioctl(cb->fd, TIOCSERSETRS485, &ctrl485);
  if (status)
  {
    DERROR(printf("ERROR! TIOCSERSETRS485 failed %i\n", status));
  }
  
  io485.outc_size = len;
  io485.outc = argv[0];
  status = ioctl(cb->fd, TIOCSERWRRS485, &io485);
  if (status != io485.outc_size)
  {
    DERROR(printf("ERROR! TIOCSERWRRS485 failed %i\n", status));
  }
  return 1;
#else
  printf("RS-485 support not implemented, using standard tx\n");
  return do_tx(cb, argc, argv);
#endif
}
int do_rx485(struct Tcontext *cb, int argc, char* argv[])
{
  int m = TIOCM_RTS;
  struct termios ti;
  /* Set raw mode and rts=1 before receiving */
  tcgetattr(cb->fd, &ti);
  cfmakeraw(&ti);
  tcsetattr(cb->fd, TCSANOW, &ti);  
  ioctl(cb->fd, TIOCMBIS, &m);
  return do_rx(cb, argc, argv);
}

int do_rxany(struct Tcontext *cb, int argc, char* argv[], int print_hex)
{
  unsigned long timeleft = atol(argv[0]);
  unsigned long strict_time_us = 0;
  unsigned char buf[100];
  fd_set readfds;
  int selcode;
  int size;
  struct timeval timeout;
  struct timeval t0, t1;
  if (argv[0][0] == '+') 
  {
    strict_time_us = timeleft*1000000;
  }
  
    
  DREAD(printf("Receive in %lu seconds\n", timeleft));

  if (cb->fd < 0)
  {
    return 1;
  }
  gettimeofday(&t0, NULL);
  while (timeleft)
  {
    FD_ZERO(&readfds);
    FD_SET(cb->fd, &readfds);
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    if ((selcode = select(cb->fd + 1,
                          &readfds,
                          NULL,
                          NULL,
                          &timeout)) != 0)
    {
      /* Something happend */
      if (selcode == -1)
      {
        DERROR(perror("ERROR! select(): "));
      }
      else
      {
        if (FD_ISSET(cb->fd, &readfds))
        {
          
          size = read(cb->fd, buf, sizeof(buf)-1);
          if(size > 0) {
            buf[size]='\0';
            if (print_hex)
            {
              int i = 0;
              while (i<size)
              {
                printf("%02X ",(int)buf[i]);
                i++;
              }
              printf("\n");
            }
            else
            {
              printf("%s",buf);
            }
            
            fflush(stdout);
          }
        }
      }
    }
    else
    {
      timeleft--;
/*      printf(".");
        fflush(stdout);*/
    }
    if (strict_time_us) 
    {
      gettimeofday(&t1, NULL);
      if (((t1.tv_sec - t0.tv_sec)*1000000 +t1.tv_usec - t0.tv_usec) > strict_time_us)
        timeleft = 0;
    }
    
  } /* while */
  return 1;
}

int do_rx(struct Tcontext *cb, int argc, char* argv[])
{
  return do_rxany(cb, argc, argv, 0);
}

int do_rxhex(struct Tcontext *cb, int argc, char* argv[])
{
  return do_rxany(cb, argc, argv, 1);
}
 

void print_com_status(int fd)
{
  static int prev_tmp = -42;
  int tmp;
  static int prev_MLines = -42;
  int MLines;
  char s[200];
  
  struct termios port_info;

  tcgetattr(fd, &port_info);
  ioctl(fd, TIOCMGET, &MLines);
  if (MLines != prev_MLines)
  {
    prev_MLines = MLines;
    get_control_state_str(fd, s);
    printf(" Control state: 0x%04X %s\r\n", MLines, s);
  }
  
  tmp = get_port_state(fd, 0xFF);
  if (tmp != prev_tmp)
  {
    prev_tmp = tmp;
    port_state_to_string(s, tmp);
    printf("  Modem state : 0x%02X %s\r\n", tmp, s);
  }
  
}



int do_rxdbg(struct Tcontext *cb, int argc, char* argv[])
{
  unsigned long timetot = atol(argv[0]);
  char buf[100];
  fd_set readfds;
  int selcode;
  int size;
  int stdin_fd;
  struct termios t;
  struct termios org_stdin_term;
  int err;
  
  struct timeval timeout;
  DREAD(printf("Receive in %lu seconds\r\n", timetot));
  if (cb->fd < 0)
  {
    return 1;
  }
  stdin_fd = 0; /*open("/dev/stdin", O_RDONLY | O_NONBLOCK);*/
  err = tcgetattr(stdin_fd, &org_stdin_term);  
  if (stdin_fd < 0)
  {
    perror("stdin:");
    stdin_fd = 0;
  }
  else
  {
    
    setbuf(stdin, NULL);
    err = tcgetattr(stdin_fd, &t);
    if (err) perror("tcgetattr");
    cfmakeraw(&t);
    tcsetattr(stdin_fd, TCSANOW, &t);
    if (err) perror("tcsetattr");
  }

  if (err) perror("tcgetattr");

  while (timetot)
  {
    FD_ZERO(&readfds);
    FD_SET(cb->fd, &readfds);
    FD_SET(stdin_fd, &readfds);
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    print_com_status(cb->fd);

    
    if ((selcode = select(cb->fd + 1,
                          &readfds,
                          NULL,
                          NULL,
                          &timeout)) != 0)
    {
      /* Something happend */
      if (selcode == -1)
      {
        DERROR(perror("ERROR! select(): "));
      }
      else
      {
        if (FD_ISSET(cb->fd, &readfds))
        {
          size = read(cb->fd, buf, sizeof(buf)-1);
          if(size > 0) {
            buf[size]='\0';
            printf("%s",buf);
            fflush(stdout);
          }
          else
          {
            perror("cb->fd");
          }
          
        }
        if (FD_ISSET(stdin_fd, &readfds))
        {
          size = read(stdin_fd, buf, 1/*sizeof(buf)-1*/);
          if(size > 0) {
            buf[size]='\0';
            if (size == 1 && buf[0] == 0x03 /* Ctrl+C */)
            {
              printf("Ctrl+C detected. Exit...\r\n");
              timetot = 0;
            }
            else
            {
              write(cb->fd, buf, size);
            }
          }
          else
          {
            perror("stdin");
          }
        }
      }
    }
    else
    {
      timetot--;
/*      printf(".");
        fflush(stdout);*/
    }
  } /* while */
  err = tcsetattr(stdin_fd, TCSANOW, &org_stdin_term);
  if (err) perror("tcsetattr");
  return 1;
}

unsigned long buf_size = 1024;
int echo_testpattern = 0;

#define TEST 0

void send_testpattern(int socket_handle, long int testsize)
{
  char *buf;

  unsigned long cnti = 0;
  unsigned long cnto_tot = 0;
  ssize_t cnto = 0;
  unsigned long bytes_sent = 0;
  time_t t1, t2;
  unsigned long row_cnt = 0;
  buf = malloc(buf_size+100);
  t1 = time(NULL);
  while (bytes_sent<testsize)
  {
    
    cnti = 0;
    do
    {
      if (((bytes_sent+cnti) % 50) != 0)
      {
        DERROR(printf("ERROR? %lu ? \n", bytes_sent+cnti));
      }
      
      sprintf(&buf[cnti],
              "-%8lu 1234567890 abcdefghij klmnopqrstuvwxyz\r\n",
              bytes_sent+cnti);
      buf[cnti+row_cnt] = '\\';
      if (row_cnt>0)
	    {
	      buf[cnti+row_cnt-1] = ' ';            
	    }
      if (row_cnt<47)
      {
        buf[cnti+row_cnt+1] = ' ';            
      }          
      
      switch (row_cnt%6)
      {
       case 0:
       case 1:
       case 2:             
        buf[cnti+row_cnt%3] = '\\';
        break;
       case 3:
        buf[cnti+2] = '/';
        break;                          
       case 4:
        buf[cnti+1] = '/';
        break;                          
       case 5:             
        buf[cnti+0] = '/';
        break;
       default:
        break;
      }
      cnti += strlen(&buf[cnti]);          
      row_cnt =(row_cnt+1)% (48);
    }
    while(cnti+bytes_sent<testsize && (cnti<buf_size));
    
    cnto_tot=0;
#if TEST        
    printf("Buffer:'\n %s'\n",   &buf[cnto_tot]);
#endif        
    do
    {
#if TEST
      printf("chunk:%lu bytes sent in %lu byte chunks\n",
             cnti, MIN(buf_size, cnti-cnto_tot));
      
      printf(buf);
      cnto = cnti;
#else
/*          printf("chunk:%lu bytes sent in %lu byte chunks\n",
            cnti, MIN(buf_size, cnti-cnto_tot));
*/
      if (echo_testpattern)
      {
        printf("%.*s", (int)(MIN(buf_size, cnti-cnto_tot)), &buf[cnto_tot]);
      }
      
      cnto = write(socket_handle, &buf[cnto_tot], cnti-cnto_tot);
      if (cnto != (cnti-cnto_tot))
      {
        DWRITE(printf("! write incomplete at %lu. %lu != %lu errno: %i\n", 
                      bytes_sent, (unsigned long)cnto, 
                      (cnti-cnto_tot), errno));
      }
      if (cnto < 0)
      {
        cnto = 0;
        if (errno == EAGAIN)
        {
          /* Ok, try again */
          fd_set fds;
          FD_ZERO(&fds);
          FD_SET(socket_handle, &fds); // 1, 2
          select(socket_handle+1,NULL, &fds, NULL, NULL);
        }
        else
        {
          DWRITE(perror("Warning!  cnto < 0 \n"));
        }
        
      }
      
#endif          
      bytes_sent += cnto;
      cnto_tot += cnto;
    }while(cnto_tot < cnti);
    
  }
  t2 = time(NULL);
  
  {
    unsigned long t_tot = t2 - t1+1;
    
    printf("Wrote %lu bytes in %lu seconds: mean %lu bytes/sec\n",
           bytes_sent, t_tot, bytes_sent/t_tot);
  }
  free(buf);
}

int do_tx_test(struct Tcontext *cb, int argc, char* argv[])
{
  unsigned long size = atol(argv[0]);
  send_testpattern(cb->fd, size);
  return 1;
}




char* get_control_state_str(int fd, char *s)
{
  int MLines;
  int i = 0;
  s[0]='\0';
  ioctl(fd, TIOCMGET, &MLines);
  while (control_state_str[i].str != NULL)
  {
    if (MLines & control_state_str[i].state)
    {
      if (s[0] != '\0')
      {
        strcat(s, ", ");
      }
      strcat(s, control_state_str[i].str);
    }
    i++;
  }
  return s;
}

// Set RTS 0 | 1 SAIH
int set_ctrlpin(struct Tcontext *cb, int argc, char* argv[])
{


	//printf("\n\tSET_RTS - SET_CTRLPIN:");

	char *par = cb->argv[cb->arg];
	char *pin = &par[1]; /* skip - */
	int value = atoi(argv[0]);
	int MLines;
	int i = 0;
  
	//printf("\n\tSetting par: %s pin %s to %i\n", par, pin, value);
	ioctl(cb->fd, TIOCMGET, &MLines);  
	//printf("\n\tRead Modem State: %i 0x%08X\n",MLines, MLines);

	while (control_state_str[i].str != NULL){
		if (strcasecmp(pin, control_state_str[i].str) == 0){
			if (value){
				MLines = MLines | control_state_str[i].state;
			}
			else{
				MLines = MLines & ~control_state_str[i].state;
			}
		}
		i++;
	}
	//printf("\n\tWrite Modem State: %i 0x%08X\n",MLines, MLines);
	ioctl(cb->fd, TIOCMSET, &MLines);  
	return 1;
}


int do_nohup(struct Tcontext *cb, int argc, char* argv[])
{
  if (cb->fd > 0)
  {
    struct termios port_info;
    
    tcgetattr(cb->fd, &port_info);    
    port_info.c_cflag = port_info.c_cflag & ~HUPCL;
    tcsetattr(cb->fd, TCSANOW, &port_info);
  }
  
  return 0;
}


int set_PX(struct Tcontext *cb, int argc, char* argv[], const char *dev)

{
#ifdef __CRIS__
	int devfd;
#endif
  int bit;
  int bitmap;
  int value;
  
  if (strcmp(argv[0],"x") == 0)
  {
    /* Whole byte */
    if (strncmp(argv[1], "0x", 2) == 0)
    {
      /* Hex value */
      bitmap = strtol(argv[1], NULL, 0);
    }
    else
    {
      bitmap = strtol(argv[1], NULL, 0);
    }
    printf("%s setting byte to 0x%02X (%i)\n", dev, bitmap, bitmap);
#ifdef __CRIS__
    devfd = open(dev, O_RDWR);
    if (devfd > 0)
    {
      value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);
      ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_SETBITS), bitmap);
      ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_CLRBITS), ~bitmap);
      close( devfd );
    }
    else
    {
      DERROR(perror("ERROR! Failed to open port"));
    }
    
#endif
  }
  else
  {
    bit = atoi(argv[0]);
    bitmap = 1 << bit;
    
    value = atoi(argv[1]);
    printf("%s setting bit %i (0x%02X) to %i\n", dev, bit, bitmap, value);
#ifdef __CRIS__
    devfd = open(dev, O_WRONLY);
    if (devfd > 0)
    {
      /*value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);*/
      if (value)
      {
        ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_SETBITS), bitmap);
      }
      else
      {
        ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_CLRBITS), bitmap);
      }
      close( devfd );
    }
    else
    {
      DERROR(perror("ERROR! Failed to open port"));
    }
#endif
  }
  
  

  return 2;
}
int set_PA(struct Tcontext *cb, int argc, char* argv[])
{
  return set_PX(cb, argc, argv, "/dev/gpioa");
}
int set_PB(struct Tcontext *cb, int argc, char* argv[])
{
  return set_PX(cb, argc, argv, "/dev/gpiob");
}


char* port_state_to_string(char *s, unsigned char state)
{
  int i;
  s[0]='\0';
  
  for (i = 0; i < 8; i++)
  {
    if (state & modem_state_str[i].state)
    {
      if (s[0] != '\0')
      {
        strcat(s, ", ");
      }
      strcat(s, modem_state_str[i].str);
    }
  }
  return s;
}

char* flow_state_to_string(char *s, unsigned char state)
{
  int i = 0;
  s[0]='\0';
  
  while (flow_state_str[i].str != NULL)
  {
    if (state == flow_state_str[i].state)
    {
      if (s[0] != '\0')
      {
        strcat(s, ", ");
      }
      strcat(s, flow_state_str[i].str);
    }
    i++;
  }
  return s;
}

void print_cflag_fields(unsigned int cflag)
{
  printf("cflag: 0x%04X", cflag);
  if ((cflag & CSIZE) == CS7)
  {
    printf(", 7bit");
  }
  else
  {
    printf(", 8bit");
  }
  if (cflag & CSTOPB)
  {
    printf(", 2stopbits");
  }
  else
  {
    printf(", 1stopbits");
  }    
  printf(", PARITY");
	if (cflag & PARENB) {

    if (cflag & PARODD) {
      printf(" ODD");
    }
    else
    {
      printf(" EVEN");
    }
  }
  else
  {
    printf(" NONE");
	}
	
	if (cflag & CRTSCTS) {
    printf(", RTS/CTS");
	}
  printf("\n");
  
}



int read_info(struct Tcontext *cb , int argc, char* argv[])
{
  unsigned char tmp;
  char s[100];
  
  printf("# Device      : %s\n", cb->dev);
  printf("  Baudrate    : %lu\n", (unsigned long)get_port_Speed(cb->fd));
  printf("  Datasize    : %lu\n", (unsigned long)get_port_DataSize(cb->fd));
  printf("  Parity      : %lu\n", (unsigned long)get_port_Parity(cb->fd));
  printf("  Stopsize    : %lu\n", (unsigned long)get_port_StopSize(cb->fd));

  tmp = get_port_FlowControl(cb->fd, TNET_COM_SET_CONTROL_REQUEST_FLOW);
  flow_state_to_string(s, tmp);
  printf("  Flow Out    : %lu %s\n", (unsigned long)tmp, s);

  tmp = get_port_FlowControl(cb->fd, TNET_COM_SET_CONTROL_REQUEST_FLOWIN);
  flow_state_to_string(s, tmp);
  printf("  Flow In     : %lu %s\n", (unsigned long)tmp, s);
  
  get_port_FlowControl(cb->fd, TNET_COM_SET_CONTROL_REQUEST_BREAK);
  flow_state_to_string(s, tmp);
  printf("  Flow break  : %lu %s\n", (unsigned long)tmp, s);
#if 0
  get_port_FlowControl(cb->fd, TNET_COM_SET_CONTROL_REQUEST_DTR);
  flow_state_to_string(s, tmp);
  printf("  Flow DTR    : %lu %s\n", (unsigned long)tmp, s);
  get_port_FlowControl(cb->fd, TNET_COM_SET_CONTROL_REQUEST_RTS);  
  port_state_to_string(s, tmp);         
  printf("  Flow RTS    : %lu %s\n", (unsigned long)tmp, s);
#endif  

  {
    struct termios port_info;
    int MLines;
    tcgetattr(cb->fd, &port_info);
    ioctl(cb->fd, TIOCMGET, &MLines);
    get_control_state_str(cb->fd, s);
    printf(" Control state: 0x%04X %s\n", MLines, s);
    printf(" ");
    print_cflag_fields(port_info.c_cflag);
  }
  
  tmp = get_port_state(cb->fd, 0xFF);
  port_state_to_string(s, tmp);
  printf("  Modem state : 0x%02X %s\n", tmp, s);


#ifdef __CRIS__
  {
    int devfd;
    int value;
    devfd = open("/dev/gpioa", O_RDONLY);
    value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);
    close(devfd);
    printf("PA: 0x%02X %i%i%i%i %i%i%i%i\n", value, 
           (value & 0x80) >>7,
           (value & 0x40) >>6,
           (value & 0x20) >>5,
           (value & 0x10) >>4,
           (value & 0x08) >>3,
           (value & 0x04) >>2,
           (value & 0x02) >>1,
           (value & 0x01) >>0 );
    
    devfd = open("/dev/gpiob", O_RDONLY);
    value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);
    close(devfd);
    printf("PB: 0x%02X %i%i%i%i %i%i%i%i\n", value, 
           (value & 0x80) >>7,
           (value & 0x40) >>6,
           (value & 0x20) >>5,
           (value & 0x10) >>4,
           (value & 0x08) >>3,
           (value & 0x04) >>2,
           (value & 0x02) >>1,
           (value & 0x01) >>0 );

    devfd = open("/dev/lp0", O_RDWR);
    if (devfd > 0)
    {
      struct etrax_pario par;
#if 0
      value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);
      printf("par0: 0x%02X %i%i%i%i %i%i%i%i\n", value, 
             (value & 0x80) >>7,
             (value & 0x40) >>6,
             (value & 0x20) >>5,
             (value & 0x10) >>4,
             (value & 0x08) >>3,
             (value & 0x04) >>2,
             (value & 0x02) >>1,
             (value & 0x01) >>0 );
#endif
      value = ioctl(devfd, PARIO_GET_STRUCT, &par);
      if (value != 0)
      {
        DERROR(perror("ERROR! ioctl PARIO_GET_STRUCT"));
      }
      printf("\n");
      print_par(0, &par);
      close( devfd );
    }
    else
    {
      DERROR(perror("ERROR! Failed to open par0 (/dev/lp0)"));
    }  
    devfd = open("/dev/lp1", O_RDWR);
    if (devfd > 0)
    {
      struct etrax_pario par;
#if 0
      value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);
      printf("par1: 0x%02X %i%i%i%i %i%i%i%i\n", value, 
             (value & 0x80) >>7,
             (value & 0x40) >>6,
             (value & 0x20) >>5,
             (value & 0x10) >>4,
             (value & 0x08) >>3,
             (value & 0x04) >>2,
             (value & 0x02) >>1,
             (value & 0x01) >>0 );
#endif
      value = ioctl(devfd, PARIO_GET_STRUCT, &par);
      if (value != 0)
      {
        DERROR(perror("ioctl PARIO_GET_STRUCT"));
      }
      printf("\n");
      print_par(1, &par);
      close( devfd );
    }
    else
    {
      DERROR(perror("ERROR! Failed to open par1 (/dev/lp1)"));
    }  
  }
#endif

  return 0;
}

#ifdef __CRIS__
#if !KERNEL_PARX_IO
volatile unsigned long * par_ctrl_data[2] =
{	R_PAR0_CTRL_DATA,
  R_PAR1_CTRL_DATA
};
volatile unsigned long par_ctrl_data_shadow[2] =
{		
  IO_STATE(R_PAR0_CTRL_DATA, oe, enable),
  IO_STATE(R_PAR0_CTRL_DATA, oe, enable),
};

const volatile unsigned long * par_status_data[2] =
{	R_PAR0_STATUS_DATA,
  R_PAR1_STATUS_DATA
};

volatile unsigned long * par_config[2] =
{	R_PAR0_CONFIG,
  R_PAR1_CONFIG
};
#endif
#endif



int set_parX(struct Tcontext *cb, int argc, char* argv[], int port)
{
#ifdef __CRIS__
	int devfd;
#endif
  int bit;
  int bitmap;
  int value;
  char dev[20];
  sprintf(dev, "/dev/lp%i", port);
  
#if !KERNEL_PARX_IO
	*par_config[port] = 
		IO_STATE(R_PAR0_CONFIG, dma, disable)      |
		IO_STATE(R_PAR0_CONFIG, ioe, noninv)       |
		IO_STATE(R_PAR0_CONFIG, iseli, inv)       |
		IO_STATE(R_PAR0_CONFIG, iautofd, inv)     |
		IO_STATE(R_PAR0_CONFIG, istrb, inv)       |
		IO_STATE(R_PAR0_CONFIG, iinit, inv)       |
		IO_STATE(R_PAR0_CONFIG, iack, noninv)       |
		IO_STATE(R_PAR0_CONFIG, ibusy, noninv)       |
		IO_STATE(R_PAR0_CONFIG, rle_in, disable)  |
		IO_STATE(R_PAR0_CONFIG, rle_out, disable) |
		IO_STATE(R_PAR0_CONFIG, enable, on)       |
		IO_STATE(R_PAR0_CONFIG, force, on)        |
		IO_STATE(R_PAR0_CONFIG, ign_ack, wait)    |
		IO_STATE(R_PAR0_CONFIG, oe_ack, wait_oe)  |
		IO_STATE(R_PAR0_CONFIG, mode, manual);
#endif  

  if ((strcmp(argv[0],"x") == 0) || (strcmp(argv[0],"d") == 0))
  {
    /* Whole byte */
    if (strncmp(argv[1], "0x", 2) == 0)
    {
      /* Hex value */
      bitmap = strtol(argv[1], NULL, 0);
    }
    else
    {
      bitmap = strtol(argv[1], NULL, 0);
    }
    printf("par%i setting byte to 0x%02X (%i)\n", port, bitmap, bitmap);

#ifdef __CRIS__
#if KERNEL_PARX_IO
    devfd = open(dev, O_RDWR);
    if (devfd > 0)
    {
      value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);
      printf("par%i: 0x%02X %i%i%i%i %i%i%i%i\n", port, value, 
             (value & 0x80) >>7,
             (value & 0x40) >>6,
             (value & 0x20) >>5,
             (value & 0x10) >>4,
             (value & 0x08) >>3,
             (value & 0x04) >>2,
             (value & 0x02) >>1,
             (value & 0x01) >>0 );

      ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_SETBITS), bitmap);
      ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_CLRBITS), ~bitmap);
      close( devfd );
    }
    else
    {
      DERROR(perror("ERROR! Failed to open port"));
    }
#else

    par_ctrl_data_shadow[port] &= ~bitmap;
    par_ctrl_data_shadow[port] |= bitmap;
    *par_ctrl_data[port] = par_ctrl_data_shadow[port];
      
#endif    
#endif
  }
  else
  {
    if (isdigit(argv[0][0]))
    {
      bit = atoi(argv[0]);
      bitmap = 1 << bit;
    
      value = atoi(argv[1]);
      printf("par%i setting bit %i (0x%02X) to %i\n", 
             port, bit, bitmap, value);
#ifdef __CRIS__
#if KERNEL_PARX_IO
      devfd = open(dev, O_WRONLY);
      if (devfd > 0)
      {
        /*value = ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS), NULL);*/
        if (value)
        {
          ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_SETBITS), bitmap);
        }
        else
        {
          ioctl(devfd, _IO(ETRAXGPIO_IOCTYPE, IO_CLRBITS), bitmap);
        }
        close( devfd );
      }
      else
      {
        DERROR(perror("ERROR! Failed to open port"));
      }
#else
      if (value)
      {
        par_ctrl_data_shadow[port] |= bitmap;
      }
      else
      {
        par_ctrl_data_shadow[port] &= ~bitmap;
      }
      *par_ctrl_data[port] = par_ctrl_data_shadow[port];
#endif
#endif
    }
    else
    {
      struct etrax_pario par;
      int io = 0;
      int arg = 0;
      int value;
      par.ctrl_data = 0;
      
#ifdef __CRIS__
      devfd = open(dev, O_WRONLY);
      if (devfd > 0)
#endif
      {
#ifdef __CRIS__
        value = ioctl(devfd, PARIO_GET_STRUCT, &par);
#else
        value = 0;
#endif
        if (value != 0)
        {
          DERROR(perror("ioctl PARIO_GET_STRUCT"));
        }
        while (arg < (argc-1) && (argv[arg][0] != '-'))
        {
          io = 0;
          if ((strcmp(argv[arg],"x") == 0) || (strcmp(argv[arg],"d") == 0))
          {
            /* Whole byte */
            arg++;
            if (strncmp(argv[arg], "0x", 2) == 0)
            {
              /* Hex value */
              bitmap = strtol(argv[arg], NULL, 0);
            }
            else
            {
              bitmap = strtol(argv[arg], NULL, 0);
            }
            arg++;
            par.ctrl_data &=  ~0x000000FF;
            par.ctrl_data |=  bitmap;
            printf("par%i setting byte to 0x%02X (%i)\n", 
                   port, bitmap, bitmap);
          }
          else
          {
            int found = 0;
            while((io < NR_PAR_CTRL_BITS) && 
                  strcmp(par_ctrl_bits[io].name, argv[arg]) != 0)
            {
              io++;
            }
            if (io < NR_PAR_CTRL_BITS)
            {
              /* We found I/O */
              found=1;
              arg++;
              value = atoi(argv[arg]);
              arg++;
              DWRITE(printf("Found I/O: %s value: %i\n", 
                            par_ctrl_bits[io].name, value));
              par.ctrl_data &= ~IO_MASK_BITNR_WIDTH( par_ctrl_bits[io].bitnr,
                                                     par_ctrl_bits[io].width);
              par.ctrl_data |=  IO_FIELD_BITNR(par_ctrl_bits[io].bitnr, value);
              par.assign_ctrl = 1;
            }
            io = 0;
            
            while((io < NR_PAR_CONFIG_BITS) && 
                  strcmp(par_config_bits[io].name, argv[arg]) != 0)
            {
              io++;
            }
            if (io < NR_PAR_CONFIG_BITS)
            {
              /* We found config */
              found = 1;
              arg++;
              value = atoi(argv[arg]);
              arg++;
              DWRITE(printf("Found config: %s value: %i\n", 
                            par_config_bits[io].name, value));
              par.config &= ~IO_MASK_BITNR_WIDTH( par_config_bits[io].bitnr,
                                                  par_config_bits[io].width);
              par.config |=  IO_FIELD_BITNR(par_config_bits[io].bitnr, value);
              par.assign_config = 1;
            }
            if (!found)
            {
              DERROR(printf("Unknown param: %s\n", argv[arg]));
              arg++;
            }
          }
          
        } /* while */
        DWRITE(printf("ctrl: 0x%08lX config: 0x%08lX\n", 
                      par.ctrl_data, par.config));
        if (par.assign_ctrl & !par.assign_config)
        {
          IO_ASSIGN(par.config, R_PAR0_CONFIG, mode, manual);
          IO_ASSIGN(par.config, R_PAR0_CONFIG, force, on);
          IO_ASSIGN(par.config, R_PAR0_CONFIG, dma, disable);
          par.assign_config = 1;
        }
#ifdef __CRIS__        
        value = ioctl(devfd, PARIO_SET_STRUCT, &par);
        if (value != 0)
        {
          DERROR(perror("ERROR! ioctl PARIO_SET_STRUCT"));
        }
        close(devfd);
#endif
        return arg;
      }
#ifdef __CRIS__
      else
      {
        DERROR(perror("ERROR! Failed to open\n"));
      }
#endif      
    }
  }
  return 2;
}

int set_par0(struct Tcontext *cb, int argc, char* argv[])
{
  return set_parX(cb, argc, argv, 0);
}

int set_par1(struct Tcontext *cb, int argc, char* argv[])
{
  return set_parX(cb, argc, argv, 1);
}




/*#**************************************************************************
*#                                                            
*# FUNCTION NAME: test_serial
*#                                                            
*# DESCRIPTION  : Test the serial port
*#                                                            
*#**************************************************************************/
void
test_serial(int restore_content)
{
#if 0
	int fd, fio;
  int size;
  off_t ofs;
  char *buf;
  char *buf2;
  char *buf_org = malloc(EEPROM_MAXSIZE);
  
	printf("Test serial: %s\n", DEVICE);
	// Open the driver
	fd = open(DEVICE, O_RDWR);
	if(fd < 0) {
		printf("Open serial device Error\n");
		perror("hwtestserial: open RDWR");
		return;
	}
  size = read(fd, buf_org, EEPROM_MAXSIZE);
  if (size > 0)
  {
    printf("EEPROM size: %i\n", size);
  }
  else
  {
    printf("EEPROM ERROR, failed to read!\n");
    perror("EEPROM ERROR");
  }
#if 0  
  /* Try to find size using lseek */
  ofs = size;
  while (ofs != -1)
  {
    size = ofs;
    ofs = lseek(fd, ofs, SEEK_SET);
    if (ofs == size)
    {
      ofs = ofs * 2;
    }
    else
    {
      ofs = -1;
    }
  }
#endif
  close(fd);
  
  fd = open(DEVICE, O_RDWR);
	if(fd < 0) {
		printf("Open EEPROM device Error\n");
		perror("hwtesteeprom: open RDWR");
		return;
	}  
  buf = malloc(size + 4);
  ofs = 0;
  while (ofs < size)
  {
    sprintf(&buf[ofs],"%04X", ofs);
    ofs +=4;
  }
  if (write(fd, buf, size) == size)
  {
    printf("Write OK\n");
  }
  else
  {
    printf("Write failed!\n");
    perror("EEPROM: Write");
  }
  buf2 = malloc(size);
  if (lseek(fd, 0, SEEK_SET) != 0 )
  {
  }
  memset(buf2, 0, size);
  
  ofs = read(fd, buf2, size);
  if (ofs == size && memcmp(buf, buf2, size) == 0)
  {
    printf("lseek and read back ok\n");
  }
  close(fd);
  memset(buf2, 0, size);
/* Open new and read */
  fd = open(DEVICE, O_RDWR);
	if(fd < 0) {
		printf("Open EEPROM device Error\n");
		perror("hwtesteeprom: open RDWR");
		return;
	}    
  ofs = read(fd, buf2, size);
  if (ofs == size && memcmp(buf, buf2, size) == 0)
  {
    printf("read back ok\n");
  }
  else
  {
    printf("ERROR: Failed to read back!\n");
  }
  
  close(fd);
  if (restore_content)
  {
    /* Write back original result */
    fd = open(DEVICE, O_RDWR);
    if(fd < 0) {
      printf("Open EEPROM device Error\n");
      perror("hwtesteeprom: open RDWR");
      return;
    }
    if (write(fd, buf_org, size) == size)
    {
      printf("Write back OK\n");
    }
  }
  
  free(buf);
  free(buf2);
  free(buf_org);
#endif
}

/*
 * For this to work, a "loopback plug" must be connected to
 * the serial port. Possible wire scheme:
 *
 * TxD -> RxD
 * RTS -> CTS, DSR
 * DTR -> CD, RI
 *
 * It basically fiddles with the pins, sends some data, and 
 * makes sure it recieves the same thing sent. Could perhaps 
 * been called flag_test() or something, but hey...
 */
int 
do_loopbacktest(struct Tcontext *cb, int argc, char *argv[])
{
  int flags, i, wcnt, rcnt, ret, status, wait, rtrn;
  char *tx = "sending...\n";
  char rx[255];
  int rts_mask = TIOCM_RTS;
  int dtr_mask = TIOCM_DTR;
  int times = 5;
  int rxtx_only = 0;
  pid_t pid;

  /* Increment i until there are no more arguments or we reach a new command option starting with '-' */
  for (i = 0; (i < (argc - 1)) && (argv[i][0] != '-'); i++) {
    if (!strcmp(argv[i] ,"rxtx")) 
      rxtx_only = 1;
    else
      times = atoi(argv[i]);
  }

  /* We return the number of arguments that belong to the -loopback option, max 2 */
  if (i >= 2)
    rtrn = 2;
  else
    rtrn = 1;	  
  
  
  if (times <= 0)
    times = 5;
  
  for (i = 0; i < times; i++) {
    /* first check the flags */
    
    if (! rxtx_only) {
      ioctl(cb->fd, TIOCMBIS, &rts_mask);
      ioctl(cb->fd, TIOCMBIC, &dtr_mask);
      ioctl(cb->fd, TIOCMGET, &flags);
    
      if (!(flags & TIOCM_CTS) && !(flags & TIOCM_DSR)) {
        printf("test failed: CTS and DSR not high when expected (RTS high, DTR low)\n");
        exit(EXIT_FAILURE);
      }
    
      if ((flags & TIOCM_CD) && (flags & TIOCM_RI)) {
        printf("test failed: CD and RI high when expected to be low (RTS high, DTR low)\n");
        exit(EXIT_FAILURE);
      }
        
      ioctl(cb->fd, TIOCMBIC, &rts_mask);
      ioctl(cb->fd, TIOCMBIS, &dtr_mask);
      ioctl(cb->fd, TIOCMGET, &flags);
    
      if (!(flags & TIOCM_CD) && !(flags & TIOCM_RI)) {
        printf("test failed: CD and RI not high when expected (RTS low, DTR high)\n");
        exit(EXIT_FAILURE);
      }
    
      if ((flags & TIOCM_CTS) && (flags & TIOCM_DSR)) {
        printf("test failed: CTS and DSR high when expected low (RTS low, DTR high)\n");
        exit(EXIT_FAILURE);
      }
    }
    
    tcflush(cb->fd, TCIOFLUSH);
    wcnt = write(cb->fd, tx, strlen(tx));

    /* The reason we fork here is if tx and rx are unconnected the read call
       will hang. The implementation below kills the read call if there is no response. */
    
    if ((pid = fork())) { /* Parent */
       /* The read call can take appr. 15000 loops. A wait of 40000 should be enough for the read o finnish. */ 
       wait = 40000;
       while (wait--) {
         ret = waitpid(pid, &status, WNOHANG);
         if (WIFEXITED(status) && (ret == pid)) {  
           rcnt = WEXITSTATUS(status);
           goto read_done;
         }
      }
      kill(pid, SIGTERM);
      waitpid(pid, 0, 0);
      rcnt = 0;                      
    }
    else { /* Child */
      rcnt = read(cb->fd, rx, 255);
      if (!strncmp(rx, tx, rcnt)) 
        exit(rcnt);
      else 
        exit(0);
    }

read_done:
    
    if (rcnt == strlen(tx))
      ;
    else {
      printf("%s test failed: read/write, %d/%d. run: %d.\n", cb->dev, rcnt, wcnt, i);
      exit(EXIT_FAILURE);
    } 
  }
  
  printf("test ok.\n");
  return rtrn;
}

void par_test(const char* dev)
{
#ifdef __CRIS__
  struct etrax_pario par;
  int devfd;
  int value;
  int port = atoi(&dev[strlen(dev)-1]);
  
  devfd = open(dev, O_RDWR);
  if (devfd > 0)
  {
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ioctl PARIO_GET_STRUCT"));
    }
    print_par(port, &par);
    printf("Setting active\n");
    IO_ASSIGN(par.config, R_PAR0_CONFIG, mode, manual);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, force, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, dma, disable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, oe, enable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, seli, active);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, autofd, active);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, strb, active);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, init, active);
    par.ctrl_data &= ~IO_MASK(R_PAR0_CTRL_DATA, data);
    par.ctrl_data |= 0;

    value = ioctl(devfd, PARIO_SET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ioctl PARIO_SET_STRUCT"));
    }
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    print_par(port, &par);
    printf("Setting inactive\n");
    IO_ASSIGN(par.config, R_PAR0_CONFIG, mode, manual);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, force, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, enable, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, dma, disable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, oe, enable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, seli, inactive);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, autofd, inactive);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, strb, inactive);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, init, inactive);
    par.ctrl_data &= ~IO_MASK(R_PAR0_CTRL_DATA, data);
    par.ctrl_data |= 0xFF;
    printf("ctrl: 0x%08lX config: 0x%08lX\n", par.ctrl_data, par.config);
    value = ioctl(devfd, PARIO_SET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ioctl PARIO_SET_STRUCT"));
    }
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    print_par(port, &par);

    close( devfd );
  }
  else
  {
    DERROR(perror("ERROR! Failed to open port"));
  }
#endif
}

int set_parXhi(int port)
{
#ifdef __CRIS__
  struct etrax_pario par;
  int devfd;
  int value;
  char dev[20];
  sprintf(dev, "/dev/lp%i", port);

  devfd = open(dev, O_RDWR);
  if (devfd > 0)
  {
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ioctl PARIO_GET_STRUCT"));
    }
    print_par(port, &par);
    printf("Setting high\n");
    IO_ASSIGN(par.config, R_PAR0_CONFIG, mode, manual);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, force, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, enable, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, dma, disable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, oe, enable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, seli, active);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, autofd, active);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, strb, active);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, init, active);
    par.ctrl_data &= ~IO_MASK(R_PAR0_CTRL_DATA, data);
    par.ctrl_data |= 0xFF;
    printf("ctrl: 0x%08lX config: 0x%08lX\n", par.ctrl_data, par.config);
    par.assign_ctrl = 1;
    par.assign_config = 1;
    
    value = ioctl(devfd, PARIO_SET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ioctl PARIO_SET_STRUCT"));
    }
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    print_par(port, &par);
    close(devfd);
    
  }
#endif  
  return 0;
}

void set_parXlo(int port)
{
#ifdef __CRIS__
  struct etrax_pario par;
  int devfd;
  int value;
  char dev[20];
  sprintf(dev, "/dev/lp%i", port);

  devfd = open(dev, O_RDWR);
  if (devfd > 0)
  {
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ioctl PARIO_GET_STRUCT"));
    }
    print_par(port, &par);
    printf("Setting low\n");
    IO_ASSIGN(par.config, R_PAR0_CONFIG, mode, manual);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, force, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, enable, on);
    IO_ASSIGN(par.config, R_PAR0_CONFIG, dma, disable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, oe, enable);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, seli, inactive);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, autofd, inactive);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, strb, inactive);
    IO_ASSIGN(par.ctrl_data, R_PAR0_CTRL_DATA, init, inactive);
    par.ctrl_data &= ~IO_MASK(R_PAR0_CTRL_DATA, data);
    par.ctrl_data |= 0x00;
    par.assign_ctrl = 1;
    par.assign_config = 1;
    printf("ctrl: 0x%08lX config: 0x%08lX\n", par.ctrl_data, par.config);
    value = ioctl(devfd, PARIO_SET_STRUCT, &par);
    if (value != 0)
    {
      DERROR(perror("ERROR! ioctl PARIO_SET_STRUCT"));
    }
    value = ioctl(devfd, PARIO_GET_STRUCT, &par);
    print_par(port, &par);
    close(devfd);
  }
#endif

}


int set_par0hi(struct Tcontext *cb, int argc, char* argv[])
{
  set_parXhi(0);
  return 0;
}

int set_par0lo(struct Tcontext *cb, int argc, char* argv[])
{
  set_parXlo(0);
  return 0;
}

int set_par1hi(struct Tcontext *cb, int argc, char* argv[])
{
  set_parXhi(1);
  return 0;
}

int set_par1lo(struct Tcontext *cb, int argc, char* argv[])
{
  set_parXlo(1);
  return 0;
}

int do_getinfospeedtest(struct Tcontext *cb, int argc, char* argv[])
{
  struct timeval tv0, tv1;
  int modem_state;
  int i;
  unsigned long cnt = 0;
  printf("Testing TIOCMGET..\n");
  gettimeofday(&tv1, NULL);
  do{
    gettimeofday(&tv0, NULL);
  } while (tv0.tv_sec == tv1.tv_sec );
  do {
    for (i = 0; i< 50; i++){
      ioctl(cb->fd, TIOCMGET, &modem_state);
      cnt++;
    }
    gettimeofday(&tv1, NULL);
  } while (tv0.tv_sec == tv1.tv_sec );
  printf("TIOCMGET %lu.%06lu-%lu.%06lu state: 0x%08lX cnt: %lu\n", 
	 tv0.tv_sec, tv0.tv_usec, tv1.tv_sec, tv1.tv_usec, 
	 (unsigned long)modem_state, cnt);

  printf("Testing TIOCMBIS/BIC..\n");
  modem_state = 0;
  gettimeofday(&tv1, NULL);
  do{
    gettimeofday(&tv0, NULL);
  } while (tv0.tv_sec == tv1.tv_sec );
  do {
    for (i = 0; i< 50; i++){
      modem_state++;
      ioctl(cb->fd, TIOCMBIS, &modem_state);
      modem_state++;
      ioctl(cb->fd, TIOCMBIC, &modem_state);
      cnt++;
    }
    gettimeofday(&tv1, NULL);
  } while (tv0.tv_sec == tv1.tv_sec );
  ioctl(cb->fd, TIOCMGET, &modem_state);
  printf("TIOCMBIS/BIC %lu.%06lu-%lu.%06lu state: 0x%08lX cnt: %lu\n", 
	 tv0.tv_sec, tv0.tv_usec, tv1.tv_sec, tv1.tv_usec, 
	 (unsigned long)modem_state, cnt);

  printf("Testing TIOCMSET..\n");
  modem_state = 0;
  gettimeofday(&tv1, NULL);
  do{
    gettimeofday(&tv0, NULL);
  } while (tv0.tv_sec == tv1.tv_sec );
  do {
    for (i = 0; i< 50; i++){
      modem_state++;
      ioctl(cb->fd, TIOCMSET, &modem_state);
      cnt++;
    }
    gettimeofday(&tv1, NULL);
  } while (tv0.tv_sec == tv1.tv_sec );
  ioctl(cb->fd, TIOCMGET, &modem_state);
  printf("TIOCMSET %lu.%06lu-%lu.%06lu state: 0x%08lX cnt: %lu\n", 
	 tv0.tv_sec, tv0.tv_usec, tv1.tv_sec, tv1.tv_usec, 
	 (unsigned long)modem_state, cnt);


  return 0;
}

/*#***************************************************************************
*# FUNCTION NAME: hexbyte
*#
*# PARAMETERS   : const char* s - String to get hex byte from
*#
*# RETURNS      : Byte value of the first two chars in s
*#
*# SIDE EFFECTS : None
*#
*# DESCRIPTION  : Returns the value of the two first (hexadecimal) chars in s.
*#                Non hex chars will give result 0 for that digit.
*#----------------------------------------------------------------------------
*# HISTORY
*#
*# DATE         NAME               CHANGES
*# ----         ----               -------
*# Mar  3 1999  Johan Adolfsson    Initial version
*#
*#***************************************************************************/
unsigned char hexbyte(const char *s)
{
  char c1 = toupper(s[0]);
  char c2 = toupper(s[1]);
  unsigned char sum = 0;
  
  if (isdigit(c1))
  {
    sum = c1 - '0';
  }
  else
  {
    if (('A' <= c1) && (c1 <= 'F'))
    {
      sum = 10 + c1 - 'A';
    }
  }
  /* Shift up Most Significant hexdigit */
  sum = sum << 4;
  
  if (isdigit(c2))
  {
    sum += c2 - '0';
  }
  else
  {
    if (('A' <= c2) && (c2 <= 'F'))
    {
      sum += 10 + c2 - 'A';
    }
  }
  return sum;
} /* hexbyte */

/* Create and retur an argv vector based on query.
 * query is modified and the returned argv contains pointers to it
 */
char** query_to_argv(char *argv0, char *query, int *argc_p)
{
  
  int num_arg = 1;
  const char *s = query;
  char **myargv;
  do
  {
    s++;
    
    num_arg++;
    s = strchr(s, '&');
  } while(s!=NULL);
  
  printf("Split query to build argv %i...\n", num_arg);
  myargv = malloc(sizeof(char*) * (num_arg + 1));
  if (myargv != NULL)
  {
    int i = 0;
    char *ws = query; /* where we write */
    char *rs = ws;    /* where we read */
    myargv[i++] = argv0;
    myargv[i++] = ws;
    while(*rs)
    {
      switch (*rs)
      {
       case '+':
        *ws++ = ' ';
        rs++;
        break;
        
       case '&':
        *ws++ = '\0';
        rs++;
        myargv[i++] = ws;
        break;
        
       case '%':
        rs++;
        *ws++ = hexbyte(rs);
        rs += 2;
        break;
        
       default:
        *ws++ = *rs++;
        break;
      } /* switch */
    }
    *ws = '\0';
#if 0
    printf("New args:\n");
    for(i=1; i < num_arg; i++)
    {
      printf("arg %i: '%s'\n", i, myargv[i]);
    }
#endif
  }
  *argc_p = num_arg;
  return myargv;
  
}

void break_handler(int s)
{
	printf("BREAK!\n");
	signal(SIGINT, break_handler);
}


/*#**************************************************************************
*#                                                            
*# FUNCTION NAME: main SAIH
*#                                                            
*# DESCRIPTION  : check flags and calls requested functions
*#                                                            
*#**************************************************************************/

int TtyFunc(int argc, char *argv[])
{
	int arg = 1;
	int i;

	#if 0
		char **myargv = NULL;
		char *query  = getenv("QUERY_STRING");
	#endif  
	const char *method = getenv("REQUEST_METHOD");

	if (method != NULL){
		useHTML = 1;
		/* CGI call */
		printf("Cache-Control: no-cache\r\n");
		printf("Pragma: no-cache\r\n");
		printf("Expires: Thu, 01 Dec 1994 16:00:00 GMT\r\n");
		printf("Content-type: text/plain\r\n");
		printf("\r\n");
	}
	signal(SIGINT, break_handler);
  
if (useHTML){
		#if 0
		printf("<HTML><BODY>\n");
		printf("<pre>\n");
		#endif
		#if 0
		{
			int i = 0;
			printf("argc: %i\n", argc);
			printf("QUERY_STRING: %s\n", query);
			for(i=1; i < argc; i++){
				printf("arg %i: %s\n", i, argv[i]);
			}
		}
							//Apache doesn't split query into argv like boa does 
		if (argc == 2 && query && strlen(query)>0){
			int myargc = 0;
			myargv = query_to_argv(argv[0], query, &myargc);
			if (myargv){
				argv = myargv;
				argc = myargc;
			}
		}
		#endif  
}

	context.fd = 0;
	context.dev="stdin";
	context.arg = arg;
	context.argc = argc;
	context.argv = argv;
	while (arg < argc) {
		i = 0;
		context.arg = arg;
		while (cmd_table[i].param != NULL && strcmp(cmd_table[i].param, argv[arg])!= 0){
			i++;
		}
		if (cmd_table[i].param == NULL){
			if (strcmp(argv[arg], "-h") == 0){
				usage();}
			else{
				printf("\n\tRS:Unknown argument: %s\n",argv[arg]);
				usage();}
		}
		else{						//Process parameter
			if (cmd_table[i].func != NULL){		//se trata de manera diferente la funcion saih
				//printf("\n\tRS232:Funcion[%d]: %s\n",i,cmd_table[i].param);
				//sleep(1);
				if (!strcmp(cmd_table[i].param,"-rxsaih")){
					int leidos = do_rxsaih(&context, argc-arg, &argv[arg+1],1,&rx);
					//printf("\n\tTty:Lectura Respuesta: %d\n", leidos);
					//printf("\n\tTty:Bytes leidos RX %d\n",rx.bytesleidos);
					return 1;
				}
				arg += cmd_table[i].func(&context, argc-arg, &argv[arg+1]);
			}
			else{
				printf("\n\tRS:%s not supported yet\n",cmd_table[i].param);
			}
		}
		arg++;
	}
	#if 0
	    if (useHTML){
		printf("</pre>\n");
		printf("</BODY></HTML>\n");
	    }
	#endif
	return 0;
}

/*#***************************************************************
*# Rutina Lectura RS232 SAIH,
*# devuelve un array de caracteres con la respuesta de la remota
*#***************************************************************/


int do_rxsaih(struct Tcontext *cb, int argc, char* argv[], int print_hex, RX *rx)
{
	unsigned long timeleft = atol(argv[0]);
	unsigned long strict_time_us = 0;
	unsigned char buf[NBR];
	fd_set readfds;
	int selcode;
	int size,leidos=0;
	struct timeval timeout;
	struct timeval t0, t1;

	//printf("\n\tDO_RXSAIH");

	TimeWait(5);

	if (argv[0][0] == '+'){
		strict_time_us = timeleft*1000000;
	}
  
	DREAD(printf("\n\tDO_RXSAIH:Receive in %lu seconds\n", timeleft));

	if (cb->fd < 0){
		return 1;}

 	gettimeofday(&t0, NULL);

	while (timeleft){
		FD_ZERO(&readfds);
		FD_SET(cb->fd, &readfds);
		timeout.tv_sec=1;
		timeout.tv_usec=0;
		if ((selcode = select(cb->fd + 1,&readfds,NULL,NULL,&timeout)) != 0){
			if (selcode == -1){				// Something happend 
				DERROR(perror("ERROR! select(): "));
			}
			else{						// Hay Posibilidad de Leer
				if (FD_ISSET(cb->fd, &readfds)){
					size = read(cb->fd, buf, sizeof(buf)-1);
				}
				memcpy(rx->bufrecv+leidos,buf,size);
				leidos=leidos+size;
			if(DEBUG){printf("\n\tDO_RXSAIH:Leidos %lu Bytes\n", leidos);}
			}
		}
		else{							// No hay Evento en el Tiempo del Select
			timeleft--;
			// fflush(stdout);
		}
		if (strict_time_us){
			gettimeofday(&t1, NULL);
			if (((t1.tv_sec - t0.tv_sec)*1000000 +t1.tv_usec - t0.tv_usec) > strict_time_us)
				timeleft = 0;
		}
	} /* while */

	rx->bytesleidos=leidos;
	close(cb->fd);

	return 1;
}

/*#***************************************************************
*# Rutina UtilTty
*# realiza las operaciones serie tty solicitadas
*#***************************************************************/

int DebugTty(int NumArg, char *ConfTty[NCConf])
{

	int i=1;
	while (i < NumArg){
		printf("Llamada UtilTty: arg %d: %s %s\n",i,ConfTty[i],ConfTty[i+1]);
		i=i+2;
	}
	
	printf("\n\tDebugTty:WRITE:");
        for(i=0;i<30;i++){
                printf("%02x ",ConfTty[10][i]);}
        printf("\n");


	/* Llamada a TtyFunc con los parametros de confiduracion */
	int Respuesta = TtyFunc(NumArg,ConfTty);

	printf("\nRespuesta UtilTty: %d\n",Respuesta);
	printf("\nAxisLoger: leidos %d bytes\n",rx.bytesleidos);

	return Respuesta;
}
