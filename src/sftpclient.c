/* sftpclient (Simple ftp client)
 *
 * $Id: sftpclient.c,v 1.101 2008-03-07 16:50:44 martinad Exp $
 *
 * Written by Bjorn Wesen
 *
 * (C) Copyright 1999-2007, Axis Communications AB, LUND, SWEDEN
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * For syntax, see usage().
 *
 * If username is not specified, anonymous is used, with password "ftpuser@"
 *
 * TODO:
 *
 * -g (get) is not implemented.
 *
 * It would probably be nicer to parse an URL-style ftp specification like
 * ftp://bjornw:password@server.com/incoming/pic.jpg:21
 *
 * Responses from the server:
 * 220 hostname FTP server ready.
 * 221 Goodbye.
 * 331 Password required for user.
 * 230 User xxx logged in.
 * 250 Command successful
 * 257 Informative response
 * 502 Command not implemented.
 * 521 Directory exists.
 * 530 Incorrect login.
 * 1yz   Positive Preliminary reply
 * 2yz   Positive Completion reply
 * 3yz   Positive Intermediate reply
 * 4yz   Transient Negative Completion reply
 * 5yz   Permanent Negative Completion reply
 */

#define _GNU_SOURCE

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <dirent.h>
#include <crypt.h>
#include <pwd.h>
#include <netdb.h>
#include <getopt.h>
#include <syslog.h>
#include <arpa/inet.h>

#if USE_BUFFER_HANDLER
#  include <libbhand.h>
#  include <buffer_handler_helper.h>
#else
#  define buffer_handler_help_init(x)
#  define buffer_handler_file_is_ok(x)   TRUE
#endif

#ifdef USE_FORMAT_NAME
#include <formatname.h>
#endif

/* The assert() macro can be locally enabled here if you set
 * USE_LOCAL_ASSERT to non-zero (i e undefine NDEBUG _before_
 * assert.h is #include:d). Otherwise assert() is used if compilation
 * is done for debug target OR if you run make/gcc/gcc-cris with your
 * own set of CFLAGS not defining NDEBUG. In other cases assert()
 * should not be enabled.
 * And beware of bug in uC-libc and uClibc versions of assert.h..
 */
/* Compensate for bug: */
#ifdef __ASSERT_H
#  undef __ASSERT_H
#  undef assert
#endif /* assert.h */

#define USE_LOCAL_ASSERT 0
#ifdef NDEBUG
#  if USE_LOCAL_ASSERT
#    undef NDEBUG
#  endif
#endif
#include <assert.h>
#ifndef NDEBUG
#  warning **** The assert() macro is enabled!
#endif

#define FALSE (1 == 0)
#define TRUE  (!FALSE)

#define FTP_BUFFER_SIZE   4096
#define MAX_RESPONSE_SIZE 1280
#define MAX_CMD_SIZE       256
#define FILE_NM_SIZ        256
#define RESPONSE_TIMEOUT    60  /* Seconds to waits for server. */
#define SHORT_TIMEOUT        2  /* Timeout for opening fifo. */

#define NOT_RETRY    FALSE

#define MPUT_ENABLED 0

#define NO_ERROR         (0)
#define ERROR_NEG        (-1)
#define ERROR_LOGGED_OUT (-2)
#define ERR_GENERAL      (1)
#define ERR_INIT_CONNECT (10)

#define ARG_AF_INET  1
#define ARG_AF_INET6 2

#define DBG_ON 0 /* Do not check in non-zero value. */
#if DBG_ON
#  define D(x) x
#  define D2(x)
#  warning Debug is ON!
#else
#  define D(x)
#  define D2(x)
#endif
#define TRACE_ON 0 /* Do not check in non-zero value. */
#if TRACE_ON
#  define TRACED(x) x
#  warning TRACE is ON!
#else
#  define TRACED(x)
#endif
#define REOPEN_STDERR 0 /* Do not check in non-zero value. */
#if REOPEN_STDERR
#  warning Will reopen stderr!
#endif

#define PRG_NAME "FTPclient "

/* Choose your favourite file name format for the debug trace: */
#  define THIS_FILE  "sftpclient.c"
/*#define THIS_FILE */ /* No name at all. */
#ifndef THIS_FILE
#  define THIS_FILE __FILE__
#endif

/* To get a time stamp in the debug trace: */
#define TIME_STAMP_DBG 0
#if TIME_STAMP_DBG
#  define TIME_STR "%s "
#  define TIME_FUNC dbg_time(),
#else
#  define TIME_STR
#  define TIME_FUNC
#endif
/* To get a pid stamp in the debug trace: */
#define PID_DBG 1
#if PID_DBG
#  include <sys/types.h>
#  include <unistd.h>
#  define PID_STR "[%d] "
#  define PID_FUNC get_pid(),
   /* Either use std getpid() or some get_pid() of your own: */
/*#  define get_pid getpid*/
#else
#  define PID_STR
#  define PID_FUNC
#endif
/* To get file name and line number in debug trace: */
#define LOC_STR_DEBUG 1
#if LOC_STR_DEBUG
#  define XSTR(s) STR(s)
#  define STR(s) #s
#  ifndef LOC_STR
#    define LOC_STR THIS_FILE ": line " XSTR(__LINE__) ": "
#  endif /* LOC_STR */
#else
#  define LOC_STR
#endif

/* To get debug trace in log-file: */
#define SYSLOG_DBG 0

