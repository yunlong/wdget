/*
 * Copyright (C) 2002-2003 the xine project
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
 * $Id: mms.h,v 1.11 2004/04/06 00:25:29 tmattern Exp $
 *
 * libmmst public header
 */

#ifndef HAVE_MMST_H
#define HAVE_MMST_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct mmst_s mmst_t;


extern char *
mmst_connect_common(int *s, int *port,
	 char *url, char **host, char **path, char **file);

extern mmst_t *
mmst_connect(int fd, const char *url_, int bandwidth);

extern int
mmst_play(mmst_t *session, uint32_t start, uint32_t length);

extern int
mmst_read(mmst_t *session, char *data, int len);

extern uint32_t
mmst_get_length(mmst_t *session);

extern void
mmst_close(mmst_t *session);

extern int
mmst_peek_header(mmst_t *session, char *data, int maxsize);

extern off_t
mmst_get_current_pos(mmst_t *session);



typedef mmst_t mmst_session_t;


extern mmst_session_t *
mmst_session_start(int fd, const char *mrl);

extern int
mmst_session_play(mmst_session_t *session, uint32_t start, uint32_t length);

extern int
mmst_session_read(mmst_session_t *session, char *data, int len);

extern int
mmst_session_seq(mmst_session_t *session, unsigned int *seq);
extern int
mmst_session_stamp(mmst_session_t *session, unsigned int *stamp);

extern void *
mmst_session_package(mmst_session_t *session);
extern int
mmst_session_packouts(mmst_session_t *session, uint32_t *num, uint32_t *size);

extern void *
mmst_session_header(mmst_session_t *session);

extern int
mmst_session_fix_header(mmst_session_t *session);
extern int
mmst_session_got_header(mmst_session_t *session, void **header);

extern int
mmst_session_peek_header(mmst_session_t *session, char *buf, int maxsize);

extern void
mmst_session_end(mmst_session_t *session);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef HAVE_MMS_H */


