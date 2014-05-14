/*
	CGI Library - CGI routines
	$Id: cgl.c,v 1.17 2001/02/22 18:15:33 harding Exp $
*/


/*
 * Copyright (c) 1998-2000 Carson S.K. Harding
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its 
 *    contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "cgl.h"


cglEnv	*cgl_Env;	/* for data from server-set environment     */
int	cglerrno;	/* internal library error code              */
int	cglsyserr;	/* save system errno                        */
char	*cglerrlist[];	/* list of error descriptions for cglerrno  */
cgllist	*cgl_Formdata;	/* data read from form (POST or GET)        */
cgllist	*cgl_Cookies;	/* cookies garnered                         */

int  cgl_parsecgibuf(cgllist *cdata, char *query);
int  cgl_parsecookiebuf(cgllist *cdata, char *cookies);


/*
 * Default controls. But make any adjustments before calling cgl_init() 
 * or cgl_initformdata().
 */
int	cgl_def_hcontrol = CGL_HASH_AUTO;	/* hash sizing behaviour */
int	cgl_def_mincount = CGL_MINTOHASH;	/* start hashing */
int	cgl_def_maxcount = CGL_MAXCOUNT;	/* max name/value pairs */
int	cgl_def_maxdata  = CGL_MAXDATA;	/* how much we're willing to read */


/*
 * internal functions and defines
 */
static	void cgl_seterrno(int n);

static	int  cgl_hashsize(cgllist *cdata);
static	int  cgl_hashnode(cgllist *cdata, cglnode *node);
static	int  cgl_linknode(cgllist *cdata, cglnode *node, int where);
static	void cgl_freehash(cgllist *cdata);
static	int  cgl_createhash(cgllist *cdata, int hashsz);
static	int  cgl_nexthashsz(cgllist *cdata);
static	unsigned int cgl_hashpjw(const char *datum, const int hashsz);

#define NULLSTR	"(null)" 	/* what we print when a var is NULL */

/* 
 * buffers used for form and cookie data: don't mess with them! 
 * the default form reading functions stuff the form data string 
 * into them, and the parsing functions chop it up _in situ_. 
 * The default (cgl_Formdata) name/value hashed list ends up setting
 * pointers into cgl_Buf, and the cookie list (cgl_Cookies) sets
 * pointers into cgl_Cookiebuf. No mallocs, minimum memory; but
 * increased chances for confusion....
 */
static char *cgl_Buf;
static char *cgl_Cookiebuf;

/*
 * error numbers and messages
 */
char *cglerrlist[] = {
#define CGL_NOERR	0
	"No error",
#define CGL_SYSERR	1
	"System error",
#define CGL_INITTWICE	2
	"Attempt to initialize twice",
#define CGL_NOMETHOD	3
	"No method specified",
#define CGL_BADMETHOD 	4
	"Bad method specified",
#define CGL_BADCONTLEN	5
	"Bad content-length",
#define CGL_SHORTREAD	6
	"Read error on post data",
#define CGL_NOQUERY	7
	"No query string",
#define CGL_BADHASHCTL	8
	"Bad hash control value",
#define CGL_BADLISTDATA	9
	"Bad list data/structure",
#define CGL_BADINSERT	10
	"Invalid insertion directive",
#define CGL_NOINIT	11
	"Data not initialized",
#define CGL_UNKNOWNTYPE 12
	"Unknown content-type encoding",
#define CGL_TOOMANY	13
	"Maximum data count exceeded",
#define CGL_NULLARG	14
	"Null value passed as argument",
#define CGL_BIGDATA	15
	"Form data byte limit exceeded"
};
#define CGL_MAXERRNO 	15


/* ------------------------------------------------------------------------ *
 * ENVIRONMENT AND FORM DATA
 * ------------------------------------------------------------------------ */

/*
 * set everything up. call cgl_initenv() and cgl_initformdata().
 */
int 
cgl_init(void)
{
	if (cgl_initenv() == -1)
		return -1;

	if (cgl_initformdata() == -1)
		return -1;

	if (cgl_initcookies() == -1)
		return -1;

	return 0;
}

/*
 * Allocate and initialize the global environment variable
 * structure cgl_Env.
 */
int
cgl_initenv(void)
{
	cgl_Env = malloc(sizeof(cglEnv));
	if (!cgl_Env) {
		cgl_seterrno(CGL_SYSERR);
		return -1;
	}

	cgl_Env->auth_type         = cgl_getenv("AUTH_TYPE");
	cgl_Env->content_length    = cgl_getenv("CONTENT_LENGTH");
	cgl_Env->content_type      = cgl_getenv("CONTENT_TYPE");
	cgl_Env->document_root     = cgl_getenv("DOCUMENT_ROOT");
	cgl_Env->gateway_interface = cgl_getenv("GATEWAY_INTERFACE");
	cgl_Env->http_accept       = cgl_getenv("HTTP_ACCEPT");
	cgl_Env->http_cookie       = cgl_getenv("HTTP_COOKIE");
	cgl_Env->http_pragma       = cgl_getenv("HTTP_PRAGMA");
	cgl_Env->http_user_agent   = cgl_getenv("HTTP_USER_AGENT");
	cgl_Env->path_info         = cgl_getenv("PATH_INFO");
	cgl_Env->path_translated   = cgl_getenv("PATH_TRANSLATED");
	cgl_Env->query_string      = cgl_getenv("QUERY_STRING");
	cgl_Env->remote_addr       = cgl_getenv("REMOTE_ADDR");
	cgl_Env->remote_host       = cgl_getenv("REMOTE_HOST");
	cgl_Env->remote_ident      = cgl_getenv("REMOTE_IDENT");
	cgl_Env->remote_port       = cgl_getenv("REMOTE_PORT");
	cgl_Env->remote_user       = cgl_getenv("REMOTE_USER");
	cgl_Env->request_method    = cgl_getenv("REQUEST_METHOD");
	cgl_Env->request_uri       = cgl_getenv("REQUEST_URI");
	cgl_Env->script_filename   = cgl_getenv("SCRIPT_FILENAME");
	cgl_Env->script_name       = cgl_getenv("SCRIPT_NAME");
	cgl_Env->server_admin      = cgl_getenv("SERVER_ADMIN");
	cgl_Env->server_name       = cgl_getenv("SERVER_NAME");
	cgl_Env->server_port       = cgl_getenv("SERVER_PORT");
	cgl_Env->server_protocol   = cgl_getenv("SERVER_PROTOCOL");
	cgl_Env->server_software   = cgl_getenv("SERVER_SOFTWARE");

	return 0;
}

/*
 * Slurp form data into cgl_Formdata
 */