#if SYSLOG_DBG
#  define dbg_printf(n, a...) \
		syslog(LOG_INFO, LOC_STR ": %s " n, __FUNCTION__, ## a)
#else
#  define dbg_printf(n, a...) \
	do { \
		fprintf(stderr, PRG_NAME PID_STR TIME_STR ": %s(), " LOC_STR, PID_FUNC TIME_FUNC __FUNCTION__); \
		fprintf(stderr, n "\n", ## a); \
	} while (0)
#endif

#define write_error(n, a...)					\
	if (usesyslog) {					\
		syslog(LOG_ERR, n, ## a);			\
	} else {						\
		fprintf(stderr, n, ## a);			\
		fprintf(stderr, "\n");				\
	}

#define write_warning(n, a...)					\
	if (usesyslog) {					\
		syslog(LOG_WARNING, n, ## a);			\
	} else {						\
		fprintf(stderr, n, ## a);			\
		fprintf(stderr, "\n");				\
	}

enum {
	FTP_PUT,
	FTP_MPUT,
	FTP_MPUT_BHAND,
	FTP_INTERACTIVE,
	FTP_GET
};

struct server_par {
	char *hostname;
	char *username;
	char *password;
	char *remote_dir;
	char *port;
	int   passive;
};

struct s_server {
	char  srv_addr[INET6_ADDRSTRLEN]; /* address of the server           */
	int   srv_family;                 /* server address family           */
	int   ctrlfd;                     /* servers control connection fd   */
	FILE *ctrlfile;                   /* same thing but buffered version */
	char  my_addr[INET6_ADDRSTRLEN];  /* address of the client           */
	int   passive;
	unsigned short dataport;          /* server data port (passive mode) */
};

static int write_to_fd(int         fd,
		       const char *buf,
		       int         len);
static int wait_response(struct s_server *server,
			 const char      *wanted,
			 int              wantlen,
			 int              log_error);
static int do_connect_and_login(struct server_par *par, 
				struct s_server *server);
static int do_cd(struct s_server *server, char *path);
static int do_rename(struct s_server *server,
		     const char      *old_name,
		     const char      *new_name);
static int do_put(struct s_server *server,
		  const char      *local_file,
		  const char      *remote_file,
		  const char      *tmp_file,
		  struct timeval  *grab_time,
		  int              is_retrying);
static int do_mkpath(struct s_server *server,
		     const char      *path);

static void qos_setup_socket(int af, int sock, int dsfield, int prio);
static void cleanup(void);
static void err_exit(void);
static void sigpipe_handler(int signum);
static void sigterm_handler(int signum);
static void response_timeout_action(void);
static int was_logged_out(const char *rbuf);
static int connect_procedure(struct server_par *primary_ptr,
			     struct server_par *secondary_ptr,
			     struct s_server   *server_ptr);
static int open_connection(int sockfd, 
				const struct sockaddr *serv_addr, 
				socklen_t addrlen);

#ifdef USE_BUFFER_HANDLER
static int mput_buffer_handler(char* buffer_id,
                               char *remote_file,
                               struct s_server *server,
                               struct server_par *primary, 
                               struct server_par *secondary);
#endif /* USE_BUFFER_HANDLER */

static char        *respbuf = NULL;
static char        *cmdbuf = NULL;
static const char  *tmp_file = 0;
static int          usesyslog = FALSE; /* Use sys log or not */
/* Seconds to waits for server. */
static unsigned int response_timeout = RESPONSE_TIMEOUT;
/* Seconds of idleness before the client logs out, 0 for no timeout. */
static unsigned int idle_timeout = 0;
static char        *buffer = NULL;
static int          remove_files = FALSE;
static int          create_remote_dir = FALSE;

static int          ever_uploaded = FALSE;

#define UPLOAD_DONE "Upload_done" /* This #define:d in buffer_handler.c too. */

#define             NOT_CONNECTED        0
#define             CONNECTING_PRIMARY   1
#define             CONNECTING_SECONDARY 2
#define             CONNECTED_PRIMARY    4
#define             CONNECTED_SECONDARY  8
#define             CONNECTING           16
#define             CONNECTED            32
static int          connect_status = NOT_CONNECTED;

static int qos_ctrl_dsfield = 0;
static int qos_ctrl_priority = 0;
static int qos_data_dsfield = 0;
static int qos_data_priority = 0;

#if PID_DBG
static inline int get_pid(void);
static int my_current_pid;

static inline int
get_pid(void)
{
	return (my_current_pid);
}
#endif
#define             DUMMY_VALUE     -100 
/* The sets for select end*/
#if TIME_STAMP_DBG
#include <sys/time.h>
#include <time.h>

static const char *dbg_time(void); /**/

static const char *
dbg_time(void) /**/
{
	struct timeval cur_time_secs;
	static char    str[50];
	struct tm      tm_time;

	gettimeofday(&cur_time_secs, NULL); /* Get a fresh time. */

	localtime_r(&((time_t)cur_time_secs.tv_sec), &tm_time);

	snprintf(str,
		 sizeof(str),
		 "%02d:%02d:%02d.%03d ",
		 tm_time.tm_hour,
		 tm_time.tm_min,
		 tm_time.tm_sec,
		 (int)cur_time_secs.tv_usec / 1000);
	return(str);
}
#endif

#ifdef USE_FORMAT_NAME
static struct format_info *format_info;

static void
formatname_cleanup(void)
{
	formatname_exit(format_info);
}
#endif /* USE_FORMAT_NAME */

static void
usage(void)
{
	fprintf(stderr, "Usage: sftpclient [options]\n"
		"Options are:\n"
		" -p <host>        put local file to host remote dir [remote file]\n"
		" -g <host>        get from host\n"
		" -i <host>        interactive to/from host [remote dir]\n"
		" -m <host>        put all files in local dir to host remote dir\n"
		" -M <host>        put all files received on stdin to host remote dir\n"
		" -n <port nbr>    specify remote port number, default is 21\n"
		" -s               use passive mode ftp\n"
		" -t <file>        use temporary file\n"
		" -f <buffer_id>   buffer ID to use while requesting files from buffer_handler\n"
		" -c <remote dir>  remote directory to start in\n"
		" -d <remote file> file to get/put\n"
		" -k <local dir>   local directory to start in\n"
		" -l <local file>  file to get/put\n"
		" -q               qos dsfield:user-priority for control flow\n"
		" -Q               qos dsfield:user-priority for data flow\n"
		" -u <username>\n"
		" -w <password>\n"
		" -T <seconds>     seconds to timeout response waits\n"
		" -I <seconds>     if no uploads to server done in last <seconds> log-out\n"
		" -L               log errors to syslog facility instead of stderr\n"
		" -D               remove local file after it has been uploaded\n"
		" -F               create remote directories\n"
		"Backup server options:\n"
		" -B <host>        specify backup server to use if primary fails\n"
		" -N <port nbr>    specify remote port number, default is 21\n"
		" -S               use passive mode ftp\n"
		" -C <remote dir>  remote directory to start in\n"
		" -U <username>\n"
		" -W <password>\n"
		);
	exit(ERR_GENERAL);
}

static inline int
max(int a, int b)
{
	return (a >= b ? a : b);
}

/* Write a buffer to a file descriptor, possibly in small chunks. */
static int
write_to_fd(int fd, const char *buf, int len)
{
	while (len) {
		int w = write(fd, buf, (size_t)len);

		if (w < 0) {
			if (errno != EINTR && errno != EAGAIN) {
				return w;
			}
		} else {
			len -= w;
			buf += w;
		}
	}
	return 0;
}

/* Copy a file between filedescriptors */
static int
rw_fd(int fd1, int fd2)
{
	int r, q = 0;
	int res = 0;

	while (!q && (r = read(fd1, buffer, FTP_BUFFER_SIZE)) > 0) {
		char *p = buffer;

		D2(fprintf(stderr, "*read %d bytes\n", r));
		while (r) {
			int w = write(fd2, p, (size_t)r);

			D2(fprintf(stderr, "written %d bytes\n", w));
			if (w < 0) {
				if (errno != EINTR && errno != EAGAIN) {
					q++;
					res = -1;
					break;
				}
			} else {
				r -= w;
				p += w;
				res += w;
			}
		}
	}

	return res;
}

/* 229 Entering Extended Passive Mode (|||<PORT>|)\r\n
 * Other characters than '|' can be used as delimiter, specified by
 * the first char after the opening '('.  This complicates parsing.
 */
static int
parse_epsv_resp(char *resp, unsigned short *port)
{
	char *p = strrchr(resp, ')');
	char delim;

	/* There must be 11 chars (for a single digit port) or more in
	 * the resp: "229 (ddd1d)"
	 * Finds the last delimiter and changes it to a '\0' to help
	 * reading the port number preceeding it (see the format in
	 * the function header)
	 */
	if (!p || ((p - resp) < 11)) {
		goto format_error;
	}
	delim = *--p;
	*p-- = '\0';
	while ((p > resp) && (*p != delim)) {
		--p;
	}
	if (*p == delim) {
		p++;
		*port = (unsigned short)atoi(p);
		return (NO_ERROR);
	}
format_error:
	write_error("malformatted EPSV response");
	return (ERROR_NEG);
}

/* Send EPRT command to the server. */
static int
send_eprt(int fd, struct sockaddr_storage *addr)
{
	char buf[128];
	char addr_buf[INET6_ADDRSTRLEN];
	char port_buf[NI_MAXSERV];
	int serr;
	int eprt_af;
	int res;

	if ((res = getnameinfo((struct sockaddr *)addr,
			       sizeof(*addr),
			       addr_buf, sizeof(addr_buf),
			       port_buf, sizeof(port_buf),
			       NI_NUMERICHOST)) != 0) {
		write_error("getnameinfo failed: %s", gai_strerror(res));
		return (ERROR_NEG);
	}

	switch (addr->ss_family) {
	case (AF_INET6):
		eprt_af = ARG_AF_INET6;
		break;
	case (AF_INET):
		eprt_af = ARG_AF_INET;
		break;
	default:
		return (ERROR_NEG);
	}
	serr = snprintf(buf, sizeof(buf), "EPRT |%d|%s|%s|\r\n",
			eprt_af, addr_buf, port_buf);
	if (-1 == serr || serr >= (int) sizeof(buf)) {
		return (ERROR_NEG);
	}
	return write_to_fd(fd, buf, (int)strlen(buf));
}

/* 227 Entering Passive Mode (125,75,247,94,230,29) */
static int
parse_port_resp(const char *resp, unsigned short *port)
{
	int p1, p2;
	char *p = strchr(resp, '(');

	D(fprintf(stderr, "parse port from: %s\n", p));
	if (!p) {
		return (ERROR_NEG);
	}
	if (sscanf(p, "(%*d,%*d,%*d,%*d,%d,%d)", &p1, &p2) == 2) {
		*port = (unsigned short)(p1 * 256 + p2);
		return (NO_ERROR);
	} else {
		return (ERROR_NEG);
	}
}

/* Send PORT command to the server. */
static int
send_port(int fd, struct sockaddr_storage *addr)
{
	char buf[128];
	char *str_poi = NULL;
	char addr_str[INET6_ADDRSTRLEN];
	char port_str[NI_MAXSERV];
	u_int16_t port;
	int res;

	if (addr->ss_family != AF_INET) {
		return (ERROR_NEG);
	}

	if ((res = getnameinfo((struct sockaddr *)addr,
			       sizeof(*addr),
			       addr_str, sizeof(addr_str),
			       port_str, sizeof(port_str),
			       NI_NUMERICHOST)) != 0) {
		write_error("getnameinfo failed: %s", gai_strerror(res));
		return (ERROR_NEG);
	}

	/* Reformat the IP address string to the format specified in RFC 959:
	 * PORT h1,h2,h3,h4,p1,p2 - where hi is the high order 8 bits of the
	 * internet host address
	 */
	str_poi = strchr(addr_str, '.');
	while (str_poi != NULL) {
		*str_poi = ',';
		str_poi = strchr(str_poi, '.');
	}
	port = (u_int16_t)atoi(port_str);

	snprintf(buf, sizeof(buf), "PORT %s,%u,%u\r\n", addr_str,
		 (port >> 8) & 0xFF, port & 0xFF);
	return write_to_fd(fd, buf, (int)strlen(buf));
}

/* Setup the data connection for passive mode
 * Returns file descriptor for socket or negative error code, one of
 *   ERROR_NEG
 *   ERROR_LOGGED_OUT (from wait_response())
 */
static int
init_passive_data_conn(struct s_server *server)
{
	int fd;
	int addrlen;
	struct sockaddr_storage addr;
	struct addrinfo hints;
	struct addrinfo *addr_res;
	char port_str[32];
	int  res;

	/* grab a socket for the data connection */
	fd = socket(server->srv_family, SOCK_STREAM, 0);
	if (fd < 0) {
		write_error("socket error: %s", strerror(errno));
		return (ERROR_NEG);
	}

	D(printf("FTP-DATA flow fd=%d qos_dsfield=%d qos_priority=%d\n",
		 fd, qos_data_dsfield, qos_data_priority));
	qos_setup_socket(server->srv_family,
			 fd, qos_data_dsfield, qos_data_priority);

	/* make server enter passive mode */
	if (server->srv_family == AF_INET) {
		write_to_fd(server->ctrlfd, "PASV\r\n", 6);

		/* and catch the 227 Entering Passive Mode (ip,port) reply */
		if ((res = wait_response(server, "227", 3, 1)) < 0) {
			return (res);
		}

		if (parse_port_resp(respbuf, &(server->dataport)) < 0) {
			return (ERROR_NEG);
		}
	} else {
		write_to_fd(server->ctrlfd, "EPSV\r\n", 6);

		/* and catch the 229 Entering Extended Passive Mode reply */
		if ((res = wait_response(server, "229", 3, 1)) < 0) {
			return (res);
		}

		if (parse_epsv_resp(respbuf, &(server->dataport)) < 0) {
			return (ERROR_NEG);
		}
	}

	snprintf(port_str, sizeof(port_str), "%d", server->dataport);
	D(dbg_printf("Connect passive data connection to server (%s, port %s)\n",
		     server->srv_addr, port_str));

	/* Connect to the server using that port and the address used in the
	 * control connection.
	 * Create a socket address structure based on that address and port,
	 * (hints defining address family and socket type)
	 */
	memset(&addr, 0, sizeof(struct sockaddr_storage));
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = server->srv_family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST;

	if (getaddrinfo(server->srv_addr, port_str, &hints, &addr_res) != 0 ||
	    sizeof(addr) < addr_res->ai_addrlen) {
		return (ERROR_NEG);
	}
	/* Save the result in the addr structure */
	memcpy(&addr, addr_res->ai_addr, addr_res->ai_addrlen);
	addrlen = addr_res->ai_addrlen;
	freeaddrinfo(addr_res);

	if (connect(fd, (struct sockaddr *)&addr, addrlen) < 0) {
		write_error("connect error: %s", strerror(errno));
		return (ERROR_NEG);
	}
	return fd;
}

/* setup the data connection for normal mode
 * be careful to start listen before we send the PORT command
 * Returns file descriptor for socket or negative error code, one of
 *   ERROR_NEG
 *   ERROR_LOGGED_OUT (from wait_response())
 */
static int
init_normal_data_conn(struct s_server *server)
{
	int fd;
	int len;
	int sockopt;
	int addrlen;
	int err;
	struct sockaddr_storage addr;
	struct addrinfo hints;
	struct addrinfo *addr_res;
	int res;

	/* allocate a socket, bind and listen to it,
	 * then tell the server where it is */

	fd = socket(server->srv_family, SOCK_STREAM, 0);
	if (fd < 0) {
		write_error("socket error: %s", strerror(errno));
		return (ERROR_NEG);
	}

	D(printf("FTP-DATA flow fd=%d qos_dsfield=%d qos_priority=%d\n",
		 fd, qos_data_dsfield, qos_data_priority));
	qos_setup_socket(server->srv_family, fd, qos_data_dsfield, qos_data_priority);

	/* Set buffer options for the socket
	 * Ignore failure, kernel will pick sizes */
	sockopt = MAX_CMD_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sockopt, sizeof(sockopt));
	sockopt = MAX_RESPONSE_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &sockopt, sizeof(sockopt));

	/* Use the control connection address, but let the system pick a
	 * new port.
	 * Create a socket address structure based on that address and
	 * no port defined (hints defining address family and the socket type)
	 */
	memset(&addr, 0, sizeof(struct sockaddr_storage));
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = server->srv_family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST;
	err = getaddrinfo(server->my_addr, NULL, &hints, &addr_res);
	if (err != 0 || sizeof(addr) < addr_res->ai_addrlen) {
		write_error("getaddrinfo failed: %s", gai_strerror(err));
		return (ERROR_NEG);
	}
	/* Save the result in the addr structure */
	memcpy(&addr, addr_res->ai_addr, addr_res->ai_addrlen);
	addrlen = addr_res->ai_addrlen;
	freeaddrinfo(addr_res);

	/* bind the socket */
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		write_error("bind error: %s", strerror(errno));
		close(fd);
		return (ERROR_NEG);
	}

	if (listen(fd, 1) < 0) {
		write_error("listen error: %s", strerror(errno));
		close(fd);
		return (ERROR_NEG);
	}

	/* Figure out what address we actually got */
	len = sizeof(addr);
	if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0) {
		write_error("getsockname error: %s", strerror(errno));
		close(fd);
		return (ERROR_NEG);
	}

	/* Send that to the server with a PORT message */
	addr.ss_family = server->srv_family;
	if (addr.ss_family == AF_INET) {
		send_port(server->ctrlfd, &addr);
	} else {
		send_eprt(server->ctrlfd, &addr);
	}

	/* Check for the 200 OK response */
	if ((res = wait_response(server, "200", 2, 1)) < 0) {
		close(fd);
		return (res);
	}

	return fd;
}

/* Setup the data connection to the server, either by normal or passive mode.
 * In normal mode, be careful to start listen() before we send the PORT command.
 * Returns file descriptor for socket or negative error code, one of
 *   ERROR_NEG
 *   ERROR_LOGGED_OUT (from wait_response())
 */
static int
init_data_connection(struct s_server *server)
{
	int fd;

	D(dbg_printf("Open data connection - "));

	if (server->passive) {
		/* passive mode */
		D(dbg_printf("passive mode\n"));
		fd = init_passive_data_conn(server);
	} else {
		/* normal mode */
		D(dbg_printf("normal mode\n"));
		fd = init_normal_data_conn(server);
	}
	return fd;
} /* init_data_connection */

/*
 * Try to open a connection on the provided socket and server address.
 * Returns negative value if failed to open connection and zero on success.
 */
static int
open_connection(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
	int tmp;
	int lon;
	struct timeval tv;
	static fd_set writeset;


	/* Set socket as non-blocking */
	tmp = fcntl(sockfd, F_GETFL, NULL);
  	tmp |= O_NONBLOCK;
  	fcntl(sockfd, F_SETFL, tmp);


	/* Connecting */
	tmp = connect(sockfd, serv_addr, addrlen);
	if (tmp == 0) {
		/* Connected successfully. */
		goto success_exit;
	}

	if (tmp == -1) {
		if (errno != EINPROGRESS) {
			/* Unexpected error. */
			write_error("connect error: %s", strerror(errno));
			return -1;
		}
		tv.tv_sec = response_timeout;
		tv.tv_usec = 0;
		for (;;) {		        
			FD_ZERO(&writeset);
			FD_SET(sockfd, &writeset);
			tmp = select(sockfd + 1, NULL, &writeset, NULL, &tv);
			if (tmp == -1 && errno != EINTR) {
				write_error("error in select(): %s", 
						strerror(errno));
				return -1;
			}
			if (tmp == 0) {
				/* Connection attemped time-outed. */
				write_error("could not connect to the host");
				return -1;
			}
			if (tmp > 0) {
				/* Some activity on the socket detected */
				lon = sizeof(tmp);
				if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
					(void*)(&tmp), &lon) < 0) {
					/* Failed to create connection. */
					write_error("error in getsockopt() %s\n", 
							strerror(errno)); 
					return -1;
				}
				if (tmp) {
					write_error("error connecting to host: %s\n", 
						strerror(tmp));
					return -1;
				} 
				/* Connected successfully. */
				goto success_exit;
			}
		}
	}

success_exit:
	/* Set socket as blocking */
	tmp = fcntl(sockfd, F_GETFL, NULL);
	tmp &= (~O_NONBLOCK);
	fcntl(sockfd, F_SETFL, tmp);

	return 0;
}

