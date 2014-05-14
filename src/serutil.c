/*!***************************************************************************
*!
*! FILE NAME  : serutil.c
*!
*! DESCRIPTION: Serial port functions.
*!
*! FUNCTIONS  : Many...
*! (EXTERNAL)
*!
*! FUNCTIONS  : 
*! (LOCAL)
*!
*! ---------------------------------------------------------------------------
*! HISTORY
*!
*! DATE         NAME               CHANGES
*! ----         ----               -------
*! Aug  5 1999  Johan Adolfsson    Initial version
*! $Log: serutil.c,v $
*! Revision 1.4  2001/01/16 11:53:12  johana
*! Fixed get_port_Flowcontrol()
*!
*! Revision 1.3  2000/09/04 12:54:47  johana
*! Fixed initialisation, added some error detection
*!
*! 
*! ---------------------------------------------------------------------------
*! (C) Copyright 1999, 2000 Axis Communications AB, LUND, SWEDEN
*!***************************************************************************/
/* $Id: serutil.c,v 1.4 2001/01/16 11:53:12 johana Exp $ */

/****************** INCLUDE FILES SECTION ***********************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>
#include <termios.h>
#include <termio.h>

/*#include "compiler.h"*/
#include "serutil.h"

/****************** CONSTANT AND MACRO SECTION ******************************/

#define LogMsg(level, string) fprintf(stderr, string)
#define FALSE 0
#define TRUE !FALSE

/*#define TCSA_MODE TCSADRAIN*/
#define TCSA_MODE TCSANOW
/****************** TYPE DEFINITION SECTION *********************************/

/****************** LOCAL FUNCTION DECLARATION SECTION **********************/

/****************** GLOBAL VARIABLE DECLARATION SECTION *********************/

/****************** LOCAL VARIABLE DECLARATION SECTION **********************/

int BreakSignaled = FALSE;

/****************** FUNCTION DEFINITION SECTION *****************************/


struct tcspeed{
	unsigned long baud;
	speed_t code;
};

static struct tcspeed tcspeeds[] = {
#ifdef B50
	{50, B50},
#endif
#ifdef B75
	{75, B75},
#endif
#ifdef B110
	{110, B110},
#endif
#ifdef B134
	{134, B134},
#endif
#ifdef B150
	{150, B150},
#endif
#ifdef B200
	{200, B200},
#endif
#ifdef B300
	{300, B300},
#endif
#ifdef B600
	{600, B600},
#endif
#ifdef B1200
	{1200, B1200},
#endif
#ifdef B1800
	{1800, B1800},
#endif
#ifdef B2400
	{2400, B2400},
#endif
#ifdef B4800
	{4800, B4800},
#endif
#ifdef B9600
	{9600, B9600},
#endif
#ifdef B19200
	{19200, B19200},
#endif
#ifdef B38400
	{38400, B38400},
#endif
#ifdef B57600
	{57600, B57600},
#endif
#ifdef B115200
	{115200, B115200},
#endif
#ifdef B230400
	{230400, B230400},
#endif
#ifdef B460800
	{460800, B460800},
#endif
#ifdef B921600
	{921600, B921600},
#endif
#ifdef B1843200
	{1843200, B1843200},
#endif
#ifdef B6250000
	{6250000, B6250000},
#endif
#ifdef B0
	{0, B0},
#endif
	{0, 0}
};



/* Retrieves the port speed from port_fd */
unsigned long int get_port_Speed(int port_fd)
{
  struct termios port_info;
  speed_t speed;
  int i = 0;
  
  if (tcgetattr(port_fd, &port_info) != 0){
    printf("Error %i %s\n", errno, strerror(errno));
  }
  
  speed = cfgetospeed(&port_info);
  while (tcspeeds[i].code != 0 && tcspeeds[i].code != speed)
  {
    i++;
  }
  if (tcspeeds[i].code == speed)
  {
    return tcspeeds[i].baud;
  }
  return 0L;
}

/* Retrieves the data size from port_fd */
unsigned char get_port_DataSize(int port_fd)
{
  struct termios port_info;
  tcflag_t datasize;

  if (tcgetattr(port_fd, &port_info) != 0){
    printf("Error %i %s\n", errno, strerror(errno));
  }
  datasize = port_info.c_cflag & CSIZE;
  /* It's a straight mapping here */
  switch (datasize)
  {
   case CS5:
    return((unsigned char) 5);
   case CS6:
    return((unsigned char) 6);
   case CS7:
    return((unsigned char) 7);
   case CS8:
    return((unsigned char) 8);
   default:
    return((unsigned char) 0);
  }
}