int
cgl_initformdata(void)
{
	unsigned int len;

	len = 0;


	/* have we already initialized the form data? */
	if (!cgl_Formdata) {
		cgl_Formdata = cgl_newlist();
		if (!cgl_Formdata)
			return -1;
	} else {
		/* initformdata has already been called */
		cgl_seterrno(CGL_INITTWICE);
		return -1;
	}


	/* is data from a GET or a POST? */
	if (cgl_Env->request_method) {

		if (strcmp(cgl_Env->request_method, "GET") == 0) {
			cgl_Formdata->source = CGL_METHOD_GET;
		} else if (strcmp(cgl_Env->request_method, "POST") == 0) {
			cgl_Formdata->source = CGL_METHOD_POST;
		} else {
			cgl_seterrno(CGL_BADMETHOD);
			return -1;
		}

	} else {

		cgl_seterrno(CGL_NOMETHOD);
		return -1;

	}

	if (cgl_Formdata->source == CGL_METHOD_POST) {

		/* get content type */
		if (strcmp(cgl_Env->content_type, 
		    "application/x-www-form-urlencoded") != 0) {
			cgl_seterrno(CGL_UNKNOWNTYPE);
			return -1;
		}

		/* get content length */
		if (cgl_Env->content_length != NULL) {
			len = atoi(cgl_Env->content_length);
		}
	
		if (len <= 0) {
			cgl_seterrno(CGL_BADCONTLEN);
			return -1;
		}

		if (len > cgl_def_maxdata) {
			cgl_seterrno(CGL_BIGDATA);
			return -1;
		}

		/* allocate the global buffer */
		cgl_Buf = malloc((size_t)len+1);
		if (!cgl_Buf) {
			cgl_seterrno(CGL_SYSERR);
			return -1;
		}

		/* slurp */
		if (fread(cgl_Buf, 1, len, stdin) != len) {
			cgl_seterrno(CGL_SHORTREAD);
			return -1;
		}

		cgl_Buf[len] = '\0';

		if (cgl_parsecgibuf(cgl_Formdata, cgl_Buf) == -1)
			return -1;

		/* XXX clean up */

	} else {

		/* GET query: parse query string */
		if (cgl_Env->query_string) {

			cgl_Buf = cgl_strdup(cgl_Env->query_string);
			if (!cgl_Buf) {
				cgl_seterrno(CGL_SYSERR);
				return -1;
			}

			if (cgl_parsecgibuf(cgl_Formdata, cgl_Buf) == -1)
				return -1;

			/* XXX clean up */

		} /* else {
			cgl_seterrno(CGL_NOQUERY);
			return -1;
		} */
	}

	return 0;
}

int
cgl_initcookies(void)
{

	if (!cgl_Env) {
		cgl_seterrno(CGL_NOINIT);
		return -1;
	}

	if (strlen(cgl_Env->http_cookie) != 0) {

		if (cgl_Cookiebuf)
			free(cgl_Cookiebuf);

		if (cgl_Cookies)
			cgl_freedata(cgl_Cookies);

		cgl_Cookies = cgl_newlist();
			
		cgl_Cookiebuf = cgl_strdup(cgl_Env->http_cookie);
		if (!cgl_Cookiebuf) {
			cgl_seterrno(CGL_SYSERR);
			return -1;
		}

		if (cgl_parsecookiebuf(cgl_Cookies, cgl_Cookiebuf) == -1)
			return -1;

	}

	return 0;
}

/*
 * free all data set up by the cgl_init*() functions.
 */
void
cgl_freeall(void)
{
	cgl_freeenv();
	cgl_freeformdata();
	cgl_freecookies();

	return;
}

/*
 * free cgl_Env and set to NULL
 */
void
cgl_freeenv(void)
{
	if (cgl_Env) {
		free(cgl_Env);
		cgl_Env = NULL;
	}

	return;
}

/*
 * free all form related data. We know all name/value strings
 * are in cgl_Buf, so when we free that we take care of them all.
 */
void
cgl_freeformdata(void)
{

	cgl_freedata(cgl_Formdata);

	if (cgl_Buf)
		free(cgl_Buf);

	cgl_Formdata = NULL;
	cgl_Buf = NULL;

	return;
}

/*
 * free all cookie related data. We know all name/value strings
 * are in cgl_Cookiebuf, so when we free that we take care of 
 * them all.
 */
void
cgl_freecookies(void)
{
	cgl_freedata(cgl_Cookies);

	if (cgl_Cookiebuf)
		free(cgl_Cookiebuf);

	cgl_Cookies = NULL;
	cgl_Cookiebuf = NULL;

	return;
}

/*
 * return pointer to value for name from cgl_Formdata.
 * if there is more than one value, returns first one.
 */
char *
cgl_getvalue(char *name)
{
	return cgl_getnodevalue(cgl_Formdata, name);
}

/*
 * return array of pointers to values for name from cgl_Formdata.
 */
char **
cgl_getvalues(int *count, char *name)
{
	return cgl_getnodevalues(cgl_Formdata, count, name);
}

/*
 * return pointer to value for name from cgl_Cookies.
 * if there is more than one value, returns first found.
 */
char *
cgl_getcookie(char *name)
{
	return cgl_getnodevalue(cgl_Cookies, name);
}

/*
 * return array of pointers to values for name from cgl_Cookies.
 */
char **
cgl_getcookies(int *count, char *name)
{
	return cgl_getnodevalues(cgl_Cookies, count, name);
}

/* ------------------------------------------------------------------------ *
 * NAME/VALUE PARSING
 * ------------------------------------------------------------------------ */

int 
cgl_parsecgibuf(cgllist *cdata, char *query)
{
	char *s;
	char *np;
	char *vp;

	if (!cdata || !query) {
		cgl_seterrno(CGL_NULLARG);
		return -1;
	}

	/* get rid of + signs */
	cgl_charify(query, '+', ' ');

	np = s = query;

	while (*np) {

		/* find name/value pairs */
		for ( ; *s && *s != '&'; s++) ;

		/* chop them up */
		if (*s == '&') {
			*s = '\0'; s++;
		}
		if ((vp = strchr(np, '=')) != NULL) {
			*vp = '\0';
			vp++;
			cgl_urlunescape(np);
			cgl_urlunescape(vp);
			if (cgl_insertnode(cdata, 
			    np, vp, CGL_INSERT_TAIL) == -1) {
				return -1;
			}
		}

		np = s;
	}

	return 0;
}

/*
 * This is almost identical to cgl_parsecgibuf(). Kept separate
 * as the cookie protocol may change (see for example rfc2109) 
 * and become more complicated.
 */