static int
do_connect_and_login(struct server_par *par, struct s_server *server)
{
	int cmd_len;
	int len;
	int rslv_retry = 3;
	int rslv_ok = 0;
	int sockopt;
	struct addrinfo hints;
	struct addrinfo *ainfo = NULL;
	struct addrinfo *conn_ainfo = NULL;
	struct sockaddr_storage saddr;
	struct sockaddr_storage saddr_res;
	int res;

	TRACED(dbg_printf("Enter "));
	if (par->hostname == NULL || strlen(par->hostname) == 0) {
		goto failed_leave;
	}

	/* If something hangs, make sure to get out */
	alarm(response_timeout);

	/* Resolve hostname first before we make any connections. */
	do {
		int err;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		err = getaddrinfo(par->hostname, par->port, &hints, &ainfo);
		if (err == 0) {
			rslv_ok = 1;
		} else if (err == EAI_AGAIN) {
			--rslv_retry;
			continue;
		} else {
			rslv_retry = 0;
			write_error("getaddrinfo failed: %s",
				    gai_strerror(err));
		}
	} while (rslv_retry && !rslv_ok);

	if (!rslv_ok) {
		write_error("%s: unknown host", par->hostname);
		goto failed_leave;
	}
	server->passive = par->passive;

	/* Traverse the address list until an AF_INET or AF_INET6
	 * one is found and we get a connection to it. */
	conn_ainfo = ainfo;
	while (conn_ainfo) {
		if (conn_ainfo->ai_family != AF_INET
#if IPV6_SUPPORT
		    && conn_ainfo->ai_family != AF_INET6
#endif
		    ) {
			conn_ainfo = conn_ainfo->ai_next;
			continue;
		}

		/* grab a TCP socket for the control connection */
		server->ctrlfd = socket(conn_ainfo->ai_family, SOCK_STREAM, 0);
		if (server->ctrlfd < 0) {
			write_error("socket error: %s", strerror(errno));
			goto failed_leave;
		}

		/* Set buffer options for the socket
		 * Ignore failure, kernel will pick sizes */
		sockopt = MAX_CMD_SIZE;
		setsockopt(server->ctrlfd, SOL_SOCKET, SO_SNDBUF,
			   &sockopt, sizeof(sockopt));
		sockopt = MAX_RESPONSE_SIZE;
		setsockopt(server->ctrlfd, SOL_SOCKET, SO_RCVBUF,
			   &sockopt, sizeof(sockopt));

		D(printf("FTP-CTRL flow fd=%d qos_ctrl_dsfield=%d qos_ctrl_priority=%d\n",
			 server->ctrlfd, qos_ctrl_dsfield, qos_ctrl_priority));
		qos_setup_socket(conn_ainfo->ai_family,
				 server->ctrlfd, qos_ctrl_dsfield, qos_ctrl_priority);

		/* make a buffered version */
		server->ctrlfile = fdopen(server->ctrlfd, "r");

		/* try connecting. */
		memset(&saddr, 0, sizeof(struct sockaddr_storage));
		memcpy(&saddr, conn_ainfo->ai_addr, conn_ainfo->ai_addrlen);
		if (open_connection(server->ctrlfd,
			(const struct sockaddr*)&saddr,
			conn_ainfo->ai_addrlen) == 0) {
			/* got connection */
			break;
		}

		if (!conn_ainfo->ai_next) {
			/* this was the last address to try, bail out */
			goto failed_leave;
		}
		conn_ainfo = conn_ainfo->ai_next;
  		continue;
	}
	if (!conn_ainfo) {
		write_error("%s: unknown host", par->hostname);
		goto failed_leave;
	}

	/* Save the address and family used in the control connection */
	server->srv_family = saddr.ss_family;
	if ((res = getnameinfo((struct sockaddr *)&saddr, sizeof(saddr),
			       server->srv_addr, sizeof(server->srv_addr), NULL, 0,
			       NI_NUMERICHOST)) != 0) {
		write_error("getnameinfo failed: %s", gai_strerror(res));
		goto failed_leave;
	}

	conn_ainfo = NULL;
	freeaddrinfo(ainfo);
	ainfo = NULL;

	len = sizeof(saddr_res);
	if (getsockname(server->ctrlfd,
			(struct sockaddr *)&saddr_res, &len) < 0) {
		write_error("getsockname error: %s", strerror(errno));
		goto failed_close;
	}
	if ((res = getnameinfo((struct sockaddr *)&saddr_res, len,
			       server->my_addr, sizeof(server->my_addr), NULL, 0,
			       NI_NUMERICHOST)) != 0) {
		write_error("getnameinfo failed: %s", gai_strerror(res));
		goto failed_close;
	}

	D(dbg_printf("server: %s, client: %s\n",
		     server->srv_addr, server->my_addr));

	alarm(0);

	/* Read the welcome banner. */
	if (wait_response(server, "220", 2, 1) < 0) {
		goto failed_close;
	}

	D(fprintf(stderr, "sftpclient[%d]: Connected to %s, "
		  "logging in.\n", getpid(), par->hostname));

        /* Give username and check for 331 Password required for <username>. */
	if ((cmd_len = snprintf(cmdbuf, MAX_CMD_SIZE,
				"USER %s\r\n", par->username)) < 0) {
		write_error("Command too long");
		goto failed_close;
	}
	write_to_fd(server->ctrlfd, cmdbuf, cmd_len);
	if (wait_response(server, "331", 2, 1) < 0) {
		goto failed_close;
	}

	/* Give password and check for 230 User xxx logged in. */
	if ((cmd_len = snprintf(cmdbuf, MAX_CMD_SIZE,
				"PASS %s\r\n", par->password)) < 0) {
		write_error("Command too long");
		goto failed_close;
	}
	write_to_fd(server->ctrlfd, cmdbuf, cmd_len);
	if (wait_response(server, "230", 3, 1) < 0) {
		goto failed_close;
	}

	/* We're in! Let's change the directory. */
	if (par->remote_dir && *par->remote_dir) {
		/* If it ends with a '/', remove the slash EXCEPT if the
		 * path is only "/".
		 */
		if ((strlen(par->remote_dir) > 1) &&
		    (par->remote_dir[strlen(par->remote_dir) - 1] == '/')) {
			par->remote_dir[strlen(par->remote_dir) - 1] = 0;
		}

		if (do_cd(server, par->remote_dir) < 0) {
			D(fprintf(stderr, "sftpclient[%d]: Failed to change to "
				  "remote directory \"%s\".\n",
				  getpid(), par->remote_dir));
			goto failed_close;
		}
	}

	/* Set type to binary and check for 200 Type set to I. */
	D(fprintf(stderr, "Setting type I (binary).\n"));
	write_to_fd(server->ctrlfd, "TYPE I\r\n", 8);
	if (wait_response(server, "200", 2, 1) < 0) {
		goto failed_close;
	}

	TRACED(dbg_printf("Leave - success"));
	return (NO_ERROR);

 failed_close:
	close(server->ctrlfd);
 failed_leave:
	if (ainfo != NULL) {
		freeaddrinfo(ainfo);
		ainfo = NULL;
	}
	TRACED(dbg_printf("Leave - failed"));
	return (ERROR_NEG);
}

