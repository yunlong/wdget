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
 * $Id: rtsp_session.c,v 1.16 2004/04/24 16:55:42 miguelfreitas Exp $
 *
 * high level interface to rtsp servers.
 */

#include <sys/types.h>
#ifdef __GNUC__
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <unistd.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define LOG_MODULE "rtsp_session"
#define LOG_VERBOSE
/*
#define LOG
*/
#include "xsysio.h"

#include "rtsp.h"
#include "rtsp_asmrp.h"
#include "rtsp_session.h"

#include "real.h"
#include "rmff.h"


#define BUF_SIZE 4096
#define HEADER_SIZE 4096


struct rtsp_session_s {

  rtsp_t       *s;
  
  /* receive buffer */
  uint8_t       *recv;
  int           recv_size;
  int           recv_read;

  /* header buffer */
  uint8_t       header[HEADER_SIZE];
  int           header_len;
  int           header_read;
  
  uint32_t      pnum;
  uint32_t      plast;
  uint32_t      psize;
  uint32_t      pstart;
  uint32_t      pstend;
  
  uint32_t      stact;
  uint32_t      stamp;
  
  rmff_header_t *hobject;
  
  rmff_pheader_t package;
};


extern rtsp_session_t * rtsp_session_start(int fd, const char *mrl)
{
  rtsp_session_t *session = (rtsp_session_t *)malloc(sizeof(rtsp_session_t));
  char *server;
  char *mrl_line=strdup(mrl);
  rmff_header_t *h;
  uint32_t bandwidth=10485800;
  
  session->recv = (uint8_t *)xbuffer_init(BUF_SIZE);
  
  session->pnum = 0;
  session->psize = 0;
  session->plast = 0;
  session->pstart = UINT_MAX;
  session->pstend = UINT_MAX;
  
  session->stact = 0;
  session->stamp = 0;
  session->hobject = NULL;
  memset(&session->package, 0, sizeof(session->package));
  
connect:
  if (session->hobject != NULL)
  {
    rmff_free_header(session->hobject);
    session->hobject = NULL;
  }

  /* connect to server */
  session->s = rtsp_connect(fd, mrl_line, NULL);
  
  if (!session->s)
  {
    xprintf(XVERBOSITY_LOG,
	    _("rtsp_session: failed to connect to server %s\n"), mrl_line);
    xbuffer_free(session->recv);
    free(session);
    return NULL;
  }

  /* looking for server type */
  if (rtsp_search_answers(session->s,"Server"))
    server=strdup(rtsp_search_answers(session->s,"Server"));
  else {
    if (rtsp_search_answers(session->s,"RealChallenge1"))
      server=strdup("Real");
    else
      server=strdup("unknown");
  }

  if (strstr(server,"Real") || strstr(server,"Helix"))
  {
    /* we are talking to a real server ... */

    h=real_setup_and_get_header(session->s, bandwidth);
    if (!h)
    {
      /* got an redirect? */
      if (rtsp_search_answers(session->s, "Location"))
      {
        free(mrl_line);
    	mrl_line=strdup(rtsp_search_answers(session->s, "Location"));
        xprintf(XVERBOSITY_DEBUG, "rtsp_session: redirected to %s\n", mrl_line);
    	rtsp_close(session->s);
    	free(server);
    	goto connect; /* *shudder* i made a design mistake somewhere */
      }
      else
      {
        xprintf(XVERBOSITY_LOG,
		_("rtsp_session: session can not be established.\n"));
        rtsp_close(session->s);
        xbuffer_free(session->recv);
        free(session);
        return NULL;
      }
    }
    
    session->header_len = rmff_dump_header(h, (char *)session->header, 1024);
    if (session->header_len < 0)
      session->header_len = 0;
    
    xbuffer_copyin(session->recv, 0, session->header, session->header_len);
    session->recv_size = session->header_len;
    session->recv_read = 0;
    
    session->hobject = h;
  }
  else
  {
    xprintf(XVERBOSITY_LOG,
	    _("rtsp_session: rtsp server type '%s' not supported yet. sorry.\n"), server);
    rtsp_close(session->s);
    free(server);
    xbuffer_free(session->recv);
    free(session);
    return NULL;
  }
  free(server);
  
  return session;
}

extern int
rtsp_session_play(rtsp_session_t *session, uint32_t start, uint32_t length)
{
	int ret = 0;
	
	session->pstend = UINT_MAX;
	
	if ((start >= 0) && (start != UINT_MAX))
	{
		session->pstart = start;
		if ((int)length > 0)
		{
			session->pstend = start + length;
			length += 1000;
		}
	}
	
	ret = real_setup_act_play_range(session->s, start, length);
	if ((ret < 0) || (ret != RTSP_STATUS_OK))
	{
		if (ret < 0)
			return ret;
		return -ENOENT;
	}
	
	return RTSP_STATUS_OK;
}

