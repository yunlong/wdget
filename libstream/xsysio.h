/*
 * Copyright (C) 2000-2003 the xine project,
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * abortable i/o helper functions
 */

#ifndef _XSYS_IO_H
#define _XSYS_IO_H


#ifdef __cplusplus
extern "C" {
#endif

#include "bswap.h"
#include "xbuffer.h"
#include "xsysutil.h"

#include <errno.h>
#include <assert.h>
#include <limits.h>
#ifdef __GNUC__
# include <unistd.h>
# include <inttypes.h>
#else
# include "stdwin.h"
#endif

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>


/* select states */
#define XIO_READ_READY    1
#define XIO_WRITE_READY   2

/* xine select return codes */
#define XIO_READY         0
#define XIO_ERROR         1
#define XIO_ABORTED       2
#define XIO_TIMEOUT       3


extern int
xio_select(int fd, int state, int timeout_msec);


extern int
xio_tcp_connect(const char *host, int port);

extern int
xio_tcp_connect_finish(int fd, int timeout_msec);

extern off_t
xio_tcp_read(int s, void *buf, off_t todo);

extern off_t
xio_tcp_write(int s, void *buf, off_t todo);

extern off_t
xio_file_read(int fd, void *buf, off_t todo);

extern off_t
xio_file_write(int fd, void *buf, off_t todo);


extern int
xio_tcp_read_line(int sock, char *str, int size);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef _XSYS_IO_H */