/* Send a rename command to the server.
 * Returns NO_ERROR if all went well, else
 * ERROR_NEG or ERROR_LOGGED_OUT.
 */
static int
do_rename(struct s_server *server, const char *old_name, const char *new_name)
{
	int len;
	int res;
	char *dir;
	char *slash;

	/* Create destination folder if it not exists */
	dir = strdup(new_name);
	slash = strrchr(dir, '/');

	if (slash != NULL) {
		*slash = '\0';
		if (do_mkpath(server, dir) != 0) {
			write_error("Rename: creating the path %s failed", dir);
			free(dir);
			return (ERROR_NEG);
		}
	}
	free(dir);

	if ((len = snprintf(cmdbuf, MAX_CMD_SIZE, "RNFR %s\r\n", old_name)) < 0) {
		write_error("Command too long");
		return (ERROR_NEG);
	}
	write_to_fd(server->ctrlfd, cmdbuf, len);

	/* Check for the 350 File exists */
	if ((res = wait_response(server, "350", 3, 1)) < 0) {
		if (ERROR_LOGGED_OUT != res) {
			write_error("Rename: %s doesn't exist", old_name);
		}
		return (res);
	}

	if ((len = snprintf(cmdbuf, MAX_CMD_SIZE, "RNTO %s\r\n", new_name)) < 0) {
		write_error("Command too long");
		return (ERROR_NEG);
	}
	write_to_fd(server->ctrlfd, cmdbuf, len);

	/* Check for the 250 Command successful */
	if ((res = wait_response(server, "250", 2, 1)) < 0) {
		if (ERROR_LOGGED_OUT != res) {
			write_error("Rename %s -> %s failed", old_name, new_name);
		}
		return (res);
	}

	return NO_ERROR;
}

