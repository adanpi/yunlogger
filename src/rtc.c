#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#ifdef __CRIS__
#include <asm/rtc.h>
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


#define MONTH_STR(m) (((m)>=0 && (m)<12)?month[(m)]:"???")

static const char *month[] = { "Ene", "Feb", "Mar", "Apr", "May",
                               "Jun", "Jul", "Ago", "Sep", "Oct",
                               "Nov", "Dic" };


/*#**************************************************************************
*#                                                            
*# FUNCTION NAME: set_time
*#                                                            
*# DESCRIPTION  : Set the time using the arguments provided
*#                return number of arguments consumed.
*#                                                            
*#**************************************************************************/
int
set_time(struct tm NewTim)
{
	int done = 0;
	int arg = 0;
#ifdef __CRIS__
	int fd, fio;
	struct rtc_time rt;
	struct tm kt; /* kernel time */
	struct timeval new_tv;
#else
	time_t now;
	struct tm *absTime;
	struct tm rt;
	
	(void) time (&now);
        absTime = localtime(&now);

	// Init the time structure with previous values:
	rt.tm_year = absTime->tm_year;
	rt.tm_mon = absTime->tm_mon;
	rt.tm_mday = absTime->tm_mday;
	rt.tm_hour = absTime->tm_hour;
	rt.tm_min = absTime->tm_min;
	rt.tm_sec = absTime->tm_sec;
#endif
#ifdef __CRIS__
	// Open the rtc driver
	fd = open("/dev/rtc", O_RDONLY);
	if(fd < 0) {
		printf("Open rtc device Error\n");
		perror("hardwaretest: open");
		return 1;
	}
	// Init the time structure
	rt.tm_year = 0;
	rt.tm_mon = 0;
	rt.tm_mday = 0;
	rt.tm_hour = 0;
	rt.tm_min = 0;
	rt.tm_sec = 0;


	/* get the RTC time */

	fio = ioctl(fd, RTC_RD_TIME, &rt); 
	if(fio < 0) {
		printf("\n\tError reading new time %d\n", fio);
		perror("\n\thardwaretest : ioctl RTC_RD_TIME");
		close(fd);
		return 1;
	}
#endif
	//asignacion nueva fecha hora
	rt.tm_year = NewTim.tm_year;
	rt.tm_mon = NewTim.tm_mon;
	rt.tm_mday = NewTim.tm_mday;
	rt.tm_hour = NewTim.tm_hour;
	rt.tm_min = NewTim.tm_min;
	rt.tm_sec = NewTim.tm_sec;

	/*printf("\n\tWrote time : %s %d %2d:%02d:%02d %d\r\n",
		       MONTH_STR(rt.tm_mon),
		       rt.tm_mday,
		       rt.tm_hour,
		       rt.tm_min,
		       rt.tm_sec,
		       rt.tm_year+1900 );***/
			
#ifdef __CRIS__   

	/* copy rtc_time to tm struct */
	kt.tm_year = rt.tm_year;
	kt.tm_mon  = rt.tm_mon;
	kt.tm_mday = rt.tm_mday;
	kt.tm_hour = rt.tm_hour;
	kt.tm_min  = rt.tm_min;
	kt.tm_sec  = rt.tm_sec;
	kt.tm_isdst = 0;  /* daylight savings time */

	/* tm_wday and tm_yday are ignored by mktime */  

	new_tv.tv_sec = (unsigned long)mktime(&kt);
	new_tv.tv_usec = 0;

	/* set the kernel time */

	settimeofday(&new_tv, NULL);


	fio = ioctl(fd, RTC_SET_TIME, &rt); 
	if(fio < 0) {
		printf("\n\tError writing new time %d\n", fio);
		perror("\n\thardwaretest : ioctl RTC_SET_TIME");
		close(fd);
		return 1;
	}
	close(fd);
#endif
	printf("\n\tACTUALIZADA FECHA-HORA SYSTEMA : %s %d %2d:%02d:%02d %d\r\n",
		       MONTH_STR(rt.tm_mon),
		       rt.tm_mday,
		       rt.tm_hour,
		       rt.tm_min,
		       rt.tm_sec,
		       rt.tm_year+1900 );

	return arg;
} /* set_time */