int 
cgl_parsecookiebuf(cgllist *cdata, char *cookies)
{
	char *s;
	char *np;
	char *vp;

	if (!cdata || !cookies) {
		cgl_seterrno(CGL_NULLARG);
		return -1;
	}

	np = s = cookies;

	while (*np) {

		/* find name/value pairs */
		for ( ; *s && *s != ';'; s++) ;

		/* chop them up */
		if (*s == ';') {
			*s = '\0';
			s++;
			while(isspace((unsigned char)*s))
				s++;
		}

		/*
		 * currently no whitespace is permitted around
		 * the '=' in the name/value pairs. This may change,
		 * see rfc2109.
		 */
		if ((vp = strchr(np, '=')) != NULL) {
			*vp = '\0';
			vp++;
			cgl_urldecode(np);
			cgl_urldecode(vp);
			if (cgl_insertnode(cdata, 
			    np, vp, CGL_INSERT_TAIL) == -1) {
				return -1;
			}
		}

		np = s;
	}

	return 0;
}


/* ------------------------------------------------------------------------ *
 * ENCODING AND DECODING
 * ------------------------------------------------------------------------ */

int 
cgl_urlencode(char *s, FILE *fw)
{
	if (!s)
		return 0;

	cgl_charify(s, ' ', '+');

	return cgl_urlescape(s, fw);
}

void 
cgl_urldecode(char *s)
{
	cgl_charify(s, '+', ' ');
	cgl_urlunescape(s);

	return;
}

int 
cgl_urlescape(char *s, FILE *fw)
{
	register int	c;

	while((c = *s++) != (char)0) {
		switch(c) {
		case '\0':
			break;
		case '%': case ' ': case '?': case '&':
		case '>': case '<': case '\"': case ';':
		case '=': case '@': case ':': case '#':
			fprintf(fw, "%%%02x", c);
			break;
		default:
			if (fputc(c, fw) == EOF)
				return EOF;
			break;
		}
	}

	return 0;
}

/* 
 * modified from the Apache code. Code shrinks string, so can
 * be done in place.
 */
int 
cgl_urlunescape(char *s)
{
	int	error;
	char	*p;

	if (!s)
		return 0;

	error = 0;

	for (p = s; *s; s++, p++) {
		if (*s != '%') {
			*p = *s;
			continue;
		} else {
			if (!isxdigit((unsigned char)*(s+1)) || !isxdigit((unsigned char)*(s+2))) {
				error = 1;
				*p = '%';
			} else {
				*p = cgl_hex2char((s+1));
				s++; s++;
				if (*p == '/' || *p == (char)0)
					error = 1;
			}
		}
	}

	*p = (char)0;
	if (error)
		return -1;
	return 0;
		
}

int 
cgl_htmlencode(char *s, FILE *fw)
{
	if (!s)
		return 0;

	return cgl_htmlescape(s, fw);
}

void 
cgl_htmldecode(char *s)
{
	if (s) {
		cgl_htmlunescape(s);
	}
	
	return;
}


short	transtab[256] = {
	   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  -1, -1, 0, -1, -1,
	   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	   0,  0,  34, 0,  0,  0,  38, 0,  0,  0,  0,  0,  0,  0,  0,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  60,  0, 62,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

/*
 * translate special chars into escape codes on way out.
 * If done into buffer would require lots of playing with memory;
 * this is easy way out.
 */
int
cgl_htmlescape(const char *s, FILE *fw)
{
	short	c;
	char	*cp;

	for (cp = (char *)s; *cp; cp++) {
		c = transtab[(unsigned char)(*cp)];
		switch(c) {
		case 0:
			if (fputc(*cp, fw) == EOF)
				return EOF;
			break;
		case -1:
			break;
		default:
			fprintf(fw, "&#%d;", c);
			break;
		}
	}

	return 0;

}

void
cgl_htmlunescape(char *s)
/* html decode numeric entities; should do symbolic, but not yet.
   String s will shrink so can overwrite contents safely. */
{
	int	n;
	char	*t;
	char	*e;

	for (t = s; *s; s++, t++) {
		if (*s != '&') {
			*t = *s;
			continue;
		}
		/* numeric begin */
		if (*(s+1) == '#') {	
			/* find end */
			for (e = s; *e != ';' && *e; e++ )
				continue;
			/* no good, do not decode */
			if (*e == (char)0) {
				*t = *s;
				continue;
			}
			/* advance past "&#" */
			s++; s++;
			/* get value */
			for(n = 0; isdigit((unsigned char)*s) && *s; s++)
				n = n * 10 + (*s - '0');
			if (n <= 8 || (n >= 11 && n <=31) ||
			    (n >= 127 && n < 160) || n >= 256) {
				/* back up, save '&' */
				s = t;  
				continue;
			} else {
				*t = n;
				s = e;
			}
		}
	}

	*t = (char)0;

	return;
}

/* 
 * ripped off from the Apache code
 */
char 
cgl_hex2char(char *what)
{
	register char digit;

	if (!what)
		return (char)0;

	digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
	digit *= 16;
	digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));

	return(digit);
}  

/*
 * change 'from''s to 'to''s
 */
void 
cgl_charify(char *s, char from, char to)
{
	char	*cp;

	if (!s)
		return;
	
	cp = s;

	while ((cp = strchr(cp, from)) != NULL) {
		*cp = to;
		cp++;
	}

	return;
}


/* ------------------------------------------------------------------------ *
 * OUTPUT
 * ------------------------------------------------------------------------ */

void 
cgl_html_header(void)
{
	printf("Content-type: text/html\n\n");
	fflush(stdout);
	return;
}

void 
cgl_content_header(char *type)
{
	printf("Content-type: %s\n\n", (type) ? type : NULLSTR);
	fflush(stdout);
	return;
}

void 
cgl_nph_header(char *version, char *status)
{
	if (!version) version = NULLSTR;
	if (!status) status = NULLSTR;

	printf("%s %s\n", version, status);
	return;
}

void 
cgl_status_header(char *status)
{
	printf("Status: %s\n", (status) ? status : NULLSTR);
	return;
}

void 
cgl_location_header(char *location)
{
	printf("Location: %s\n", (location) ? location : NULLSTR);
	return;
}

void 
cgl_pragma_header(char *s)
{
	printf("Pragma: %s\n", (s) ? s : NULLSTR);
	return;
}

int 
cgl_accept_image(void)
{
	if (!cgl_Env->http_accept) {
		cgl_seterrno(CGL_NOINIT);
		return -1;
	}

	if (strstr(cgl_Env->http_accept, "image") != NULL)
		return 1;
	return 0;
}

void 
cgl_html_begin(char *title)
{
	printf("<HTML>\n<HEAD>\n");
	printf("<TITLE>%s</TITLE>\n", (title) ? title : NULLSTR);
	printf("</HEAD>\n<BODY>\n");
	return;
}

void 
cgl_html_end(void)
{
	printf("</BODY>\n</HTML>\n");
	fflush(stdout);
	return;
}