/* Put a file to the server, possibly utilizing a temporary file on the server
 * and then a rename.
 * Returns NO_ERROR (0) if all went well, else either ERROR_LOGGED_OUT (-2)
 * or ERROR_NEG (-1).
 */
static int
do_put(struct s_server *server,
       const char      *local_file,
       const char      *remote_file,
       const char      *temp_file,
       struct timeval  *grab_time,
       int              is_retrying)
{
	int                     len;
	int                     sock;
	int                     file;
	struct sockaddr_storage ssaddr;
	int                     addr_size;
	int                     retry;
	int                     try_mkpath = TRUE;
	int                     res;

	TRACED(dbg_printf("Enter %s", __FUNCTION__));
	if (!local_file || *local_file == '\0') {
		return ERROR_NEG;
	}

	if (!remote_file || *remote_file == '\0') {
		char *t;

		if ((t = strrchr(local_file, '/')))
			remote_file = &t[1];
		else
			remote_file = local_file;
	}

#ifdef USE_FORMAT_NAME
	{
		static char formatted_name[256];

		/* If this is a retry we already have a valid name for our
		 * current file in formatted_name[]. Then don't call
		 * formatname() again or else there will be gaps in the file
		 * name sequence.
		 */
		if (!is_retrying) {
			if (grab_time == NULL) {
				/* the grab time is encoded in the filename */
				formatname(formatted_name, 
					   sizeof(formatted_name), 
					   remote_file, 
					   format_info, 
					   local_file);
			} else {
				/* we have an explicit grab time available */
				formatstring(formatted_name, 
					   sizeof(formatted_name), 
					   remote_file, 
					   format_info, 
					   grab_time);
			}
		}

		remote_file = formatted_name;
	}
#else
	(void)is_retrying;  /* To avoid warning for unused parameter. */
#endif

	do {
		retry = FALSE;

		/* Init the data connection. In pasv case, it returns the data connection
		 * already setup to the server. In the normal case, it returns the socket
		 * that will get the accept() later when the server sends out a connection
		 * back to us.
		 */
		/* Check for file first! */
		if ((file = open(local_file, O_RDONLY)) < 0) {
			write_warning("Could not open local file: %s, %s.",
				      local_file, strerror(errno)); /**/
			return ERROR_NEG;
		}

		if ((sock = init_data_connection(server)) < 0) {
			close(file);
			TRACED(dbg_printf("Leave."));
			/* Return error code from init_data_connection(). */
			return (sock);
		}

		alarm(response_timeout);

		/* Send the STOR command with the filename: */

		if ((len = snprintf(cmdbuf, MAX_CMD_SIZE, "STOR %s\r\n",
				    temp_file ? temp_file : remote_file)) < 0) {
			write_error("Command too long");
			TRACED(dbg_printf("Leave."));
			return ERROR_NEG;
		}
		write_to_fd(server->ctrlfd, cmdbuf, len);

		/* Wait for a positive reply (1yz): */
		if ((res = wait_response(server, "150", 1, !(create_remote_dir && try_mkpath))) < 0) {
			if (ERROR_LOGGED_OUT == res) {
				TRACED(dbg_printf("Leave."));
				return (ERROR_LOGGED_OUT);
			} else if (respbuf[0] == '4') {
				/* Transient error, try again. */
				/* 421 No Transfer Timeout is NOT transient and is
				 * handled above.
				 */
				retry = TRUE;
				usleep(100000);
			} else if (create_remote_dir && try_mkpath) {
				/* Remote dir may not exist, try to create it. */
				char *dir = strdup(remote_file);
				char *slash = strrchr(dir, '/');

				if (slash != NULL) {
					*slash = '\0';
					if (do_mkpath(server, dir) == 0) {
						retry = TRUE;
					}
				}
				free(dir);
				/* Only try this once. */
				try_mkpath = FALSE;
			} else {
				D(dbg_printf("Non-transient error: %c%c%c",
					     respbuf[0],
					     respbuf[1],
					     respbuf[2]));
				 /*FIXME Handle error!!*/
			}

		} else {
			ever_uploaded = TRUE;
		}

		if (retry) {
			close(sock);
			close(file);
		}

	} while (retry);

	/* Now its time to accept that connection back from the server (if not pasv). */

	alarm(response_timeout);

	if (!server->passive) {
		int s;

		addr_size = sizeof(ssaddr);
		s = accept(sock, (struct sockaddr *)&ssaddr, &addr_size);

		if (s < 0) {
			write_error("accept error: %s", strerror(errno));
			close(sock);
			TRACED(dbg_printf("Leave."));
			return ERROR_NEG;
		}
		close(sock);
		sock = s;
	}

	rw_fd(file, sock);

	alarm(0);

	close(file);
	close(sock);

	/* wait for the 226 Transfer complete. */

	if ((res = wait_response(server, "226", 2, 1)) < 0) {
		TRACED(dbg_printf("Leave."));
		return (res);
	}

	if (remove_files) {
		unlink(local_file);
	}

	if (temp_file && ((res = do_rename(server, temp_file, remote_file)) < 0)) {
		TRACED(dbg_printf("Leave."));
		return (res);
	}

#ifdef USE_FORMAT_NAME
	formatname_nextstring(format_info);
#endif

	TRACED(dbg_printf("Leave. OK"));
	return NO_ERROR;
} /* do_put() */

