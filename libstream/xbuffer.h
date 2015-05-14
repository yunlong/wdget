/* 
 * Copyright (C) 2000-2004 the xine project
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
 * $Id: xbuffer.h,v 1.4 2004/09/26 22:54:53 valtri Exp $
 *
 *
 * generic dynamic buffer functions. The goals
 * of these functions are (in fact many of these points
 * are todos):
 * - dynamic allocation and reallocation depending
 *   on the size of data written to it.
 * - fast and transparent access to the data.
 *   The user sees only the raw data chunk as it is
 *   returned by the well-known malloc function.
 *   This is necessary since not all data-accessing
 *   functions can be wrapped here.
 * - some additional health checks are made during
 *   development (eg boundary checks after direct
 *   access to a buffer). This can be turned off in
 *   production state for higher performance.
 * - A lot of convenient string and memory manipulation
 *   functions are implemented here, where the user
 *   do not have to care about memory chunk sizes.
 * - Some garbage collention could be implemented as well;
 *   i think of a global structure containing infos
 *   about all allocated chunks. This must be implemented
 *   in a thread-save way...
 *
 * Here are some drawbacks (aka policies):
 * - The user must not pass indexed buffers to xbuffer_*
 *   functions.
 * - The pointers passed to xbuffer_* functions may change
 *   (eg during reallocation). The user must respect that.
 */

#ifndef HAVE_XBUFFER_H
#define HAVE_XBUFFER_H


#ifdef __GNUC__
# include <inttypes.h>
#else
# include "stdwin.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*
 * returns an initialized pointer to a buffer.
 * The buffer will be allocated in blocks of
 * chunk_size bytes. This will prevent permanent
 * reallocation on slow growing buffers.
 */
extern void *
xbuffer_init(int chunk_size);

extern void *
_xbuffer_free(void *buf);
/*
 * frees a buffer, the macro ensures, that a freed
 * buffer pointer is set to NULL
 */
#define xbuffer_free(buf) \
  *(void **)&buf = _xbuffer_free(buf)

/*
 * duplicates a buffer
 */
extern void *
xbuffer_dup(void *buf);

extern void *
_xbuffer_copyin(void *buf, int index, const void *data, int len);
/*
 * will copy len bytes of data into buf at position index.
 */
#define xbuffer_copyin(buf,i,data,len) \
  *(void **)&buf = _xbuffer_copyin(buf,i,data,len)

/*
 * will copy len bytes out of buf+index into data.
 * no checks are made in data. It is treated as an ordinary
 * user-malloced data chunk.
 */
extern void
xbuffer_copyout(void *buf, int index, void *data, int len);

extern void *
_xbuffer_set(void *buf, int index, uint8_t b, int len);
/*
 * set len bytes in buf+index to b.
 */
#define xbuffer_set(buf,i,b,len) \
  *(void **)&buf = _xbuffer_set(buf,i,b,len)

extern void *
_xbuffer_strcat(void *buf, char *data);
/*
 * concatnates given buf (which schould contain a null terminated string)
 * with another string.
 */
#define xbuffer_strcat(buf,data) \
  *(void **)&buf = _xbuffer_strcat(buf,data)

extern void *
_xbuffer_strcpy(void *buf, int index, char *data);
/*
 * copies given string to buf+index
 */
#define xbuffer_strcpy(buf,index,data) \
  *(void **)&buf = _xbuffer_strcpy(buf,index,data)

/*
 * returns a pointer to the first occurence of ch.
 * note, that the returned pointer cannot be used
 * in any other xbuffer_* functions.
 */
extern char *
xbuffer_strchr(void *buf, int ch);

/*
 * get allocated memory size
 */
extern int
xbuffer_get_size(void *buf);

extern void *
_xbuffer_ensure_size(void *buf, int size);
/*
 * ensures a specified buffer size if the user want to
 * write directly to the buffer. Normally the special
 * access functions defined here should be used.
 */
#define xbuffer_ensure_size(buf,data) \
  *(void **)&buf = _xbuffer_ensure_size(buf,data)


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef HAVE_XBUFFER_H */


