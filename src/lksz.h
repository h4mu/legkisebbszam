/***************************************************************************
 *   Copyright (C) 2006 by Hámor Tamás   *
 *   hamuguy@gmail.com   *
 *                                                                         *
 *   Permission is hereby granted, free of charge, to any person obtaining *
 *   a copy of this software and associated documentation files (the       *
 *   "Software"), to deal in the Software without restriction, including   *
 *   without limitation the rights to use, copy, modify, merge, publish,   *
 *   distribute, sublicense, and/or sell copies of the Software, and to    *
 *   permit persons to whom the Software is furnished to do so, subject to *
 *   the following conditions:                                             *
 *                                                                         *
 *   The above copyright notice and this permission notice shall be        *
 *   included in all copies or substantial portions of the Software.       *
 *                                                                         *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR     *
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, *
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR *
 *   OTHER DEALINGS IN THE SOFTWARE.                                       *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*#define DEBUG 1*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifdef CYGWIN
#	include <cygwin/in.h>
#endif
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>

#include <limits.h>

#define MSGLEN 83	/* "%80s\r\l\0" */
#define PARMLEN 51
#define MAXCONN 32
#define STDIN 0
#define INVALID 0

/*	LEGKISEBB-BEJELENTKEZES %80[^\00-\32]
	LEGKISEBB-MEGAD %Ld
	LEGKISEBB-NYERT %80[^\00-\32]
	LEGKISEBB-SZAM %Ld
	LEGKISEBB-KIJELENTKEZES %80[^\00-\32]
*/