static void
do_quit(struct s_server *server)
{
	write_to_fd(server->ctrlfd, "QUIT\r\n", 6);
	/* Lets hope the socket lingers. otherwise who cares... */
	close(server->ctrlfd);
}

/* Checks for the special 421 response code. This is the code for
 * 'No Transfer Timeout' or 'Service Not Available' or similar,
 * all of which means that the server has forcibly logged us out.
 * Returns TRUE if buffer 'rbuf' starts with '421'.
 */
static int
was_logged_out(const char *rbuf)
{
#define MSG_421 "421"

	if (!strncmp(rbuf, MSG_421, strlen(MSG_421))) {
		D(dbg_printf("421 message."));
		return (TRUE);
	} else {
		return (FALSE);
	}
}

#if defined(MPUT_ENABLED) && MPUT_ENABLED
static int
do_mput(int fd, const char **files, const char *temp_name)
{
	int res = 0;

	for (; *files; files++) {
		if (do_put(fd, *files, 0, tmp_file, NOT_RETRY) < 0) {
			res = -1;
			break;
		}
	}

	return res;
}
#endif

/* change local directory */

static int
do_lcd(char *local_dir)
{
	char *p = local_dir;

	while (isgraph(*p)) {
		p++;
	}
	*p = '\0';

	if (*local_dir && chdir(local_dir) < 0) {
		return -1;
	} else if (chdir(getenv("HOME")) < 0) {
		return -2;
	}
	return 0;
}

/* change remote directory */

static int
do_cd(struct s_server *server, char *path)
{
	char *p = path;
	int path_len = 0;
	int len;

	while (isgraph(*p)) {
		p++;
		path_len++;
	}
	*p = '\0';

	if ((len = snprintf(cmdbuf, MAX_CMD_SIZE, "CWD %s\r\n", path)) < 0) {
		write_error("Command too long");
		return -1;
	}
	write_to_fd(server->ctrlfd, cmdbuf, len);

	if (wait_response(server, "250", 2, 1) < 0) {
		return -1;
	}

	return 0;
}

/*****************************************************************************
*# FUNCTION NAME: do_mkpath
*# PARAMETERS   : struct s_server *server
*#                const char *path - String holding path to directory. A path
*#                                   is a sequence of directory names separated
*#                                   by '/'-characters. The path may start
*#                                   with any number of leading
*#                                   '/'-characters. Multiple leading '/' are
*#                                   ignored and replaced with a single '/'.
*# RETURNS      : Error code.
*# DESCRIPTION  : Recursive function to make remote directory.
*#                See RFC 959 "FILE TRANSFER PROTOCOL (FTP)",
*#                            "APPENDIX II - DIRECTORY COMMANDS"
*#***************************************************************************/
static int
do_mkpath(struct s_server *server, const char *path)
{
	int         retval = NO_ERROR;
	const char *slash;
	const char *path_start = path;

	slash = strrchr(path, '/'); /* Find last slash in string. */

	/* Multiple leading '/' are ignored and replaced with a single '/'. */
	while (('/' == path_start[0]) && ('/' == path_start[1])) {
		path_start++;
	}

	if (slash != NULL) {
		/* Cut off last part of path. But don't edit string (it's
		 * const) - make a copy:
		 */
		char *tmp = strndup(path_start, (size_t)(slash - path_start));

		/* Don't make the recursive call if the path is "". */
		if (strlen(tmp) > 0) {
			retval = do_mkpath(server, tmp); /* Recursive call. */
		}
		free(tmp);
	}

	if (retval == 0) {
		int len;

		if ((len = snprintf(cmdbuf, MAX_CMD_SIZE, "MKD %s\r\n", path_start)) < 0) {
			write_error("Command too long");
			return (ERROR_NEG);
		}
		write_to_fd(server->ctrlfd, cmdbuf, len);

		/* Check for OK or "directory already exist" or 550 (since some
		   servers erroneously return 550 when directory exists). */
		/* Expect 257 Informative response "PATHNAME created": */
		if (wait_response(server, "257", 2, 1) < 0 &&
		    /* Ignore bogus 550 error: */
		    strncmp(respbuf, "550", 3) != 0 && /* BUG: Any real error 550 is ignored too.*/
		    /* Ignore 521 error "Directory exists": */
		    strncmp(respbuf, "521", 3) != 0) {
			/* Any other error means failure for real: */
			syslog(LOG_INFO, "MKD failed! - %s", respbuf);
			return (ERROR_NEG);
		}
	}
	return (retval);
}

/* Parse command line for commands. Only used in interactive mode.*/
static int
parse_command_line(struct s_server *server, char *command_line)
{
	char cmd[16];
	short len;

	if (*command_line == '\0') {
		return 0;
	}

	while (*command_line != '\0' && isspace(*command_line)) {
		command_line++;
	}

	if (*command_line == '\0') {
		return 0;
	}

	len = 0;
	while (isgraph(*command_line) && len < 15) {
		cmd[len++] = tolower(*command_line);
		command_line++;
	}

	if (len == 0) {
		return 0;
	}

	cmd[len] = '\0';

	while (*command_line != '\0' && isspace(*command_line)) {
		command_line++;
	}

	switch (cmd[0]) {
	case 'b':
		switch (cmd[1]) {
		case 'y':
			if (len == 2 || (cmd[2] == 'e' && len == 3)) {
				return 1;
			}
			break;
		default:
			break;
		}
		break;
	case 'c':
		switch (cmd[1]) {
		case 'd':
			if (len == 2) {
				return do_cd(server, command_line);
			}
		default:
			break;
		}
		break;
	case 'g':
		if (!strncmp("get", cmd, (size_t)len)) {
		}
		break;
	case 'l':
		if (!strncmp("lcd", cmd, (size_t)len)) {
			do_lcd(command_line);
		}
		break;
	case 'm':
		if (!strncmp("mkdir", cmd, (size_t)len)) {
			return do_mkpath(server, command_line);
		}
#if defined(MPUT_ENABLED) && MPUT_ENABLED
		if (!strncmp("mput", cmd, len)) {
			/*return do_mput(fd, files, temp_name);*/
		}
#endif
		break;
	case 'p':
		if (!strncmp("put", cmd, (size_t)len)) {
			char *local_file = command_line;
			char *remote_file = 0;

			if (*command_line == '\0') {
				break;
			}
			while (isgraph(*command_line)) {
				command_line++;
			}
			*command_line++ = '\0';
			while (*command_line != '\0' &&
			       isspace(*command_line)) {
				command_line++;
			}
			if (isgraph(*command_line)) {
				remote_file = command_line;
				while (isgraph(*command_line)) {
					command_line++;
				}
				*command_line++ = '\0';
			}
			return do_put(server, 
				      local_file, 
				      remote_file, 
				      tmp_file, 
				      NULL, 
				      NOT_RETRY);
		}
		break;
	case 'q':
		if (!strncmp("quit", cmd, (size_t)len)) {
			return 1;
		}
		break;
	case 'r':
		if (!strncmp("rename", cmd, (size_t)len)) {
			char *old_name;
			char *new_name;
			char *p = command_line;

			while (isgraph(*p)) {
				p++;
			}
			*p++ = '\0';
			old_name = command_line;
			while (isspace(*p)) {
				p++;
			}
			new_name = p;
			while (isgraph(*p)) {
				p++;
			}
			*p = '\0';
			return do_rename(server, old_name, new_name);
		}
		break;
	default:
		break;
	}

	return 0;
} /* parse_command_line() */