/* Retrieves the parity settings from port_fd */
unsigned char get_port_Parity(int port_fd)
{
  struct termios port_info;

  if (tcgetattr(port_fd, &port_info) != 0){
    printf("Error %i %s\n", errno, strerror(errno));
  }
  if ((port_info.c_cflag & PARENB) == 0)
    return((unsigned char) TNET_COM_PARITY_NONE);

  if ((port_info.c_cflag & PARENB) != 0 &&
      (port_info.c_cflag & PARODD) != 0)
    return((unsigned char) TNET_COM_PARITY_ODD);

/* NOTE: How handle TNET_COM_PARITY_MARK  and TNET_COM_PARITY_SPACE ? */
  return((unsigned char) TNET_COM_PARITY_EVEN);
}

/* Retrieves the stop bits size from port_fd */
unsigned char get_port_StopSize(int port_fd)
{
  struct termios port_info;

  if (tcgetattr(port_fd, &port_info) != 0){
    printf("Error %i %s\n", errno, strerror(errno));
  }
  if ((port_info.c_cflag & CSTOPB) == 0)
  {
    
    return((unsigned char) TNET_COM_STOPSIZE1);
  }
  else
  {
    return((unsigned char) TNET_COM_STOPSIZE2);
  }
  
/* NOTE: How handle TNET_COM_STOPSIZE1_5 */
} /* get_port_StopSize */
  
/* Retrieves the flow control status, including DTR and RTS status,
  from port_fd */
unsigned char get_port_FlowControl(int port_fd, unsigned char Which)
{
  struct termios port_info;
  int MLines;

  /* Gets the basic informations from the port */
  if (tcgetattr(port_fd, &port_info) != 0){
    printf("Error %i %s\n", errno, strerror(errno));
  }
  ioctl(port_fd, TIOCMGET, &MLines);

  /* Check wich kind of information is requested */
  switch (Which)
  {
    /* Com Port Flow Control Setting (outbound/both) */
   case TNET_COM_SET_CONTROL_REQUEST_FLOW:
  /* XON/XOFF can really be at the same time as RTSCTS */
    if ((port_info.c_iflag & IXON))
      return((unsigned char) TNET_COM_SET_CONTROL_FLOW_XONXOFF);
    if ((port_info.c_cflag & CRTSCTS))
      return((unsigned char) TNET_COM_SET_CONTROL_FLOW_RTSCTS);
    return((unsigned char) 0);
    break;
    
    /* BREAK State  */
   case TNET_COM_SET_CONTROL_REQUEST_BREAK:
    if (BreakSignaled == TRUE)
      return((unsigned char) TNET_COM_SET_CONTROL_BREAK_ON);
    else
      return((unsigned char) TNET_COM_SET_CONTROL_BREAK_OFF);
    break;
        
    /* DTR Signal State */
   case TNET_COM_SET_CONTROL_REQUEST_DTR:
    if ((MLines & TIOCM_DTR) == 0)
      return((unsigned char) TNET_COM_SET_CONTROL_DTR_OFF );
    else
      return((unsigned char) TNET_COM_SET_CONTROL_DTR_ON );
    break;

    /* RTS Signal State */
   case TNET_COM_SET_CONTROL_REQUEST_RTS:
    if ((MLines & TIOCM_RTS) == 0)
      return((unsigned char) TNET_COM_SET_CONTROL_RTS_OFF);
    else
      return((unsigned char) TNET_COM_SET_CONTROL_RTS_ON);
    break;

    /* Com Port Flow Control Setting (inbound) */
   case TNET_COM_SET_CONTROL_REQUEST_FLOWIN:
    if ((port_info.c_iflag & IXOFF))
      return((unsigned char) TNET_COM_SET_CONTROL_FLOWIN_XOFF );
    if ((port_info.c_cflag & CRTSCTS))
      return((unsigned char) TNET_COM_SET_CONTROL_FLOWIN_RTSCTS );
    return((unsigned char) TNET_COM_SET_CONTROL_FLOWIN_NONE);
    break;

   default:
    if ((port_info.c_iflag & IXON))
      return((unsigned char) TNET_COM_SET_CONTROL_FLOW_XONXOFF);
    if ((port_info.c_cflag & CRTSCTS))
      return((unsigned char) TNET_COM_SET_CONTROL_FLOW_RTSCTS);
    return((unsigned char) 0);
    break;
  }
} /* get_port_FlowControl */