void 
cgl_put_heading(int level, char *heading)
{
	if (level < 1) level = 1;
	else if (level > 6) level = 6;

	printf("<h%d>%s</h%d>\n", 
	    level, (heading) ? heading : NULLSTR, level);
	return;
}

void 
cgl_put_hidden(char *name, char *value)
{
	if (!name) name = NULLSTR;
	if (!value) value = NULLSTR;

	printf("<input type=hidden name=\"%s\" value=\"%s\">\n", name, value);
	return;
}


int 
cgl_put_cookie(char *name, char *opaque, 
    char *expires, char *path, char *domain, int secure)
{
	if (!name || !opaque) {
		cgl_seterrno(CGL_NULLARG);
		return -1;
	}

	printf("Set-Cookie: %s=%s;", name, opaque);


	if (expires != NULL)
		printf(" expires=%s;", expires);
	if (path != NULL)
		printf(" path=%s;", path);
	if (domain != NULL)
		printf(" domain=%s;", domain);
	if (secure)
		printf(" secure");
	printf("\n");

	return 0;

}

/* ------------------------------------------------------------------------ *
 * UTILITY
 * ------------------------------------------------------------------------ */

/*
 * return a pointer to a buffer containing an
 * ascii string formatted according to the Netscape
 * preliminary specification for cookies.
 */
char *cgl_cookietime(time_t *t)
{
	struct tm *tm;
	static char tbuf[36];

	tm = gmtime(t);

	strftime(tbuf, sizeof tbuf, "%A, %d-%b-%y %H:%M:%S GMT", tm);

	return tbuf;
}

/*
 * concatentate s2 onto s1, return pointer to new string.
 */
char *
cgl_stradd(char *s1, char *s2)
{
	size_t len = 0;
	char *buf;

	if (!s1 && !s2)
		return NULL;

	if (s1)
		len += strlen(s1);
	if (s2)
		len += strlen(s2);

	
	buf = malloc(len+1);
	if (!buf) {
		cgl_seterrno(CGL_SYSERR);
		return NULL;
	}

	if (s1) {
		strcpy(buf, s1);
		if (s2)
			strcat(buf, s2);
	} else strcpy(buf, s2);
		
	return buf;
}

/*
 * similar to above, generate a file path from path, 
 * file and extension.
 */
char *
cgl_mkpath(char *p, char *f, char *x)
{
	char	*buf;
	size_t	pl = 0;
	size_t	fl = 0;
	size_t	xl = 0;
	size_t	i  = 0;

	if (!p && !f)
		return NULL;

	if (p)
		pl = strlen(p);
	if (f)
		fl = strlen(f);
	if (x)
		xl = strlen(x)+1;

	buf = malloc(pl+fl+xl+2);
	if (!buf) {
		cgl_seterrno(CGL_SYSERR);
		return NULL;
	}

	if (p) {
		strcpy(buf, p);
		i = pl;
		if (f) {
			buf[i++] = '/';
			strcpy((buf+i), f);
			i += fl;
		}
	} else if (f) {
		i += pl;
		strcpy(buf+i, f);
	}

	if (x) {
		buf[i++] = '.';
		strcpy(buf+i, x);
	}

	return buf;
}

/*
 * Our strdup() -- so we can be more strictly ansi.
 */
char *
cgl_strdup(const char *s)
{
	char	*cp;
	size_t	len;

	len = strlen(s) + 1;

	cp = malloc(len);
	if (cp == NULL)
		return NULL;

	(void)memcpy(cp, s, len);

	return cp;
}

/*
 * getenv(), but return a pointer to a zero-length
 * string, rather than NULL. So str* functions don't
 * blow up when used by naive (or lazy) programmers.
 */
char *
cgl_getenv(char *name)
{
	char	*ep;
	
	ep = getenv(name);

	return (ep) ? ep : "";
}

/* ------------------------------------------------------------------------ *
 * ERRORS AND DEBUGGING
 * ------------------------------------------------------------------------ */

void 
cgl_perror(FILE *fw, char *s)
{

	fprintf(fw, "%s: %s\n", s, cgl_strerror());

	return;
}

char *
cgl_strerror(void)
{
	if (cglerrno == CGL_SYSERR)
		return strerror(cglsyserr);
	else if (cglerrno > CGL_MAXERRNO)
		return "(cglerrno out of range!)";

	return cglerrlist[cglerrno];
}

static void 
cgl_seterrno(int n)
{
	if (n == CGL_SYSERR)
		cglsyserr = errno;
	else
		cglsyserr = 0;
		
	cglerrno = n;

	return;
}

/*
 * dump everything we know, except for the hash table 
 * contents, to fw.
 */
void 
cgl_dump(FILE *fw)
{
	fprintf(fw, "Environment:\n");
	cgl_dumpenv(fw);

	fprintf(fw, "Form data:\n");
	cgl_dumpform(fw);

	fprintf(fw, "Hash stats:\n");
	cgl_dumphstats(cgl_Formdata, fw);

	return;
}

/*
 * dump the contents of the global environment structure 
 * cgl_Env to fw.
 */
