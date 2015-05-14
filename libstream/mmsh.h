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
 * $Id: mmsh.h,v 1.4 2004/04/06 00:25:29 tmattern Exp $
 *
 * libmmsh public header
 */

#ifndef HAVE_MMSH_H
#define HAVE_MMSH_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct mmsh_s mmsh_t;

extern char *
mmsh_connect_common(int *s ,int *port,
	 char *url, char **host, char **path, char **file);

extern mmsh_t *
mmsh_connect(int fd, const char *url_, int bandwidth);

extern int
mmsh_play(mmsh_t *session, uint32_t start, uint32_t length);

extern int
mmsh_read(mmsh_t *session, char *data, int len);

extern uint32_t
mmsh_get_length(mmsh_t *session);

extern void
mmsh_close(mmsh_t *session);

extern int
mmsh_peek_header(mmsh_t *session, char *data, int maxsize);

extern off_t
mmsh_get_current_pos(mmsh_t *session);



typedef mmsh_t mmsh_session_t;


extern mmsh_session_t *
mmsh_session_start(int fd, const char *mrl);

extern int
mmsh_session_play(mmsh_session_t *session, uint32_t start, uint32_t length);

extern int
mmsh_session_read(mmsh_session_t *session, char *data, int len);

extern int
mmsh_session_seq(mmsh_session_t *session, unsigned int *seq);
extern int
mmsh_session_stamp(mmsh_session_t *session, unsigned int *stamp);

extern void *
mmsh_session_package(mmsh_session_t *session);
extern int
mmsh_session_packouts(mmsh_session_t *session, uint32_t *num, uint32_t *size);

extern void *
mmsh_session_header(mmsh_session_t *session);

extern int
mmsh_session_fix_header(mmsh_session_t *session);
extern int
mmsh_session_got_header(mmsh_session_t *session, void **header);

extern int
mmsh_session_peek_header(mmsh_session_t *session, char *buf, int maxsize);

extern void
mmsh_session_end(mmsh_session_t *session);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef HAVE_MMSH_H */