/* Return the status of the modem control lines (CD, CTS, DSR, RI) */
unsigned char get_port_state(int port_fd, unsigned char PMState)
{
  int MLines;
  unsigned char MState = (unsigned char) 0;
    
  ioctl(port_fd, TIOCMGET, &MLines);

  if ((MLines & TIOCM_CAR) != 0)
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_CD;
  }
  if ((MLines & TIOCM_RNG) != 0)
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_RI;
  }
  if ((MLines & TIOCM_DSR) != 0)
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_DSR;
  }
  if ((MLines & TIOCM_CTS) != 0)
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_CTS;
  }

  if ((MState & TNET_COM_MODEMSTATE_CD) != 
      (PMState & TNET_COM_MODEMSTATE_CD))
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_CD_DELTA;
  }
  if ((MState & TNET_COM_MODEMSTATE_RI) != 
      (PMState & TNET_COM_MODEMSTATE_RI))
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_RI_TRAIL;
  }
  if ((MState & TNET_COM_MODEMSTATE_DSR) != 
      (PMState & TNET_COM_MODEMSTATE_DSR))
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_DSR_DELTA;
  }
  if ((MState & TNET_COM_MODEMSTATE_CTS) != 
      (PMState & TNET_COM_MODEMSTATE_CTS))
  {
    MState += (unsigned char) TNET_COM_MODEMSTATE_CTS_DELTA;
  }
  
  return (MState);
}

/* Set the serial port data size */
void set_port_DataSize(int port_fd, unsigned char DataSize)
{
  struct termios port_info;
  tcflag_t PDataSize;

  switch (DataSize)
  {
   case 5:
    PDataSize = CS5;
    break;
   case 6:
    PDataSize = CS6;
    break;
   case 7:
    PDataSize = CS7;
    break;
   case 8:
    PDataSize = CS8;
    break;
   default:
    PDataSize = CS8;
    break;
  }

  tcgetattr(port_fd, &port_info);
  port_info.c_cflag = port_info.c_cflag &
    ((port_info.c_cflag & ~CSIZE) | PDataSize);
  tcsetattr(port_fd, TCSA_MODE, &port_info);
}

/* Set the serial port parity */
void set_port_Parity(int port_fd, unsigned char Parity)
{
  struct termios port_info;

  tcgetattr(port_fd, &port_info);

  switch (Parity)
  {
   case TNET_COM_PARITY_NONE:
    port_info.c_cflag = port_info.c_cflag & ~PARENB;
    break;
   case TNET_COM_PARITY_ODD :
    port_info.c_cflag = port_info.c_cflag | PARENB | PARODD;
    break;
   case TNET_COM_PARITY_EVEN:
    port_info.c_cflag = (port_info.c_cflag | PARENB) & ~PARODD;
    break;
    /* There's no support for MARK and SPACE parity so sets no parity */
   case TNET_COM_PARITY_MARK:
   case TNET_COM_PARITY_SPACE:
   default:
    LogMsg(LOG_WARNING,"Requested unsupported parity. Set to no parity");
    port_info.c_cflag = port_info.c_cflag & ~PARENB;
    break;
  }

  tcsetattr(port_fd, TCSA_MODE, &port_info);
}

/* Set the serial port stop bits size */
void set_port_StopSize(int port_fd, unsigned char StopSize)
{
  struct termios port_info;

  tcgetattr(port_fd, &port_info);

  switch (StopSize)
  {
   case 1:
    port_info.c_cflag = port_info.c_cflag & ~CSTOPB;
    break;
   case 2:
    port_info.c_cflag = port_info.c_cflag | CSTOPB;
    break;
   case 3:
    port_info.c_cflag = port_info.c_cflag & ~CSTOPB;
    LogMsg(LOG_WARNING,"Requested unsupported 1.5 bits stop size. "
           "Set to 1 bit stop size");
    break;
   default:
    port_info.c_cflag = port_info.c_cflag & ~CSTOPB;
    break;
  }

  tcsetattr(port_fd, TCSA_MODE, &port_info);
}