void 
cgl_dumpenv(FILE *fw)
{
	if (!cgl_Env) {
		cgl_seterrno(CGL_NOINIT);
		return;
	}

	fprintf(fw, "  AUTH_TYPE:         %s\n", 
	    (cgl_Env->auth_type[0]) ? cgl_Env->auth_type : "NULL");
	fprintf(fw, "  CONTENT_LENGTH:    %s\n", 
	    (cgl_Env->content_length[0]) ? cgl_Env->content_length : "NULL");
	fprintf(fw, "  CONTENT_TYPE :     %s\n", 
	    (cgl_Env->content_type[0] ) ? cgl_Env->content_type  : "NULL");
	fprintf(fw, "  DOCUMENT_ROOT:     %s\n",
	    (cgl_Env->document_root[0]) ? cgl_Env->document_root : "NULL");
	fprintf(fw, "  GATEWAY_INTERFACE: %s\n", 
	    (cgl_Env->gateway_interface[0]) ? cgl_Env->gateway_interface: "NULL");
	fprintf(fw, "  HTTP_ACCEPT:       %s\n", 
	    (cgl_Env->http_accept[0]) ? cgl_Env->http_accept : "NULL");
	fprintf(fw, "  HTTP_COOKIE:       %s\n", 
	    (cgl_Env->http_cookie[0]) ? cgl_Env->http_cookie : "NULL");
	fprintf(fw, "  HTTP_PRAGMA:       %s\n",
	    (cgl_Env->http_pragma[0]) ? cgl_Env->http_pragma : "NULL");
	fprintf(fw, "  HTTP_USER_AGENT:   %s\n", 
	    (cgl_Env->http_user_agent[0]) ? cgl_Env->http_user_agent : "NULL");
	fprintf(fw, "  PATH_INFO:         %s\n", 
	    (cgl_Env->path_info[0]) ? cgl_Env->path_info : "NULL");
	fprintf(fw, "  PATH_TRANSLATED:   %s\n", 
	    (cgl_Env->path_translated[0]) ? cgl_Env->path_translated : "NULL");
	fprintf(fw, "  QUERY_STRING:      %s\n", 
	    (cgl_Env->query_string[0]) ? cgl_Env->query_string : "NULL");
	fprintf(fw, "  REMOTE_ADDR:       %s\n", 
	    (cgl_Env->remote_addr[0]) ? cgl_Env->remote_addr : "NULL");
	fprintf(fw, "  REMOTE_HOST:       %s\n", 
	    (cgl_Env->remote_host[0]) ? cgl_Env->remote_host : "NULL");
	fprintf(fw, "  REMOTE_IDENT:      %s\n", 
	    (cgl_Env->remote_ident[0]) ? cgl_Env->remote_ident : "NULL");
	fprintf(fw, "  REMOTE_PORT:       %s\n",
	    (cgl_Env->remote_port[0]) ? cgl_Env->remote_port : "NULL");
	fprintf(fw, "  REMOTE_USER:       %s\n",
	    (cgl_Env->remote_user[0]) ? cgl_Env->remote_user : "NULL");
	fprintf(fw, "  REQUEST_METHOD:    %s\n", 
	    (cgl_Env->request_method[0]) ? cgl_Env->request_method : "NULL");
	fprintf(fw, "  REQUEST_URI:       %s\n",
	    (cgl_Env->request_uri[0]) ? cgl_Env->request_uri : "NULL");
	fprintf(fw, "  SCRIPT_FILENAME:   %s\n",
	    (cgl_Env->script_filename[0]) ? cgl_Env->script_filename : "NULL");
	fprintf(fw, "  SCRIPT_NAME:       %s\n", 
	    (cgl_Env->script_name[0]) ? cgl_Env->script_name : "NULL");
	fprintf(fw, "  SERVER_ADMIN:      %s\n",
	    (cgl_Env->server_admin[0]) ? cgl_Env->server_admin : "NULL");
	fprintf(fw, "  SERVER_NAME:       %s\n", 
	    (cgl_Env->server_name[0]) ? cgl_Env->server_name : "NULL");
	fprintf(fw, "  SERVER_PORT:       %s\n", 
	    (cgl_Env->server_port[0]) ? cgl_Env->server_port : "NULL");
	fprintf(fw, "  SERVER_PROTOCOL:   %s\n", 
	    (cgl_Env->server_protocol[0]) ? cgl_Env->server_protocol : "NULL");
	fprintf(fw, "  SERVER_SOFTWARE:   %s\n", 
	    (cgl_Env->server_software[0]) ? cgl_Env->server_software : "NULL");


	return;
}

void 
cgl_dumpform(FILE *fw)
{
	if (!cgl_Formdata) {
		cgl_seterrno(CGL_NOINIT);
		return;
	}

	cgl_dumpdata(cgl_Formdata, fw);	

	return;
}

/* ------------------------------------------------------------------------ *
 * HASHED LIST FUNCTIONS
 * ------------------------------------------------------------------------ */

/*
 * return pointer to values for name from cdata.
 */
char *
cgl_getnodevalue(cgllist *cdata, char *name)
{
	if (!cdata)
		cgl_seterrno(CGL_NOINIT);
	else if (cgl_fetchnode(cdata, name))
		return cdata->cur->value;
	
	return NULL;
}

/*
 * return array of pointers to values for name from cdata.
 */
char **
cgl_getnodevalues(cgllist *cdata, int *count, char *name)
{
	int	ac;
	char	**ap;
	char	**av;

	cglnode *np;

	if (!cdata) {
		cgl_seterrno(CGL_NOINIT);
		return NULL;
	}

	ac = 0;
	av = NULL;

	/*
	 * fetchnode, then follow list of duplicates to build array.
	 */
	if (cgl_fetchnode(cdata, name)) {

		/*
		 * we could spare the two passes if we knew we had a
		 * reasonable maximum value limit. This is slower, 
		 * but has no surprise limits...
		 */

		for (np = cdata->cur; np; np = np->dupe)
			ac++;

		if (ac) {

			av = calloc(ac + 1, sizeof(char *));
			if (!av)
				goto abort_on_error;

			for (ap=av, np=cdata->cur; np; ap++, np=np->dupe)
				*ap = np->value;
		}
	}

	*count = ac;
	return av;

abort_on_error:

	/* if we fail, it's on malloc */
	cgl_seterrno(CGL_SYSERR);

	*count = -1;
	return NULL;
}

/* 
 * find the list member containing name, set cdata->cur
 * to point to it. Returns 1 on success, 0 on failure.
 * 
 * Multiple values that have the same name are returned
 * as a list off cdata->cur->dupe. Note that the linear
 * search has to find duplicates (so every search searchs
 * the entire node list), but the hash search gets them
 * for free, as they were already found when hashing the
 * nodes in the first place.
 */
cglnode *
cgl_fetchnode(cgllist *cdata, char *name)
{
	int	hvl;

	cglnode   *cp;
	cglnode   *np;


	if (!cdata) {
		cgl_seterrno(CGL_NOINIT);
		return NULL;
	}

	if (!name) {
		cgl_seterrno(CGL_NULLARG);
		return NULL;
	}

	/* use hashtable if can */
	if (cdata->hash.h_control != CGL_HASH_OFF 
	    && cdata->hash.h_table != NULL) {

		hvl = cgl_hashpjw(name, cdata->hash.h_size);

		for (cp = cdata->hash.h_table[hvl]; cp; cp = cp->bucket) {
			if (strcmp(cp->name, name) == 0)
				return cdata->cur = cp;
		}			

	} else {

		for (np = NULL, cp = cdata->head; cp; cp = cp->next) {
			if (strcmp(cp->name, name) == 0) {
				if (np)
					np->dupe = cp;
				else
					cdata->cur = cp;
				np = cp;
			}
		}

		if (np)
			return cdata->cur;
	}

	return 0;
}