extern 
int rtsp_session_read (rtsp_session_t *session, char *data, int len)
{
	int copy = len;
	char *dest = data;
	
	int fill = session->recv_size - session->recv_read;
	char *source = (char *)session->recv + session->recv_read;
	
	if (len < 0)
		return 0;
	
	while (copy > fill)
	{
		fill = session->recv_size - session->recv_read;
		if (fill > 0)
		{
			session->stact = session->stamp;
			
			if (session->plast > 0)
			{
				session->pnum++;
				session->psize += session->plast;
				session->plast = 0;
			}
			xutil_fast_memcpy(dest, source, fill);
			
			copy -= fill;
			dest += fill;
		}
		
		session->recv_read = 0;
		
		session->recv_size = rtsp_get_rdt_chunk (
			 session->s, 
			 (void *)&session->package, 
			 &session->recv
		);
		
		source = (char *)session->recv;
		fill = session->recv_size;
		
		if ((session->pstart != UINT_MAX) && 
			(session->package.timestamp < session->pstart))
		{
			session->recv_read = 0;
			session->recv_size = 0;
			continue;
		}
		
		if ((session->pstend != UINT_MAX) && 
				(session->package.timestamp >= session->pstend))
		{
			session->recv_size = 0;
		}
		
		if (session->recv_size == 0)
		{
			lprintf ("%d of %d bytes provided\n", len - copy, len);
			return len - copy;
		}
		
		session->plast = session->package.length;
		rtsp_get_rdt_stamp(session->s, &session->stamp);
		
//		if(JoinPoint == session->stamp)
//		{
//		printf("%s find JoinPoint stamp is %u %u \n", s, JoinPoint, session->stamp);
//		}
		
		if (session->hobject && (session->package.length > 0))
		{
			rmff_header_t *h = session->hobject;
			
			if ((session->package.stream_number < h->prop->num_streams) && 
					(h->streams && h->streams[session->package.stream_number]))
			{
				rmff_mdpr_t *s = h->streams[session->package.stream_number];
				
				s->max_total_num++;
				s->max_total_size += session->package.length;
				/**
				if ((session->package.timestamp + 1000) >= s->duration)
				{
					s->avg_bit_rate = (uint32_t)((double)s->max_total_size * 8 * 1000 / s->duration);
					s->avg_packet_size = s->max_total_size / s->max_total_num;
				}
				*/

			}
		}
	}
	
	session->stact = session->stamp;
	xutil_fast_memcpy(dest, source, copy);
	session->recv_read += copy;
	
	lprintf ("%d bytes provided\n", len);
	
	return len;
}

extern int rtsp_session_stamp(rtsp_session_t *session, unsigned int *stamp)
{
	if (session->stact <= 0)
		return -ENODATA;
	
	if (stamp != NULL)
		*stamp = session->stact;
	
	return (int)session->pnum;
}

extern void *
rtsp_session_package(rtsp_session_t *session)
{
	if (NULL == session)	/* ( || (session->package.length <= 0)) */
		return NULL;
	return (void *)&session->package;
}

extern int 
rtsp_session_packouts(rtsp_session_t *session, uint32_t *num, uint32_t *size)
{
	if (session->pnum <= 0)
		return -ENODATA;
	
	if (num != NULL)
		*num = session->pnum;
	if (size != NULL)
		*size = session->psize;
	return session->pnum;
}


extern void * rtsp_session_header(rtsp_session_t *session)
{
	if ((NULL == session) || (NULL == session->hobject))
		return NULL;
	return (void *)session->hobject;
}

extern int rtsp_session_fix_header(rtsp_session_t *session)
{
	rmff_header_t *h;

	uint32_t count = 0;
	uint32_t max_total_num = 0;
	uint32_t max_total_size = 0;
	
	if ((NULL == session) || (NULL == session->hobject))
		return -ENODATA;
	
	h = session->hobject;
	
	if (h->streams)
	{
		rmff_mdpr_t **streams = h->streams;
		
		while (*streams)
		{
			rmff_mdpr_t *s = *streams;
		/**
     		s->avg_bit_rate = (uint32_t)((double)s->max_total_size * 8 * 1000 / s->duration);
			s->avg_packet_size = s->max_total_size / s->max_total_num;
 		*/	
			max_total_num += s->max_total_num;
			max_total_size += s->max_total_size;
			
			count++;
			streams++;
		}
	}
	if ((h->prop && h->streams) && (h->prop->duration > 0) && 
		(max_total_num > 0) && (max_total_size > 0))
	{
	/**
		h->prop->avg_bit_rate = (uint32_t)((double)max_total_size * 8 * 1000 / h->prop->duration);
  
		h->prop->num_packets = max_total_num;
		h->prop->avg_packet_size = max_total_size / max_total_num;
 	*/
	}
	if (h->data != NULL)
	{
		h->data->num_packets = session->pnum;
		h->data->size = sizeof(rmff_data_t) + session->psize;
		
		if (h->prop != NULL)
		{
			h->prop->index_offset = h->prop->data_offset + h->data->size;
		}
	}
	
	return 0;
}

extern int rtsp_session_got_header(rtsp_session_t *session, void **header)
{
	if ((NULL == session) || (NULL == session->hobject))
		return -ENODATA;
	if (header != NULL)
	{
		*header = (void *)session->hobject;
		session->hobject = NULL;
	}
	return 0;
}

extern int rtsp_session_peek_header(rtsp_session_t *session, char *buf, int maxsize)
{
  int len;

  len = (session->header_len < maxsize) ? session->header_len : maxsize;

  memcpy(buf, session->header, len);
  return len;
}

extern void rtsp_session_end(rtsp_session_t *session)
{
  if (session->s)
    rtsp_close(session->s);
  if (session->recv)
    xbuffer_free(session->recv);
  if (session->hobject)
    rmff_free_header(session->hobject);
  free(session);
}