/* parse command lines from stdin and do stuff */
static int
interactive_mode(struct s_server *server)
{
	static char command_line[512];
	int quit;

	do {
		if (!fgets(command_line, sizeof(command_line) - 1, stdin)) {
			break;
		}
		if ((quit = parse_command_line(server, command_line)) == 0) {
			write(fileno(stdout), "0\n", strlen("0\n"));
		} else {
			write(fileno(stdout), "1\n", strlen("1\n"));
		}
	} while (!quit);

	return 0;
}

/* Read FTP reply to one command from stream.
 * The reply is written to respbuf buffert.
 */
static int
read_response(FILE *stream)
{
	int len;
	char line[MAX_RESPONSE_SIZE];
        char last_line_head[5];

	/* Reset the response buffer. */
	respbuf[0] = 0;

	/*
	* Read the first line of the reply.
	*/
	if (fgets(line, MAX_RESPONSE_SIZE, stream) == NULL) {
		write_warning("Error reading reply from server.");
		return ERROR_NEG;
	}
	len = strlen(line);
	if (line[len - 2] == '\r') {
		len--;
	}
	strncat(respbuf, line, MAX_RESPONSE_SIZE - strlen(respbuf));

	/*
	* Figure out how the last line of the reply should begin.
	*/
	memcpy(last_line_head, line, 3);
	last_line_head[3] = ' ';
	last_line_head[4] = 0;

	/*
	* Read reply until the last line is found.
	*/
	while (strncmp(last_line_head, line, 4) != 0) {
		if (fgets(line, MAX_RESPONSE_SIZE, stream) == NULL) {
			write_warning("Error reading reply from server.");
			return ERROR_NEG;
		}

		len = strlen(line);
		if (line[len - 2] == '\r') {
			len--;
		}
		/* If the whole reply is longer then MAX_RESPONSE_SIZE,
                 * continue reading but discard the part that don't fit
                 */
		if ((strlen(respbuf) + len) < MAX_RESPONSE_SIZE) {
			strncat(respbuf, line, MAX_RESPONSE_SIZE - strlen(respbuf));
		}

	}

        /* Remove last \n in the respbuf. */
	respbuf[strlen(respbuf) - 1] = 0;

	return NO_ERROR;
}

/* Waits for a response from the server. If no response is
 * recivied within a timeout, the alrm signal is generated.
 * If the response is the one waited for, return NO_ERROR.
 * If it is a code 421 response return ERROR_LOGGED_OUT.
 * If it is some other transient or permanent negative reply, return ERROR_NEG.
 * If the connection breaks, return ERROR_NEG.
 */
static int
wait_response(struct s_server *server, const char *wanted, int wantlen, int log_error)
{
	int            ret_val = NO_ERROR; /* Optimistic default. */

	TRACED(dbg_printf("Enter %s ", __FUNCTION__));
	/* Make sure we dont get stuck here. */
	alarm(response_timeout);
	ret_val = read_response(server->ctrlfile);
	alarm(0);
	
	if (ret_val == NO_ERROR) {
		if (!strncmp(respbuf, wanted, (size_t)wantlen)) {
			ret_val = NO_ERROR;
		} else if (respbuf[0] == '4' ||
			   respbuf[0] == '5') {
			/* Negative reply */
			if (was_logged_out(respbuf)) {
				ret_val = ERROR_LOGGED_OUT;
			} else {
				if (log_error) {
					write_warning(respbuf);
				}
				ret_val = ERROR_NEG;
			}
		}
        }

	if (NO_ERROR == ret_val) {
		TRACED(dbg_printf("Leave - success"));
	} else {
		TRACED(dbg_printf("Leave - failure; returns %d", ret_val));
	}

	return (ret_val);
} /* wait_response() */

static void
sigalrm_handler(int signum)
{
	(void)signum;

	TRACED(dbg_printf("SIGALRM"));

	response_timeout_action();
}

static void
sigterm_handler(int signum)
{
	(void)signum;

	cleanup();

	TRACED(dbg_printf("SIGTERM Exit 0"));
	exit(0);
}

static void
sigpipe_handler(int signum)
{
	(void)signum;

	cleanup();
	TRACED(dbg_printf("sftpclient got SIGPIPE"));
	err_exit();
}

static void
response_timeout_action(void)
{
	if (CONNECTING != connect_status) {
		cleanup();
		write_warning("Timeout waiting for response from server.");
		err_exit();
	}
}

static void
cleanup(void)
{
	fclose(stdin);
}

static void
err_exit(void)
{
	if (ever_uploaded) {
		TRACED(dbg_printf("Exit ERR_GENERAL (%d)", ERR_GENERAL));
		exit(ERR_GENERAL);
	} else {
		TRACED(dbg_printf("Exit ERR_INIT_CONNECT (%d)", ERR_INIT_CONNECT));
		exit(ERR_INIT_CONNECT);
	}
}

/* Connects to remote FTP server. If primary not available
 * tries with secondary server.
 * Returns TRUE if connected.
 */
static int
connect_procedure(struct server_par *primary_ptr,
		  struct server_par *secondary_ptr,
		  struct s_server   *server_ptr)
{
	TRACED(dbg_printf("Enter %s", __FUNCTION__));
	/* First try primary, then secondary. */
	connect_status = CONNECTING;
	if (do_connect_and_login(primary_ptr, server_ptr) < 0 &&
	    do_connect_and_login(secondary_ptr, server_ptr) < 0) {
		TRACED(dbg_printf("Leave."));
		return (ERROR_NEG);
	} else {
		connect_status = CONNECTED;
		TRACED(dbg_printf("Leave."));
		return (NO_ERROR);
	}
	TRACED(dbg_printf("Leave."));
}

static void
qos_setup_socket(int af, int sock, int dsfield, int prio)
{
	int tos;

	/*
	 * ds-field is 6bits
	 */
	dsfield &= 0x3f;
	tos = dsfield << 2;

	switch (af) {
	case AF_INET:
		setsockopt(sock, SOL_IP, IP_TOS, &tos, sizeof(tos));
		break;
	case AF_INET6:
#ifdef IPV6_TCLASS
		setsockopt(sock, SOL_IPV6, IPV6_TCLASS, &tos, sizeof(tos));
#endif
		break;
	default:
		write_error("%s: unsupported AF", __func__);
		break;
	}
	setsockopt(sock, SOL_SOCKET, SO_PRIORITY, &prio, sizeof(prio));
}


#if USE_BUFFER_HANDLER
static int
mput_buffer_handler(char* buffer_id, char *remote_file, struct s_server *server,
                    struct server_par *primary, struct server_par *secondary)
{
        /* TODO implement the idle_timeout feature */
	if (bhand_init(buffer_id) != NO_ERR) {
		write_error("error connecting to buffer_handler");
		return FALSE;
	}

	int  res;
	char local_file_name[FILE_NM_SIZ];
	int  reconnect_retry = FALSE;
	int len;
        int ret_status = TRUE;
	struct timeval grab_time;

	while (TRUE) {
		/* Get file name to upload. */
                res = bhand_get_filename(local_file_name, FILE_NM_SIZ, &grab_time);
		if (res == COM_ERR) {
			write_error("error communicating with buffer_handler");
			return FALSE;
		}
		if (res == NO_FILES) {
			/* done uploading */
			break;                
		}
                /* TODO why do we need len !?? */
		len = strnlen(local_file_name, FILE_NM_SIZ);

		if (connect_status == NOT_CONNECTED) {
			if (connect_procedure(primary, secondary, server) < 0) {
				D(dbg_printf("Reconnect failed."));
				ret_status = FALSE;
				goto finished;
			}
		}

		/* Try uploading file until we give up. */
		do {
			res = do_put(server,
				     local_file_name,
				     remote_file,
				     tmp_file,
				     &grab_time,
				     reconnect_retry);
			if (ERROR_LOGGED_OUT == res) {
				/* Logged out.
				 * Reconnect and retry. */
				if (connect_procedure(primary, secondary, server) < 0) {
					D(dbg_printf("Reconnect failed."));
					ret_status = FALSE;
					goto finished;

				} else {
					D(dbg_printf("OK reconnect."));
					reconnect_retry = TRUE;
					/* One more round for put retry. */
				}
			} else if (NO_ERROR == res) {
				reconnect_retry = FALSE; /* OK put. No retry. */
			} else {
				D(dbg_printf("Other error from put: %d", res));
				ret_status = FALSE;
				goto finished;
			}
		} while (reconnect_retry); /* Retry if logged out. */

		/* notify buffer_handler that the file was uploaded */
	        bhand_file_uploaded();

	}


finished:
	