/*
 * insert a name=value pair into the list cdata, the position of the
 * insertion governed by 'where'. If the insertion would cause
 * the list to grow beyond cdata->hash.h_mincount (CGL_MINTOHASH)
 * elements, and the list is not already hashed, the element is inserted 
 * and cgl_hashlist() is called  on the list. Future lookups in the list 
 * will then use the hash table to find nodes.
 *
 * if the table is already hashed, and with the new insertion 
 * the hash table becomes over-populated, or there are too
 * many collisions, cgl_hashlist() is called to resize the table.
 *
 * the int cdata->hash.h_control can be set to coerce behaviour. If
 * set to CGL_HASH_OFF (-1), the list will never be hashed. If
 * set to CGL_HASH_AUTO (0, the default) the automatic behaviour
 * above takes place. If set to some positive integer, that value
 * is used for the initial hashtable size, and elements are
 * hashed from the very start of insertion. You may wish to use
 * this if you know that you have a large number of name=value
 * pairs returning from a form, and especially if you know you
 * are going to do repetitive searchs on values not known at
 * compile time: i.e. parsing a template/script.
 *
 * cdata has a maximum capacity of cdata->maxcount items,
 * initialized to CGL_MAXCOUNT (1000) by cgl_newlist(). If you
 * are returning more items than this, you will need to adjust this.
 * The size limit is to prevent abuse of a form by returning
 * massive quantities of name=value pairs. You may even wish to
 * set this lower in specific programs where you know the number
 * values to be returned is low.
 *
 * NOTE: memory is not allocated for storing name and value!
 * name and value likely point into a 'chopped up' buffer, and we
 * don't want to do unnecessary allocations. If name and value
 * are local or subject to overwriting, strdup() them FIRST, then
 * pass the duplicates! 
 *
 * NOTE: And, related to this, cgl_freedata() does not free 
 * name/value pointers, only the linked list nodes. If you're 
 * creating and destroying name/value data sets, this is a 
 * wonderful opportunity to create a massive memory leak.
 */
int 
cgl_insertnode(cgllist *cdata, char *name, void *value, int where)
{
	int	retval;
	cglnode *node;


	if (!cdata) {
		cgl_seterrno(CGL_NOINIT);
		return -1;
	}

	if (!name) {
		cgl_seterrno(CGL_NULLARG);
		return -1;
	}

	if (cdata->maxcount && cdata->count >= cdata->maxcount) {
		cgl_seterrno(CGL_TOOMANY);
		return -1;
	}

	/* create the node */
	node = calloc(1, sizeof(cglnode));
	if (!node) {
		cgl_seterrno(CGL_SYSERR);
		return -1;
	}
		
	node->name = name;
	node->value = value;

	if (cgl_hashsize(cdata) == -1)
		return -1;

	/*
	 * if we've started hashing, nodes continue to be
	 * hashed in, even if hash.h_control is set to CGL_HASH_OFF.
	 * Auto sizing is toggled off, and the search routines will
	 * now walk the linked list of name/value pairs, but
 	 * hashing doesn't actually stop until cgl_hashlist() is
	 * called with a hashsz parameter of 0, which blows the
	 * hash table away.
	 */
	if (cdata->hash.h_table) 
		if ((retval = cgl_hashnode(cdata, node)) <= 0)
			return retval;

	if (cgl_linknode(cdata, node, where) == -1)
		return -1;


	cdata->count++;

	return 0;
}

/*
 * adjust the hash behaviour: build hash table if needed,
 * adjust size as called for.
 */
static int
cgl_hashsize(cgllist *cdata)
{
	int	hashsz;
	int	control;


	hashsz = cdata->hash.h_size;
	control = cdata->hash.h_control;

	/* 
	 * - if hashing is off, as long as h_unique is not
	 *   set, return. But if h_unique _is_ set, then behave
	 *   as if the default hashing was on: we need hashing
	 *   to enforce uniqueness--else we do a search of the
	 *   list on each insertion. yuck.
	 * - if hashing is on, or if insertion will cross the
	 *   hashing threshold and turn hashing on, insert the
	 *   node into the hash table
	 * - if the collision count grows past 50% of the
	 *   size of the table, resize the table
	 */

	switch(control) {
	case CGL_HASH_OFF:

		if (cdata->hash.h_unique != CGL_HASH_UNIQUE)
			return 0;
		/* FALLTHROUGH */

	case CGL_HASH_AUTO:

		if (cdata->count > cdata->hash.h_mincount) {
			if (hashsz == 0)
				hashsz = cgl_nexthashsz(cdata);
			else {
				/* resize */
				if (cdata->hash.h_collisions 
				    > (hashsz / 2)) {
					hashsz = cgl_nexthashsz(cdata);
				}
			}
		}

		break;

	default:

		/* anything greater than HASH_AUTO is taken as
		   an explicit hash table size */
		if (control > CGL_HASH_AUTO)
			hashsz = control;
	
		/* impossible/incorrect value for hash */
		else if (control < CGL_HASH_OFF) {
			cgl_seterrno(CGL_BADHASHCTL);
			return -1;
		}

		break;
	}

	/* if hashsz to set != current size, rehash */
	if (hashsz != cdata->hash.h_size) {
		if (cgl_hashlist(cdata, hashsz) == -1)
			return -1;
	}


	return 0;
}

/*
 * insert a node into the hash table. Return 0 on success,
 * and -1 if something went wrong (in malloc).
 */
static int 
cgl_hashnode(cgllist *cdata, cglnode *node)
{
	int	hvl;
	int	retval;
	cglnode *hp;
	cglnode *op;

	assert(cdata != NULL);
	assert(node != NULL);

	retval = 0;

	/* 
	 * forget any dupes: necessary if linear search has
	 * been done before hashing invoked. Never in CGI
	 * form data, but if using routines for anything
	 * else...
	 */
	node->dupe = NULL;


	/* insert the node into the hashtable */

	hvl = cgl_hashpjw(node->name, cdata->hash.h_size);

	op = cdata->hash.h_table[hvl];

	for (hp = op; hp; hp = hp->bucket) {
		if (strcmp(hp->name, node->name) == 0) {
			if (cdata->hash.h_unique == CGL_HASH_UNIQUE) {
				 /* make pointers match */
				hp->name = node->name;
				hp->value = node->value;
				/* 
				 * don't need your stink'n node: 
				 * no changes to list
				 */
				free(node);
				retval=0;
			} else {
				node->dupe = hp->dupe;
				hp->dupe = node;
				retval=1;
			}
			break;
		}
	}


	/* not a new dupe, name doesn't exist in table yet */
	if (!hp) {
		/* do stats: record collisions and hash count
		   for _unique_ names (# of name/value pairs
		   is stored in list count, and we don't care
		   when we come to look at how the hash table
		   is performing) */
		if (op)				/* slot has an entry */
			cdata->hash.h_collisions++;
		else				/* slot not used yet */
			cdata->hash.h_used++;
		cdata->hash.h_count++;

		node->bucket = op;
		cdata->hash.h_table[hvl] = node;
		retval = 1;
	}

	return retval;
}

