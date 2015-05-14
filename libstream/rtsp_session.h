/*
 * Copyright (C) 2000-2003 the xine project
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
 * $Id: rtsp_session.h,v 1.6 2003/12/09 00:02:31 f1rmb Exp $
 *
 * high level interface to rtsp servers.
 */

#ifndef RTSP_SESSION_H
#define RTSP_SESSION_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtsp_session_s rtsp_session_t;


extern rtsp_session_t *
rtsp_session_start(int fd, const char *mrl);

extern int
rtsp_session_play(rtsp_session_t *session, uint32_t start, uint32_t length);

extern int
rtsp_session_read(rtsp_session_t *session, char *data, int len);

extern int
rtsp_session_stamp(rtsp_session_t *session, unsigned int *stamp);

extern void *
rtsp_session_package(rtsp_session_t *session);
extern int
rtsp_session_packouts(rtsp_session_t *session, uint32_t *num, uint32_t *size);

extern void *
rtsp_session_header(rtsp_session_t *session);

extern int
rtsp_session_fix_header(rtsp_session_t *session);
extern int
rtsp_session_got_header(rtsp_session_t *session, void **header);

extern int
rtsp_session_peek_header(rtsp_session_t *session, char *buf, int maxsize);

extern void
rtsp_session_end(rtsp_session_t *session);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef RTSP_SESSION_H */