/* Set the port flow control and DTR and RTS status */
void set_port_FlowControl(int port_fd, unsigned char flow_control)
{
  struct termios port_info;
  int MLines;

  /* Gets the base status from the port */
  tcgetattr(port_fd, &port_info);
  ioctl(port_fd, TIOCMGET, &MLines);

  /* Check which settings to change */
  switch (flow_control)
  {
    /* No Flow Control (outbound/both) */
   case TNET_COM_SET_CONTROL_FLOW_NONE:
    port_info.c_iflag = port_info.c_iflag & ~IXON;
    port_info.c_iflag = port_info.c_iflag & ~IXOFF;
    port_info.c_cflag = port_info.c_cflag & ~CRTSCTS;
    break;
    /* XON/XOFF Flow Control (outbound/both) */
   case TNET_COM_SET_CONTROL_FLOW_XONXOFF:
    port_info.c_iflag = port_info.c_iflag | IXON;
    port_info.c_iflag = port_info.c_iflag | IXOFF;
    break;
    /* HARDWARE Flow Control (outbound/both) */
   case TNET_COM_SET_CONTROL_FLOW_RTSCTS:
    port_info.c_cflag = port_info.c_cflag | CRTSCTS;
    break;
    /* BREAK State ON */
   case TNET_COM_SET_CONTROL_BREAK_ON:
    tcsendbreak(port_fd, 0);
    BreakSignaled = TRUE;
    break;
    /* BREAK State OFF */
   case TNET_COM_SET_CONTROL_BREAK_OFF:
    tcsendbreak(port_fd, 0);
    BreakSignaled = FALSE;
    break;
    /* DTR Signal State ON */
   case TNET_COM_SET_CONTROL_DTR_ON:
    MLines = MLines | TIOCM_DTR;
    break;
    /* DTR Signal State OFF */
   case TNET_COM_SET_CONTROL_DTR_OFF:
    MLines = MLines & ~TIOCM_DTR;
    break;
    /* RTS Signal State ON */
   case TNET_COM_SET_CONTROL_RTS_ON:
    MLines = MLines | TIOCM_RTS;
    break;
    /* RTS Signal State OFF */
   case TNET_COM_SET_CONTROL_RTS_OFF:
    MLines = MLines & ~TIOCM_RTS;
    break;
    /* No Flow Control (inbound) */
   case TNET_COM_SET_CONTROL_FLOWIN_NONE:
    port_info.c_iflag = port_info.c_iflag & ~IXOFF;
    break;
    /* XON/XOFF Flow Control (inbound) */
   case TNET_COM_SET_CONTROL_FLOWIN_XOFF:
    port_info.c_iflag = port_info.c_iflag | IXOFF;
    break;
    /* HARDWARE Flow Control (inbound) */
   case TNET_COM_SET_CONTROL_FLOWIN_RTSCTS:
    port_info.c_cflag = port_info.c_cflag | CRTSCTS;
    break;
   default:
    LogMsg(LOG_WARNING,"Requested unsupported flow control, "
           "setting to no flow control");
    port_info.c_iflag = port_info.c_iflag & ~IXON;
    port_info.c_iflag = port_info.c_iflag & ~IXOFF;
    port_info.c_cflag = port_info.c_cflag & ~CRTSCTS;
    break;
  }
    
  tcsetattr(port_fd, TCSA_MODE, &port_info);
  ioctl(port_fd, TIOCMSET, &MLines);
}

/* Set the serial port speed */ 
void set_port_Speed(int port_fd, unsigned long baud_rate)
{
  struct termios port_info;
  speed_t speed;
  int i = 0;
  long min_diff = 0x7FFFFFF;
  long diff;
  
  int min_ix = 0;
  
  
  while (tcspeeds[i].code != 0 && tcspeeds[i].baud != baud_rate)
  {
    diff = labs(tcspeeds[i].baud - baud_rate);
    if (diff < min_diff)
    {
      min_diff = diff;
      min_ix = i;
    }
    i++;
  }
  if (tcspeeds[i].baud == baud_rate)
  {
    speed = tcspeeds[i].code;
  }
  else
  {
    LogMsg(LOG_WARNING,"Unknwon baud rate requested. Setting to nearest");
    
    speed = tcspeeds[min_ix].code;
  }
  //printf("\n\tSet_Port_Speed: %lu %lu\n", baud_rate, (unsigned long)speed);
  
  tcgetattr(port_fd, &port_info);
  cfsetospeed(&port_info, speed);
  cfsetispeed(&port_info, speed);
  tcsetattr(port_fd, TCSA_MODE, &port_info);
}



/****************** END OF FILE serutil.c ***********************************/