static int
cgl_linknode(cgllist *cdata, cglnode *node, int where)
{
	cglnode *head;
	cglnode *tail;
	cglnode *cur;

	/* insert the node in the linked list: now the fun begins */
	if (!cdata->head) {

		cdata->head = node;
		cdata->tail = node;
		cdata->cur = node;

	} else {

		head = cdata->head;
		tail = cdata->tail;
		cur  = cdata->cur;

		/* XXX cur had better NOT be NULL! */

		switch(where) {
		case CGL_INSERT_TAIL:
			node->prev = tail;
			tail->next = node;
			cdata->tail = node;
			cdata->cur = node;
			/* node->next is already NULL */
			break;
		case CGL_INSERT_HEAD:
			node->next = head;
			head->prev = node;
			cdata->head = node;
			cdata->cur = node;
			/* node->prev is already NULL */
			break;
		case CGL_INSERT_PREV:
			if (cur->prev)
				cur->prev->next = node;
			node->next = cur;
			node->prev = cur->prev;
			cur->prev = node;
			cdata->cur = node;
			break;
		case CGL_INSERT_NEXT:
			if (cur->next)
				cur->next->prev = node;
			node->prev = cur;
			node->next = cur->next;
			cur->next = node;
			cdata->cur = node;
			break;
		default:
			cgl_seterrno(CGL_BADINSERT);
			return -1; /* or just insert at tail ? */
			/* NOTREACHED */
			break;
		}

	}

	return 0;
}

/* 
 * create or resize the hashtable for a list
 */
int 
cgl_hashlist(cgllist *cdata, int hashsz)
{
	cglnode *np;

	if (!cdata) {
		cgl_seterrno(CGL_NOINIT);
		return -1;
	}

	/* if already hashed, free hash table and pointers */
	if (cdata->hash.h_table)
		cgl_freehash(cdata);

	if (hashsz > 0) {

		/* create the new hash table: sets cdata->hash.h_size */
		if (cgl_createhash(cdata, hashsz) == -1)
			return -1;	
	
		/* walk the data list inserting pointers to members
		   into the hashtable */
		for (np = cdata->head; np; np = np->next) {
			if (cgl_hashnode(cdata, np) == -1)
				return -1;
		}
	}

	return 0;
	
}

/*
 * create a hashed list suitable for handling CGI form and cookie
 * data; set the minimum-count-before-hashing and hash control to 
 * defaults.
 */
cgllist *
cgl_newlist(void)
{
	cgllist *dp;

	dp = calloc(1, sizeof(cgllist));
	if (!dp)
		cgl_seterrno(CGL_SYSERR);
	else {
		dp->hash.h_control  = cgl_def_hcontrol;
		dp->hash.h_unique   = CGL_HASH_DUPES;
		dp->hash.h_mincount = cgl_def_mincount;
		dp->maxcount        = cgl_def_maxcount;
	}

	return dp;
}

/*
 * create a hashed list to function like an associative array: 
 * enforce hashing, enforce unique keys, set datasize to be
 * unlimited.
 */
cgllist *
cgl_newhash(void)
{
	cgllist	*dp;

	dp = calloc(1, sizeof(cgllist));
	if (!dp)
		cgl_seterrno(CGL_SYSERR);
	else {
		dp->hash.h_control  = CGL_HASH_AUTO;
		dp->hash.h_unique   = CGL_HASH_UNIQUE;
		dp->hash.h_mincount = 0;
		dp->maxcount        = 0;
	}

	return dp;
}


/*
 * free a data set; lists, hash and all.
 *
 * NOTE: cgl_freedata() does not free  name/value pointers, it only 
 * frees the linked list nodes. If you're creating and destroying 
 * name/value data sets, failing to free the actual data first is a 
 * choice way to create a massive memory leak.
 */
void 
cgl_freedata(cgllist *cdata)
{
	cglnode *pp;
	cglnode *qp;

	if (cdata) {

		if (cdata->hash.h_table)
			free(cdata->hash.h_table);

		/* reset pointer and counters */
		cdata->hash.h_table = NULL;
		cdata->hash.h_size = 0;
		cdata->hash.h_count = 0;
		cdata->hash.h_collisions = 0;
		cdata->hash.h_used = 0;
		cdata->hash.h_resize = 0;
		/*
		 *  cdata->hash.h_control stays as is,
		 *  cdata->hash.h_unique stays as is,
		 *  cdata->hash.h_mincount stays as is
		 */

		for (qp = cdata->head; qp; qp = pp) {
			pp = qp->next;
			free(qp);
		}

		cdata->head = NULL;
		cdata->tail = NULL;
		cdata->cur  = NULL;
	}

	return;
}

/*
 * move along the list of values, set cdata->cur as asked
 * and return pointer to it.
 */
cglnode *
cgl_firstnode(cgllist *cdata)
{
	if (cdata)
		return cdata->cur = cdata->head;
	return NULL;
}

cglnode *
cgl_nextnode(cgllist *cdata)
{
	if (cdata && cdata->cur)
		return cdata->cur = cdata->cur->next;
	return NULL;
}

cglnode *
cgl_prevnode(cgllist *cdata)
{
	if (cdata && cdata->cur)
		return cdata->cur = cdata->cur->prev;
	return NULL;
}

cglnode *
cgl_lastnode(cgllist *cdata)
{
	if (cdata)
		return cdata->cur = cdata->tail;
	return NULL;
}

/*
 * free the table and pointers associated with a hashed list
 */
static void 
cgl_freehash(cgllist *cdata)
{
	assert(cdata != NULL);

	/* we could walk through the values list and reset
	   all the bucket (and dupe) pointers. But we're
	   not going to. */

	if (cdata->hash.h_table)
		free(cdata->hash.h_table);

	/* reset pointer and counters */
	cdata->hash.h_table = NULL;
	cdata->hash.h_size = 0;
	cdata->hash.h_count = 0;
	cdata->hash.h_collisions = 0;
	/*
	 *  cdata->hash.h_used stays as is,
	 *  cdata->hash.h_resize stays as is,
	 *  cdata->hash.h_control stays as is,
	 *  cdata->hash.h_unique stays as is,
	 *  cdata->hash.h_mincount stays as is
	 */

	return;
}

/*
 * attach a hash table to cdata->hash.h_table
 */
static int 
cgl_createhash(cgllist *cdata, int hashsz)
{
	assert(cdata != NULL);

	cdata->hash.h_table = calloc(hashsz, sizeof(cglnode *));
	if (!cdata->hash.h_table) {
		cgl_seterrno(CGL_SYSERR);
		return -1;
	}

	cdata->hash.h_size = hashsz;

	return 0;
}

/*
 * set the current hash table size from an ascending list of primes.
 * is one prime better than another? I just picked some pretty ones.
 */
static int 
cgl_nexthashsz(cgllist *cdata)
{
	static	int i;
	static	int primes[] = { 31, 131, 313, 919, 3319, 6691 };

	assert(cdata != NULL);

	i = cdata->hash.h_resize++;

	if (i < 5)
		return primes[i];

	return primes[5];
}

/* "An adaptation of Peter Weinberger's (PJW) generic hashing
   algorithm based on Allen Holub's version", from Dr.Dobb's,
   April 96 */