	bhand_cleanup();
	return ret_status;
}
#endif /* USE_BUFFER_HANDLER */

int
main(int argc, char **argv)
{
	struct s_server   server;
	int               mode = -1;
	char             *local_dir = 0;
	char             *local = 0;
	char             *remote_file = 0;
	char             *buffer_id = NULL;
	struct server_par primary = { NULL, "anonymous", "ftpuser@", NULL, "21", FALSE };
	struct server_par secondary = { NULL, "anonymous", "ftpuser@", NULL, "21", FALSE };

	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigterm_handler);
	signal(SIGPIPE, sigpipe_handler);
	/* setup the SIGALRM handler which will kill us if we hang on something */
	signal(SIGALRM, sigalrm_handler);

#if PID_DBG
	my_current_pid = getpid();
#endif

#if REOPEN_STDERR
	if (freopen("/dev/console", "a", stderr) == NULL) {
		write_error("Error freopen stderr to /dev/console. %m");
	}
#endif

	if (argc < 3) {
		usage();
	}

	/* decode command-line options: */
	while (1) {
		int c = getopt(argc, argv, "p:g:i:m:M:n:t:f:c:d:k:l:u:w:T:I:sLDFB:N:SC:U:W:q:Q:");

		if (c == -1)
			break;

		switch (c) {
		case 'p':
			mode = FTP_PUT;
			primary.hostname = optarg;
			break;
		case 'g':
			mode = FTP_GET;
			primary.hostname = optarg;
			break;
		case 'i':
			mode = FTP_INTERACTIVE;
			primary.hostname = optarg;
			break;
		case 'm':
			mode = FTP_MPUT;
			primary.hostname = optarg;
			break;
		case 'M':
			mode = FTP_MPUT_BHAND;
			primary.hostname = optarg;
			break;
		case 's':
			primary.passive = TRUE;
			break;
		case 'k':
			local_dir = optarg;
			break;
		case 'l':
			local = optarg;
			break;
		case 'q':
			{
				char *endptr;

				qos_ctrl_dsfield = strtoul(optarg, &endptr, 0);
				if (endptr != optarg && endptr[0] != '\0') {
					qos_ctrl_priority = strtoul(endptr + 1, NULL, 0);
				}
			}
			break;
		case 'Q':
			{
				char *endptr;

				qos_data_dsfield = strtoul(optarg, &endptr, 0);
				if (endptr != optarg && endptr[0] != '\0') {
					qos_data_priority = strtoul(endptr + 1, NULL, 0);
				}
			}
			break;
		case 'c':
			primary.remote_dir = optarg;
			break;
		case 'd':
			remote_file = optarg;
			break;
		case 't':
			tmp_file = optarg;
			break;
		case 'f':
			buffer_id = optarg;
			break;
		case 'u':
			primary.username = optarg;
			break;
		case 'w':
			primary.password = optarg;
			break;
		case 'T':
			response_timeout = (unsigned int)atoi(optarg);
			break;
		case 'I':
			idle_timeout = (unsigned int)atoi(optarg);
			break;
		case 'L':
			usesyslog = TRUE;
			break;
		case 'n':
			primary.port = optarg;
			break;
		case 'D':
			remove_files = TRUE;
			break;
		case 'F':
			create_remote_dir = TRUE;
			break;
		case 'B':
			secondary.hostname = optarg;
			break;
		case 'N':
			secondary.port = optarg;
			break;
		case 'S':
			secondary.passive = TRUE;
			break;
		case 'C':
			secondary.remote_dir = optarg;
			break;
		case 'U':
			secondary.username = optarg;
			break;
		case 'W':
			secondary.password = optarg;
			break;
		default:
			usage();
			break;
		}
	}

	if (optind != argc || mode < 0)
		usage();

	if (mode == FTP_GET) {
		write_error("get is not implemented.");
		exit(ERR_GENERAL);
	}

	/* Open syslog facility */

	if (usesyslog)
		openlog(argv[0], LOG_PID, LOG_USER);

	TRACED(dbg_printf("Enter %s", __FUNCTION__));

	if ((buffer = malloc(FTP_BUFFER_SIZE)) == NULL ||
	    (respbuf = malloc(MAX_RESPONSE_SIZE)) == NULL ||
	    (cmdbuf = malloc(MAX_CMD_SIZE)) == NULL) {
		write_error("Out of memory.");
		goto errout;
	}

	buffer_handler_help_init(usesyslog);

	/* Check, if there is a local file that it can be read at all. */
	if (local_dir && chdir(local_dir) < 0) {
		write_error("Can't cd to local dir: %s", strerror(errno));
		goto errout;
	}

	/* Check, if there is a local directory that it can be read at all. */
	if (local && access(local, R_OK) < 0) {
		write_error("Can't read local file: %s", strerror(errno));
		goto errout;
	}


#ifdef USE_FORMAT_NAME
	format_info = formatname_init();
	atexit(formatname_cleanup);
#endif
	if (connect_procedure(&primary, &secondary, &server) < 0) {
		goto errout;
	}

	/* put/get file */

	switch (mode) {
	case FTP_PUT:
		/* Check that there is a local file. */
		if (!local) {
			write_error("No local file.");
			goto errout;
		}
			
		/* If no remote file was given, use the local filename. */

		if (!remote_file) {
			remote_file = strrchr(local, '/');
			if (remote_file && remote_file[1] != '\0') {
				remote_file++;
			} else {
				remote_file = local;
			}
		}
		if (do_put(&server, 
			   local, 
			   remote_file, 
			   tmp_file,
			   NULL,
			   NOT_RETRY) < 0) {
			goto errout;
		}
		break;
	case FTP_MPUT:
	{
		struct dirent *dirp;
		DIR *dp;

		/* Check that there is a local directory. */
		if (!local_dir) {
			write_error("No local directory.");
			goto errout;
		}

		if (!(dp = opendir(local_dir))) {
			goto errout;
		}
		while ((dirp = readdir(dp))) {
			if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
				do_put(&server, 
				       dirp->d_name, 
				       remote_file, 
				       tmp_file,
 				       NULL,
				       NOT_RETRY);
			}
		}
		if (closedir(dp) < 0) {
			goto errout;
		}
		break;
	}
#ifdef USE_BUFFER_HANDLER
	case FTP_MPUT_BHAND:
		if (!mput_buffer_handler(buffer_id, remote_file, &server, &primary, &secondary)) {
			goto errout;
		}
		break;
#endif /* USE_BUFFER_HANDLER */
	case FTP_INTERACTIVE:
		D(fprintf(stderr, "sftpclient[%d]: Starting interactive "
			  "ftp mode.\n", getpid()));
		write(fileno(stdout), "OK\n", strlen("OK\n"));
		interactive_mode(&server);
		break;
	case FTP_GET:
		break;
	default:
		break;
	} /* End switch (mode) */

	/* quit and exit */
	do_quit(&server);

	TRACED(dbg_printf("Exit 0"));
	exit(0);

 errout:
	if (mode == FTP_INTERACTIVE) {
		write(fileno(stdout), "ERROR\n", strlen("ERROR\n"));
	}
	err_exit();

	return (0); /* Formality. Exit already done, so unreachable. */
} /* main() */
