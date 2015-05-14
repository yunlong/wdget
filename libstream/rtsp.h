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
 * $Id: rtsp.h,v 1.4 2003/12/09 00:02:31 f1rmb Exp $
 *
 * a minimalistic implementation of rtsp protocol,
 * *not* RFC 2326 compilant yet.
 */

#ifndef HAVE_RTSP_H
#define HAVE_RTSP_H


#ifdef __cplusplus
extern "C" {
#endif


#define RTSP_DATA_NULL         0x0000
#define RTSP_DATA_KEYFRAME     0x0002


/* some codes returned by rtsp_request_* functions */

#define RTSP_STATUS_SET_PARAMETER  10
#define RTSP_STATUS_OK            200

#define RTSP_STATUS_FAILED(v)     ((v < 200) || (v > 299))


typedef struct rtsp_s rtsp_t;


extern rtsp_t *
rtsp_connect(int fd, const char *mrl, const char *user_agent);

extern int
rtsp_request_options(rtsp_t *s, const char *what);

extern int
rtsp_request_describe(rtsp_t *s, const char *what);

extern int
rtsp_request_setup(rtsp_t *s, const char *what);

extern int
rtsp_request_setparameter(rtsp_t *s, const char *what);

extern int
rtsp_request_play(rtsp_t *s, const char *what);

extern int
rtsp_request_tearoff(rtsp_t *s, const char *what);

extern int
rtsp_send_ok(rtsp_t *s);

extern int
rtsp_read_data(rtsp_t *s, char *buffer, unsigned int size);

extern char *
rtsp_search_answers(rtsp_t *s, const char *tag);

extern void
rtsp_add_to_payload(char **payload, const char *string);

extern void
rtsp_free_answers(rtsp_t *s);

extern int
rtsp_read(rtsp_t *s, char *data, int len);

extern void
rtsp_close (rtsp_t *s);

extern void
rtsp_set_session(rtsp_t *s, const char *id);

extern char *
rtsp_get_session(rtsp_t *s);

extern char *
rtsp_get_mrl(rtsp_t *s);

/**
 * 
extern int
rtsp_peek_header (rtsp_t *s, char *data); */

extern void
rtsp_schedule_field(rtsp_t *s, const char *string);

extern void
rtsp_unschedule_all(rtsp_t *s);
extern void
rtsp_unschedule_field(rtsp_t *s, const char *string);


extern int
rtsp_get_rdt_stamp(rtsp_t *session, unsigned int *stamp);
extern int
rtsp_get_rdt_chunk(rtsp_t *session, void *packet, unsigned char **buffer);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef HAVE_RTSP_H */