#define CHAR_BIT	8
#define BITS_IN_int     (sizeof(int) * CHAR_BIT)
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       (~((unsigned int)(~0) >> ONE_EIGHTH))

static unsigned int 
cgl_hashpjw(const char *datum, const int hashsz)
{
        unsigned int hash_value, i;

	assert(datum != NULL);

        for (hash_value = 0; *datum; ++datum) {
                hash_value = ( hash_value << ONE_EIGHTH) + * datum;
                if ((i = hash_value & HIGH_BITS) != 0)
                        hash_value =
                                (hash_value ^ ( i >> THREE_QUARTERS)) &
                                    ~HIGH_BITS;
        }

        return hash_value % hashsz;
}

/*
 * delete the node that cdata->cur points to. tricky bit is
 * disentangling it from the hash structure and dupe list.
 * Question du jour is: do we decrement collision and slot
 * use counts as we remove items?
 */
int 
cgl_deletenode(cgllist *cdata)
{
	int	hvl;
	cglnode *np;
	cglnode *hp;
	cglnode *cp;
	cglnode *op;
	cglnode *dop;
	cglnode *dcp;


	if (!cdata) {
		cgl_seterrno(CGL_NULLARG);
		return -1;
	}

	np = cdata->cur;

	/* no data in list, or cursor past head or tail... */
	if (!np)
		return 0;

	if (cdata->hash.h_table) {

		/* remove node from hash table */
		hvl = cgl_hashpjw(np->name, cdata->hash.h_size);

		hp = cdata->hash.h_table[hvl];

		assert(hp != NULL);

		/* is head of bucket list and dupe list? */
		if (np == hp) {
			if (np->dupe) {
				cdata->hash.h_table[hvl] = np->dupe;
				np->dupe->bucket = np->bucket;
			} else {
				cdata->hash.h_table[hvl] = np->bucket;
				cdata->hash.h_count--;
				/* if (!np->bucket) 
				 +	cdata->hash.h_used--;
				 + cdata->hash.h_collisions--;
				 */
			}
		} else {

			/* find in bucket list */
			for (op = cp = hp; cp; cp = cp->bucket) {
				if (strcmp(cp->name, np->name) == 0)
					break;
				op = cp;
			}

			/* head of dupe list? */
			if (cp == np) {
				if (np->dupe) {
					op->bucket = np->dupe;
					np->dupe->bucket = np->bucket;
				} else {
					op->bucket = np->bucket;
					cdata->hash.h_count--;
					/*
					 + cdata->hash.h_collisions--;
					 */
				}
			} else {

				/* find in dupe list */
				for (dop = dcp = cp; dcp; dcp = dcp->dupe) {
					if (dcp == np)
						break;
					dop = dcp;
				}

				/* ok:
					dcp == np
					dop == dupe member before np
				*/
				dop->dupe = np->dupe;
				/* and we're done. Whew! */
			}
		}

	}

	/* 
	 * remove node from linked list. cdata->cur is left pointing
 	 * to the next node, unless it is null, in which case cdata
	 * cur is left pointing to the previous node. Will this be
	 * problem in loops? Need some more thought about this.
	 */
	if (np == cdata->head)
		cdata->head = cdata->head->next;

	if (np == cdata->tail)
		cdata->tail = cdata->tail->prev;

	if (np->next) {
		np->next->prev = np->prev;
		cdata->cur = np->next;
	} else
		cdata->cur = np->prev;

	if (np->prev)
		np->prev->next = np->next;

	cdata->count--;

	/* free at last */
	free(np);

	return 0;
}

void 
cgl_dumpdata(cgllist *cdata, FILE *fw)
{
	cglnode *np;

	assert(cdata != NULL);

	for (np = cdata->head; np; np=np->next)
		fprintf(fw, "  %s: %s\n", np->name, (char *)(np->value));

	return;
}

void 
cgl_dumphstats(cgllist *cdata, FILE *fw)
{

	assert(cdata != NULL);

	fprintf(fw, "  h_control:    %d\n", cdata->hash.h_control);
	fprintf(fw, "  h_count:      %d\n", cdata->hash.h_count);
	fprintf(fw, "  h_collisions: %d\n", cdata->hash.h_collisions);
	fprintf(fw, "  h_used:       %d\n", cdata->hash.h_used);
	fprintf(fw, "  h_resize:     %d\n", cdata->hash.h_resize);
	fprintf(fw, "  h_size:       %d\n", cdata->hash.h_size);
	fprintf(fw, "  h_table:      %s\n", 
	    (cdata->hash.h_table) ? "allocated" : "not allocated");

	return;
}

void 
cgl_dumphash(cgllist *cdata, FILE *fw)
{
	int	i;

	cglnode *hp;
	cglnode *bp;
	cglnode *np;

	assert(cdata != NULL);

	if (cdata->hash.h_table) {
		for (i = 0;  i < cdata->hash.h_size; i++) {
			hp = cdata->hash.h_table[i];
			if (hp) {
				fprintf(fw, "hash %d:\n", i);
				for (bp = hp; bp; bp = bp->bucket) {
				    fprintf(fw, "  n/v: \"%s\", \"%s\"\n",
					bp->name, (char *)(bp->value));
				    for (np = bp->dupe; np; np=np->dupe) {
					fprintf(fw, "    dupe: \"%s\", \"%s\"\n",
					    np->name, (char *)(np->value));
				    }
				}
			}
		}

	}

	return;
}

/*
 * as cgl_dumpdata() but print values as pointer values. If
 * you're storing something other than strings as values.
 */
void 
cgl_dumprawdata(cgllist *cdata, FILE *fw)
{
	cglnode *np;

	assert(cdata != NULL);

	for (np = cdata->head; np; np=np->next)
		fprintf(fw, "  %s: %p\n", np->name, np->value);

	return;
}

/*
 * as cgl_dumphash() but print values as pointer values. If
 * you're storing something other than strings as values.
 */
void 
cgl_dumprawhash(cgllist *cdata, FILE *fw)
{
	int	i;

	cglnode *np;
	cglnode *hp;
	cglnode *bp;

	assert(cdata != NULL);

	if (cdata->hash.h_table) {
		for (i = 0;  i < cdata->hash.h_size; i++) {
			hp = cdata->hash.h_table[i];
			if (hp) {
				fprintf(fw, "hash %d:\n", i);
				for (bp = hp; bp; bp = bp->bucket) {
				    fprintf(fw, "  n/v: \"%s\", %p\n",
					bp->name, bp->value);
				    for (np = bp->dupe; np; np=np->dupe) {
					fprintf(fw, "    dupe: \"%s\", %p\n",
					    np->name, np->value);
				    }
				}
			}
		}

	}

	return;
}

/* END */





