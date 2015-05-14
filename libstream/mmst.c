/*
 * Copyright (C) 2002-2004 the xine project
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
 * $Id: mms.c,v 1.52.2.3 2005/04/21 19:45:45 mroi Exp $
 *
 * MMS over TCP protocol
 *   based on work from major mms
 *   utility functions to handle communication with an mms server
 *
 * TODO:
 *   error messages
 *   enable seeking !
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __GNUC__
# include <unistd.h>
#endif
#ifdef WIN32
#  include <winsock.h>
#else
#  include <sys/time.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(HAVE_ICONV) && defined(HAVE_LANGINFO_CODESET)
#define USE_ICONV
#include <iconv.h>
#include <locale.h>
#include <langinfo.h>
#endif

/********** logging **********/
#define LOG_MODULE      "mmst"
#define LOG_VERBOSE
/*
#define LOG 
*/
#include "xsysio.h"
#include "mmst.h"

#include "mms_asf.h"


/* 
 * mms specific types 
 */

#define MMST_PORT 1755

#define BUF_SIZE 102400

#define CMD_HEADER_LEN   40
#define CMD_PREFIX_LEN    8
#define CMD_BODY_LEN   1024

#define ASF_HEADER_LEN 8192


#define MMS_PACKET_ERR        0
#define MMS_PACKET_COMMAND    1
#define MMS_PACKET_ASF_HEADER 2
#define MMS_PACKET_ASF_PACKET 3

#define ASF_HEADER_PACKET_ID_TYPE 2
#define ASF_MEDIA_PACKET_ID_TYPE  4


struct mmst_buffer_s
{
  uint8_t *buffer;
  int pos;
};

struct mmst_packet_header_s
{
  uint32_t  packet_len;
  uint8_t   flags;
  uint8_t   packet_id_type;
  uint32_t  packet_seq;
};

typedef struct mmst_buffer_s mmst_buffer_t;
typedef struct mmst_packet_header_s mmst_packet_header_t;


struct mmst_s {

  int           s;
  int           refers;
  
  /* url parsing */
  char         *url;
  char         *proto;
  char         *host;
  int           port;
  char         *user;
  char         *password;
  char         *uri;

  /* command to send */
  char          scmd[CMD_HEADER_LEN + CMD_BODY_LEN];
  char         *scmd_body; /* pointer to &scmd[CMD_HEADER_LEN] */
  int           scmd_len; /* num bytes written in header */
  
  /* receive buffer */
  uint8_t       buf[BUF_SIZE];
  uint8_t       buf_data[BUF_SIZE];
  uint8_t       buf_head[32];
  int           buf_read;
  int           buf_size;
  
  uint8_t       asf_header[ASF_HEADER_LEN];
  uint32_t      asf_header_len;
  uint32_t      asf_header_read;
  int           seq_num;
  int           num_stream_ids;
  int           stream_ids[ASF_MAX_NUM_STREAMS];
  int           stream_types[ASF_MAX_NUM_STREAMS];
  int           asf_packet_len;
  uint64_t      file_len;
  char          guid[37];
  uint32_t      bitrates[ASF_MAX_NUM_STREAMS];
  uint32_t      bitrates_pos[ASF_MAX_NUM_STREAMS];
  int           bandwidth;
  
  int           has_audio;
  int           has_video;
  int           live_flag;
  off_t         current_pos;
  int           eos;

  uint32_t      pnum;
  uint32_t      plast;
  uint32_t      psize;
  uint32_t      pstart;
  
  uint32_t      stact;
  uint32_t      stamp;
  
  mmsf_header_t *hobject;
  mmst_packet_header_t package;
};


static void
mmst_buffer_init (mmst_buffer_t *mmst_buffer, uint8_t *buffer)
{
  mmst_buffer->buffer = buffer;
  mmst_buffer->pos = 0;
}

static void
mmst_buffer_put_8 (mmst_buffer_t *mmst_buffer, uint8_t value)
{
  mmst_buffer->buffer[mmst_buffer->pos] = value & 0xff;

  mmst_buffer->pos += 1;
}

#if 0
static void
mmst_buffer_put_16 (mmst_buffer_t *mmst_buffer, uint16_t value)
{
  mmst_buffer->buffer[mmst_buffer->pos]     = value          & 0xff;
  mmst_buffer->buffer[mmst_buffer->pos + 1] = (value  >> 8)  & 0xff;

  mmst_buffer->pos += 2;
}
#endif

static void
mmst_buffer_put_32 (mmst_buffer_t *mmst_buffer, uint32_t value)
{
  mmst_buffer->buffer[mmst_buffer->pos]     = value          & 0xff;
  mmst_buffer->buffer[mmst_buffer->pos + 1] = (value  >> 8)  & 0xff;
  mmst_buffer->buffer[mmst_buffer->pos + 2] = (value  >> 16) & 0xff;
  mmst_buffer->buffer[mmst_buffer->pos + 3] = (value  >> 24) & 0xff;

  mmst_buffer->pos += 4;
}


static void print_command (char *data, int len)
{
#ifdef LOG
  int i;
  int dir = LE_32 (data + 36) >> 16;
  int comm = LE_32 (data + 36) & 0xFFFF;

  printf ("----------------------------------------------\n");
  if (dir == 3) {
    printf ("send command 0x%02x, %d bytes\n", comm, len);
  } else {
    printf ("receive command 0x%02x, %d bytes\n", comm, len);
  }
  printf ("  start sequence %08x\n", LE_32 (data +  0));
  printf ("  command id     %08x\n", LE_32 (data +  4));
  printf ("  length         %8x \n", LE_32 (data +  8));
  printf ("  protocol       %08x\n", LE_32 (data + 12));
  printf ("  len8           %8x \n", LE_32 (data + 16));
  printf ("  sequence #     %08x\n", LE_32 (data + 20));
  printf ("  len8  (II)     %8x \n", LE_32 (data + 32));
  printf ("  dir | comm     %08x\n", LE_32 (data + 36));
  if (len >= 4)
    printf ("  prefix1        %08x\n", LE_32 (data + 40));
  if (len >= 8)
    printf ("  prefix2        %08x\n", LE_32 (data + 44));

  for (i = (CMD_HEADER_LEN + CMD_PREFIX_LEN); i < (CMD_HEADER_LEN + CMD_PREFIX_LEN + len); i += 1) {
    unsigned char c = data[i];
    
    if ((c >= 32) && (c < 128))
      printf ("%c", c);
    else
      printf (" %02x ", c);
    
  }
  if (len > CMD_HEADER_LEN)
    printf ("\n");
  printf ("----------------------------------------------\n");
#endif
}  

static int
send_command (mmst_t *session, int command,
	 uint32_t prefix1, uint32_t prefix2, int length)
{
  int    len8;
  off_t  n;
  mmst_buffer_t command_buffer;

  len8 = (length + 7) / 8;

  session->scmd_len = 0;

  mmst_buffer_init(&command_buffer, (uint8_t *)session->scmd);
  mmst_buffer_put_32 (&command_buffer, 0x00000001);   /* start sequence */
  mmst_buffer_put_32 (&command_buffer, 0xB00BFACE);   /* #-)) */
  mmst_buffer_put_32 (&command_buffer, len8 * 8 + 32);
  mmst_buffer_put_32 (&command_buffer, 0x20534d4d);   /* protocol type "MMS " */
  mmst_buffer_put_32 (&command_buffer, len8 + 4);
  mmst_buffer_put_32 (&command_buffer, session->seq_num);
  session->seq_num++;
  mmst_buffer_put_32 (&command_buffer, 0x0);          /* timestamp */
  mmst_buffer_put_32 (&command_buffer, 0x0);
  mmst_buffer_put_32 (&command_buffer, len8 + 2);
  mmst_buffer_put_32 (&command_buffer, 0x00030000 | command); /* dir | command */
  /* end of the 40 byte command header */
  
  mmst_buffer_put_32 (&command_buffer, prefix1);
  mmst_buffer_put_32 (&command_buffer, prefix2);

  if (length & 7)
    memset(session->scmd + length + CMD_HEADER_LEN + CMD_PREFIX_LEN, 0, 8 - (length & 7));

  n = xio_tcp_write (session->s, session->scmd, len8 * 8 + CMD_HEADER_LEN + CMD_PREFIX_LEN);
  if (n != (len8 * 8 + CMD_HEADER_LEN + CMD_PREFIX_LEN)) {
    return 0;
  }

  print_command (session->scmd, length);

  return 1;
}

#ifdef USE_ICONV
static iconv_t
string_utf16_open()
{
	return iconv_open("UTF-16LE", nl_langinfo(CODESET));
}

static void
string_utf16_close(iconv_t url_conv)
{
	if (url_conv != (iconv_t)-1)
	{
		iconv_close(url_conv);
		url_conv = (iconv_t)-1;
	}
}

static void
string_utf16(iconv_t url_conv, char *dest, char *src, int len)
{
	memset(dest, 0, 1000);
	
	if (url_conv == (iconv_t)-1)
	{
		int i;
		
		for (i = 0; i < len; i++)
		{
			dest[i * 2] = src[i];
			dest[i * 2 + 1] = 0;
		}
		dest[i * 2] = 0;
		dest[i * 2 + 1] = 0;
	}
	else
	{
		char *ip, *op;
		size_t len1, len2;
		
		ip = src; op = dest;
		len1 = len; len2 = 1000;
		iconv(url_conv, &ip, &len1, &op, &len2);
	}
}

#else
static void
string_utf16(int unused, char *dest, char *src, int len)
{
	int i;
	
	memset (dest, 0, 1000);
	
	for (i = 0; i < len; i++)
	{
		dest[i * 2] = src[i];
		dest[i * 2 + 1] = 0;
	}
	dest[i * 2] = 0;
	dest[i * 2 + 1] = 0;
}
#endif


/*
 * return packet type
 */
static int
get_packet_header(mmst_t *session, mmst_packet_header_t *header)
{
	size_t len;
	int packet_type;
	
	header->packet_len     = 0;
	header->packet_seq     = 0;
	header->flags          = 0;
	header->packet_id_type = 0;
	len = xio_tcp_read (session->s, session->buf, 8);
	if (len != 8)
		goto error;
	memcpy(session->buf_head, session->buf, 8);
	memset(session->buf_head + 8, 0, sizeof(session->buf_head) - 8);
	
	if (LE_32(session->buf + 4) == 0xb00bface)
	{
		/* command packet */
		header->flags = session->buf[3];
		len = xio_tcp_read (session->s, session->buf + 8, 4);
		if (len != 4)
			goto error;
		memcpy(session->buf_head + 8, session->buf + 8, 4);
		
		header->packet_len = LE_32(session->buf + 8) + 4;
		lprintf("mms command\n");
		packet_type = MMS_PACKET_COMMAND;
	}
	else
	{
		header->packet_seq     = LE_32(session->buf);
		header->packet_id_type = session->buf[4];
		header->flags          = session->buf[5];
		header->packet_len     = LE_16(session->buf + 6) - 8;
		if (header->packet_id_type == ASF_HEADER_PACKET_ID_TYPE)
		{
			lprintf("asf header\n");
			packet_type = MMS_PACKET_ASF_HEADER;
		}
		else
		{
			lprintf("asf packet\n");
			packet_type = MMS_PACKET_ASF_PACKET;
		}
	}
	return packet_type;
	
error:
	lprintf("read error, len=%d\n", len);
	return MMS_PACKET_ERR;
}


static int
get_packet_command (mmst_t *session, uint32_t packet_len)
{
	int  command = 0;
	size_t len;
	
	/* always enter session loop */
	lprintf("packet_len: %d bytes\n", packet_len);
	
	len = xio_tcp_read(session->s, session->buf + 12, packet_len) ;
	if (len != packet_len)
	{
		return 0;
	}
	print_command ((char *)session->buf, len);
	
	/* check protocol type ("MMS ") */
	if (LE_32(session->buf + 12) != 0x20534D4D)
	{
		lprintf("unknown protocol type: %c%c%c%c (0x%08X)\n",
				session->buf[12], session->buf[13], session->buf[14], session->buf[15],
				LE_32(session->buf + 12));  
		return 0;
	}
	
	command = LE_32 (session->buf + 36) & 0xFFFF;
	lprintf("command = 0x%2x\n", command);
	
	return command;
}

static int
get_answer(mmst_t *session)
{
	int command = 0;
	mmst_packet_header_t header;
	
	switch (get_packet_header (session, &header))
	{
	case MMS_PACKET_ERR:
		xprintf(XVERBOSITY_LOG, "libmmst: failed to read mms packet header\n");
		break;
	case MMS_PACKET_COMMAND:
		command = get_packet_command (session, header.packet_len);
		if (command == 0x1b)
		{
			if (!send_command (session, 0x1b, 0, 0, 0))
			{
			  xprintf(XVERBOSITY_LOG, "libmmst: failed to send command\n");
			  return 0;
			}
			/* FIXME: limit recursion */
			command = get_answer (session);
		}
		break;
	case MMS_PACKET_ASF_HEADER:
		xprintf(XVERBOSITY_LOG,
			  "libmmst: unexpected asf header packet\n");
		break;
	case MMS_PACKET_ASF_PACKET:
		xprintf(XVERBOSITY_LOG,
			  "libmmst: unexpected asf packet\n");
		break;
	}
	
	return command;
}

static int
get_asf_header (mmst_t *session)
{
	off_t len;
	int stop = 0;
	
	session->asf_header_read = 0;
	session->asf_header_len = 0;
	
	while (!stop)
	{
		int command;
		mmst_packet_header_t header;
		
		switch (get_packet_header (session, &header))
		{
		case MMS_PACKET_ERR:
			xprintf(XVERBOSITY_LOG,
					"libmmst: failed to read mms packet header\n");
			return 0;
			break;
		case MMS_PACKET_COMMAND:
			command = get_packet_command (session, header.packet_len);
			if (command == 0x1b)
			{
				if (!send_command (session, 0x1b, 0, 0, 0))
				{
					xprintf(XVERBOSITY_LOG,
							"libmmst: failed to send command\n");
					return 0;
				}
				command = get_answer (session);
			}
			else
			{
				xprintf(XVERBOSITY_LOG,
					  "libmmst: unexpected command packet\n");
			}
			break;
		case MMS_PACKET_ASF_HEADER:
		case MMS_PACKET_ASF_PACKET:
			len = xio_tcp_read (session->s,
				 session->asf_header + session->asf_header_len, header.packet_len);
			if (len != (off_t)header.packet_len)
			{
				xprintf(XVERBOSITY_LOG,
					  "libmmst: get_header failed\n");
				return 0;
			}
			session->asf_header_len += header.packet_len;
			lprintf("header flags: %d\n", header.flags);
			
			if ((header.flags == 0X08) || (header.flags == 0X0C))
				stop = 1;
			break;
		}
	}
	lprintf ("get header packet succ\n");
	return 1;
}

static int
interp_asf_header(mmst_t *session)
{
	int ret = -EINVAL;
	int i;
	
	session->asf_packet_len = 0;
	session->num_stream_ids = 0;
	
	if (session->hobject != NULL)
	{
		mms_free_header(session->hobject);
		session->hobject = NULL;
	}
	ret = mms_got_header(
		 session->asf_header, session->asf_header_len, &session->hobject);
	if (ret < 0)
		return ret;
	
	/*
	 * parse header
	 */
	i = 30;
	while (i < (int)session->asf_header_len)
	{
		int guid;
		uint64_t length;
		
		guid = mms_get_guid(session->asf_header, i);
		i += 16;
		length = LE_64(session->asf_header + i);
		i += 8;
		
		switch (guid)
		{
		case GUID_ASF_FILE_PROPERTIES:
			session->asf_packet_len = LE_32(session->asf_header + i + 92 - 24);
			session->file_len       = LE_64(session->asf_header + i + 40 - 24);
			lprintf("file object, file_length = %lld, packet length = %d",
					session->file_len, session->asf_packet_len);
			break;
			
		case GUID_ASF_STREAM_PROPERTIES:
			{
			  uint16_t flags;
			  uint16_t stream_id;
			  int      type;
			  int      encrypted;
			
			  guid = mms_get_guid(session->asf_header, i);
			  switch (guid) {
				case GUID_ASF_AUDIO_MEDIA:
				  type = ASF_STREAM_TYPE_AUDIO;
				  session->has_audio = 1;
				  break;
			
				case GUID_ASF_VIDEO_MEDIA:
				case GUID_ASF_JFIF_MEDIA:
				case GUID_ASF_DEGRADABLE_JPEG_MEDIA:
				  type = ASF_STREAM_TYPE_VIDEO;
				  session->has_video = 1;
				  break;
			  
				case GUID_ASF_COMMAND_MEDIA:
				  type = ASF_STREAM_TYPE_CONTROL;
				  break;
			
				default:
				  type = ASF_STREAM_TYPE_UNKNOWN;
			  }
			
			  flags = LE_16(session->asf_header + i + 48);
			  stream_id = flags & 0x7F;
			  encrypted = flags >> 15;
			
			  lprintf ("stream object, stream id: %d, type: %d, encrypted: %d\n",
					   stream_id, type, encrypted);
			  
			  if (session->num_stream_ids < ASF_MAX_NUM_STREAMS && stream_id < ASF_MAX_NUM_STREAMS) {
				session->stream_types[stream_id] = type;
				session->stream_ids[session->num_stream_ids] = stream_id;
				session->num_stream_ids++;
			  } else {
				lprintf ("too many streams, skipping\n");
			  }
			
			}
			break;
		
		case GUID_ASF_STREAM_BITRATE_PROPERTIES:
			{
			  uint16_t streams = LE_16(session->asf_header + i);
			  uint16_t stream_id;
			  int j;
			
			  lprintf ("stream bitrate properties\n");
			  lprintf ("streams %d\n", streams); 
			
			  for(j = 0; j < streams; j++) {
				stream_id = LE_16(session->asf_header + i + 2 + j * 6);
				lprintf ("stream id %d\n", stream_id); 
				session->bitrates[stream_id] = LE_32(session->asf_header + i + 4 + j * 6);
				session->bitrates_pos[stream_id] = i + 4 + j * 6;
				lprintf ("stream id %d, bitrate %d\n", stream_id, 
						 session->bitrates[stream_id]);
			  }
			}
			break;
		
		default:
			lprintf ("unknown object\n");
			break;
		}
		
		lprintf ("length    : %lld\n", length);
		
		if (length > 24)
		{
			i += (int)(length - 24);
		}
	}
	
	return session->asf_packet_len;
}

const static char *const mmst_proto_s[] = { "mms", "mmst", NULL };

static int
mmst_valid_proto (char *proto)
{
	int i = 0;
	
	lprintf("mmst_valid_proto\n");
	
	if (!proto)
		return 0;
	
	while(mmst_proto_s[i])
	{
		if (!strcasecmp(proto, mmst_proto_s[i]))
		{
			return 1;
		}
		i++;
	}
	return 0;
}

static void
mmst_report_progress(mmst_t *session, int p)
{
/**
  xine_event_t             event;
  xine_progress_data_t     prg;

  prg.description = _("Connecting MMS server (over tcp)...");
  prg.percent = p;
  
  event.type = XINE_EVENT_PROGRESS;
  event.data = &prg;
  event.data_length = sizeof (xine_progress_data_t);
  
  xine_event_send (stream, &event);
 */
}


/**
 * returns 1 on error
 */
static int
mmst_tcp_connect(mmst_t *session)
{
  int progress, res;
  
  if (!session->port) session->port = MMST_PORT;
  
  /* 
   * try to connect 
   */
  lprintf("try to connect to %s on port %d \n", session->host, session->port);
  session->s = xio_tcp_connect (session->host, session->port);
  if (session->s == -1) {
    xprintf(XVERBOSITY_LOG, "failed to connect '%s'\n", session->host);
    return 1;
  }

  /* connection timeout 15s */
  progress = 0;
  do {
    mmst_report_progress(session, progress);
    res = xio_select (session->s, XIO_WRITE_READY, 500);
    progress += 1;
  } while ((res == XIO_TIMEOUT) && (progress < 30));
  if (res != XIO_READY) {
    return 1;
  }
  lprintf ("connected\n");
  return 0;
}

static void mmst_gen_guid(char guid[])
{
  int i = 0;
  static char digit[] = "0123456789ABCDEF";

  srand(time(NULL));
  for (i = 0; i < 36; i++) {
    guid[i] = digit[(int) ((16.0*rand())/(RAND_MAX+1.0))];
  }
  guid[8] = '-'; guid[13] = '-'; guid[18] = '-'; guid[23] = '-';
  guid[36] = '\0';
}

/*
 * return 0 on error
 */
static int
mmst_choose_best_streams(mmst_t *session)
{
  int     i;
  int     res;
  int     audio_stream = 0;
  int     video_stream = 0;
  int     max_arate    = 0;
  int     min_vrate    = 0;
  int     min_bw_left  = 0;
  int     stream_id;
  int     bandwitdh_left;
  
  /* command 0x33 */
  /* choose the best quality for the audio stream */
  /* i've never seen more than one audio stream */
  lprintf("num_stream_ids=%d\n", session->num_stream_ids);
  
  for (i = 0; i < session->num_stream_ids; i++)
  {
    stream_id = session->stream_ids[i];
    switch (session->stream_types[stream_id]) {
      case ASF_STREAM_TYPE_AUDIO:
        if ((int)session->bitrates[stream_id] > max_arate) {
          audio_stream = stream_id;
          max_arate = session->bitrates[stream_id];
        }
        break;
      default:
        break;
    }
  }
  
  /* choose a video stream adapted to the user bandwidth */
  bandwitdh_left = session->bandwidth - max_arate;
  if (bandwitdh_left < 0) {
    bandwitdh_left = 0;
  }
  lprintf("bandwitdh %d, left %d\n", session->bandwidth, bandwitdh_left);
  
  min_bw_left = bandwitdh_left;
  for (i = 0; i < session->num_stream_ids; i++) {
    stream_id = session->stream_ids[i];
    switch (session->stream_types[stream_id]) {
      case ASF_STREAM_TYPE_VIDEO:
        if (((bandwitdh_left - (int)session->bitrates[stream_id]) < min_bw_left) &&
            (bandwitdh_left >= (int)session->bitrates[stream_id])) {
          video_stream = stream_id;
          min_bw_left = bandwitdh_left - session->bitrates[stream_id];
        }
        break;
      default:
        break;
    }
  }
  
  /* choose the lower bitrate of */
  if (!video_stream && session->has_video) {
    for (i = 0; i < session->num_stream_ids; i++) {
      stream_id = session->stream_ids[i];
      switch (session->stream_types[stream_id]) {
        case ASF_STREAM_TYPE_VIDEO:
          if (((int)session->bitrates[stream_id] < min_vrate) ||
              (!min_vrate)) {
            video_stream = stream_id;
            min_vrate = (int)session->bitrates[stream_id];
          }
          break;
        default:
          break;
      }
    }
  }
  
  lprintf("selected streams: audio %d, video %d\n", audio_stream, video_stream);
  lprintf("disabling other streams\n");
  memset (session->scmd_body, 0, 40);
  
  for (i = 1; i < session->num_stream_ids; i++) {
    session->scmd_body [ (i - 1) * 6 + 2 ] = 0xFF;
    session->scmd_body [ (i - 1) * 6 + 3 ] = 0xFF;
    session->scmd_body [ (i - 1) * 6 + 4 ] = session->stream_ids[i] ;
    session->scmd_body [ (i - 1) * 6 + 5 ] = session->stream_ids[i] >> 8;
    if ((session->stream_ids[i] == audio_stream) ||
        (session->stream_ids[i] == video_stream)) {
      session->scmd_body [ (i - 1) * 6 + 6 ] = 0x00;
      session->scmd_body [ (i - 1) * 6 + 7 ] = 0x00;
    } else {
      lprintf("disabling stream %d\n", session->stream_ids[i]);
      session->scmd_body [ (i - 1) * 6 + 6 ] = 0x02;
      session->scmd_body [ (i - 1) * 6 + 7 ] = 0x00;
      
      /* forces the asf demuxer to not choose session stream */
      if (session->bitrates_pos[session->stream_ids[i]]) {
        session->asf_header[session->bitrates_pos[session->stream_ids[i]]]     = 0;
        session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 1] = 0;
        session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 2] = 0;
        session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 3] = 0;
      }
    }
  }
  
  if (!send_command (session, 0x33, session->num_stream_ids, 
                     0xFFFF | session->stream_ids[0] << 16, 
                     session->num_stream_ids * 6 + 2)) {
    xprintf(XVERBOSITY_LOG,
            "libmmst: mmst_choose_best_streams failed\n");
    return 0;
  }
  
  if ((res = get_answer (session)) != 0x21) {
    xprintf(XVERBOSITY_LOG,
            "libmmst: unexpected response: %02x (0x21)\n", res);
  }
  
  return 1;
}

/**
 * TODO: error messages <BR>
 *       network timing request
 */
extern mmst_t *
mmst_connect(int fd, const char *url, int bandwidth)
{
#ifdef USE_ICONV
  iconv_t url_conv;
#else
  int     url_conv = 0;
#endif
  mmst_t  *session;
  char    str[1024];
  int     res;
 
  if (!url)
    return NULL;

  session = (mmst_t*) xutil_malloc (sizeof (mmst_t));

//  session->stream          = stream;
  session->url             = strdup (url);
  session->s               = -1;
  
  session->refers          = 0;
  session->seq_num         = 0;
  session->scmd_body       = session->scmd + CMD_HEADER_LEN + CMD_PREFIX_LEN;
  session->asf_header_len  = 0;
  session->asf_header_read = 0;
  session->num_stream_ids  = 0;
  session->asf_packet_len  = 0;
  session->buf_read        = 0;
  session->buf_size        = 0;
  session->has_audio       = 0;
  session->has_video       = 0;
  
  if (bandwidth <= 0)
	bandwidth = 10485800;
  session->bandwidth       = bandwidth;
  session->current_pos     = 0;
  session->eos             = 0;
  
  session->pnum = 0;
  session->plast = 0;
  session->psize = 0;
  session->pstart = UINT_MAX;
  
  session->stact = 0;
  session->stamp = 0;
  session->hobject = NULL;
  memset(&session->package, 0, sizeof(session->package));
  
  if (!xutil_parse_url (session->url, &session->proto, &session->host, &session->port,
                  &session->user, &session->password, &session->uri)) {
    lprintf ("invalid url\n");
    goto fail;
  }
  mmst_report_progress(session, 0);
  
  if (!mmst_valid_proto(session->proto)) {
    lprintf ("unsupported protocol\n");
    goto fail;
  }
 
  if (fd >= 0)
  {
    session->s = fd;
    fd = -1;
    session->refers = 1;
//	if (mmst_tcp_connect(session))
// 	goto fail;
  }
  else
  {
    session->refers = 0;
    if (mmst_tcp_connect(session))
      goto fail;
  }
  
  //
 
  //
  
  mmst_report_progress(session, 30);
  
#ifdef USE_ICONV
  url_conv = string_utf16_open();
#endif
  /*
   * let the negotiations begin...
   */

  /* command 0x1 */
  lprintf("send command 0x01\n");
  mmst_gen_guid(session->guid);
  snprintf (str, sizeof(str), "\x1c\x03NSPlayer/7.0.0.1956; {%s}; Host: %s",
    session->guid, session->host);
  string_utf16 (url_conv, session->scmd_body, str, strlen(str) + 2);

  if (!send_command (session, 1, 0, 0x0004000b, strlen(str) * 2 + 8)) {
    xprintf(XVERBOSITY_LOG,
            "libmmst: failed to send command 0x01\n");
    goto fail;
  }
  
  if ((res = get_answer (session)) != 0x01) {
    xprintf(XVERBOSITY_LOG, "libmmst: unexpected response: %02x (0x01)\n", res);
    goto fail;
  }
  
  mmst_report_progress(session, 40);
  
  /* TODO: insert network timing request here */
  /* command 0x2 */
  lprintf("send command 0x02\n");
  string_utf16 (url_conv, &session->scmd_body[8], "\002\000\\\\192.168.0.100\\TCP\\1037\0000", 28);
  memset (session->scmd_body, 0, 8);
  if (!send_command (session, 2, 0, 0, 28 * 2 + 8)) {
    xprintf(XVERBOSITY_LOG, "libmmst: failed to send command 0x02\n");
    goto fail;
  }
  
  switch (res = get_answer (session)) {
    case 0x02:
      /* protocol accepted */
      break;
    case 0x03:
      xprintf(XVERBOSITY_LOG, "libmmst: protocol failed\n");
      goto fail;
      break;
    default:
      lprintf("unexpected response: %02x (0x02 or 0x03)\n", res);
      goto fail;
  }

  mmst_report_progress(session, 50);

  /* command 0x5 */
  {
    mmst_buffer_t command_buffer;
    char *path;
    int pathlen;

    /* remove the first '/' */
    path = session->uri;
    pathlen = strlen(path);
    if (pathlen > 1) {
      path++;
      pathlen--;
    }
    
    lprintf("send command 0x05\n");
    mmst_buffer_init(&command_buffer, (uint8_t *)session->scmd_body);
    mmst_buffer_put_32 (&command_buffer, 0x00000000); /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x00000000); /* ?? */
    string_utf16 (url_conv, session->scmd_body + command_buffer.pos, path, pathlen);
    if (!send_command (session, 5, 1, 0xffffffff, pathlen * 2 + 12))
      goto fail;
  }
  
  switch (res = get_answer (session)) {
    case 0x06:
      {
        int xx, yy;
        /* no authentication required */
      
        /* Warning: sdp is not right here */
        xx = session->buf[62];
        yy = session->buf[63];
        session->live_flag = ((xx == 0) && ((yy & 0xf) == 2));
        lprintf("live: live_flag=%d, xx=%d, yy=%d\n", session->live_flag, xx, yy);
      }
      break;
    case 0x1A:
      /* authentication request, not yet supported */
      xprintf(XVERBOSITY_LOG,
               "libmmst: authentication request, not yet supported\n");
      goto fail;
      break;
    default:
      xprintf(XVERBOSITY_LOG,
               "libmmst: unexpected response: %02x (0x06 or 0x1A)\n", res);
      goto fail;
  }

  mmst_report_progress(session, 60);

  /* command 0x15 */
  lprintf("send command 0x15\n");
  {
    mmst_buffer_t command_buffer;
    mmst_buffer_init(&command_buffer, (uint8_t *)session->scmd_body);
    mmst_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x00800000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, 0x40AC2000);                  /* ?? */
    mmst_buffer_put_32 (&command_buffer, ASF_HEADER_PACKET_ID_TYPE);   /* Header Packet ID type */
    mmst_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    if (!send_command (session, 0x15, 1, 0, command_buffer.pos)) {
      xprintf(XVERBOSITY_LOG,
               "libmmst: failed to send command 0x15\n");
      goto fail;
    }
  }
  
  if ((res = get_answer (session)) != 0x11) {
    xprintf(XVERBOSITY_LOG,
             "libmmst: unexpected response: %02x (0x11)\n", res);
    goto fail;
  }

  session->num_stream_ids = 0;

  if (!get_asf_header(session))
    goto fail;
  if (interp_asf_header(session) <= 0)
    goto fail;
  
  mmst_report_progress(session, 70);
  
  if (!mmst_choose_best_streams(session)) {
    xprintf(XVERBOSITY_LOG,
             "libmmst: mmst_choose_best_streams failed");
    goto fail;
  }
/**
  mmst_report_progress(session, 80);
  
  * command 0x07 *
  {
    mmst_buffer_t command_buffer;
    mmst_buffer_init(&command_buffer, (uint8_t *)session->scmd_body);
    mmst_buffer_put_32 (&command_buffer, 0x00000000);    * 64 byte float timestamp *
    mmst_buffer_put_32 (&command_buffer, 0x00000000);    
    mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);    * ?? *
    mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);    * first packet sequence *
    mmst_buffer_put_8  (&command_buffer, 0xFF);          * max stream time limit (3 bytes) *
    mmst_buffer_put_8  (&command_buffer, 0xFF);
    mmst_buffer_put_8  (&command_buffer, 0xFF);
    mmst_buffer_put_8  (&command_buffer, 0x00);          * stream time limit flag *
    mmst_buffer_put_32 (&command_buffer, ASF_MEDIA_PACKET_ID_TYPE);    * asf media packet id type *
    if (!send_command (session, 0x07, 1, 0x0001FFFF, command_buffer.pos)) {
      xprintf(XVERBOSITY_LOG,
               "libmmst: failed to send command 0x07\n");
      goto fail;
    }
  }
 */
  
  mmst_report_progress(session, 100);
  
#ifdef USE_ICONV
  string_utf16_close(url_conv);
#endif
  
  lprintf("mmst_connect: passed\n" );
  
  return session;

fail:
  if (session->s != -1)
    close (session->s);
  if (session->url)
    free(session->url);
  if (session->proto)
    free(session->proto);
  if (session->host)
    free(session->host);
  if (session->user)
    free(session->user);
  if (session->password)
    free(session->password);
  if (session->uri)
    free(session->uri);
  
  free (session);
  
  return NULL;
}

extern int
mmst_play(mmst_t *session, uint32_t start, uint32_t length)
{
	double stamp = 0;
	mmst_buffer_t command_buffer;
	
	uint32_t stlow = 0;
	uint32_t sthigh = 0;
	uint8_t *stbuffer = NULL;
	
	mmsf_header_t *h = session->hobject;
	
	stamp = (uint32_t)(start * (h->prop.duration / 10000) / h->prop.packets);
	stamp /= 1000;
	
	if ((start > 0) && (start != UINT_MAX))
		session->pstart = start;
	
//	if ((start != 0) && (start != (uint32_t)-1))
	{
//		stamp = ((double)start)/1000;
		stbuffer = (uint8_t *)(void *)&stamp;
		
		stlow = LE_32(stbuffer + 0);
		sthigh = LE_32(stbuffer + sizeof(uint32_t));
	}
	
	/* command 0x07, sending play */
	
	mmst_buffer_init(&command_buffer, (uint8_t *)session->scmd_body);
	mmst_buffer_put_32 (&command_buffer, stlow);         /* 64 byte float timestamp */
	mmst_buffer_put_32 (&command_buffer, sthigh);        
	mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);    /* ?? */
	mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);    /* first packet sequence */
	mmst_buffer_put_8  (&command_buffer, 0xFF);          /* max stream time limit (3 bytes) */
	mmst_buffer_put_8  (&command_buffer, 0xFF);
	mmst_buffer_put_8  (&command_buffer, 0xFF);
	mmst_buffer_put_8  (&command_buffer, 0x00);          /* stream time limit flag */
	mmst_buffer_put_32 (&command_buffer, ASF_MEDIA_PACKET_ID_TYPE);    /* asf media packet id type */
	
	if (!send_command (session, 0x07, 1, 0x0001FFFF, command_buffer.pos))
	{
		xprintf(XVERBOSITY_LOG,
				"libmmst: failed to send command 0x07\n");
		goto fail;
	}
	
	lprintf("mmst_play: passed\n" );
	
	return 1;
	
fail:
	return 0;
}

static int
get_media_packet(mmst_t *session)
{
	int command;
	
	off_t len = 0;
	mmst_packet_header_t *header = &session->package;

	header->flags = 0;
	header->packet_len = 0;
	header->packet_seq = 0;
	
	switch (get_packet_header (session, header))
	{
	case MMS_PACKET_ERR:
		xprintf(XVERBOSITY_LOG,
			  "libmmst: failed to read mms packet header\n");
		return 0;
		break;
		
	case MMS_PACKET_COMMAND:
		command = get_packet_command (session, header->packet_len);
		switch (command)
		{
		case 0x1e:
			{
				uint32_t error_code;
				
				/* Warning: sdp is incomplete. Do not stop if error_code==1 */
				error_code = LE_32(session->buf + CMD_HEADER_LEN);
				lprintf("End of the current stream. Continue=%d\n", error_code);
				
				if (error_code == 0)
				{
					session->eos = 1;
					return 0;
				}
			}
			break;
			
		case 0x20:
			{
				lprintf("new stream.\n");
				/* asf header */
				if (!get_asf_header (session))
				{
					xprintf(XVERBOSITY_LOG,
							"failed to read new ASF header\n");
					return 0;
				}
				if (interp_asf_header(session) <= 0)
					return 0;
				if (!mmst_choose_best_streams(session))
					return 0;
				
				/* send command 0x07 */
				/* TODO: ugly */
				/* command 0x07 */
				{
				mmst_buffer_t command_buffer;
				mmst_buffer_init(&command_buffer, (uint8_t *)session->scmd_body);
				mmst_buffer_put_32 (&command_buffer, 0x00000000);    /* 64 byte float timestamp */
				mmst_buffer_put_32 (&command_buffer, 0x00000000);    
				mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);    /* ?? */
				mmst_buffer_put_32 (&command_buffer, 0xFFFFFFFF);    /* first packet sequence */
				mmst_buffer_put_8  (&command_buffer, 0xFF);          /* max stream time limit (3 bytes) */
				mmst_buffer_put_8  (&command_buffer, 0xFF);
				mmst_buffer_put_8  (&command_buffer, 0xFF);
				mmst_buffer_put_8  (&command_buffer, 0x00);          /* stream time limit flag */
				mmst_buffer_put_32 (&command_buffer, ASF_MEDIA_PACKET_ID_TYPE);  /* asf media packet id type */
				if (!send_command (session, 0x07, 1, 0x0001FFFF, command_buffer.pos)) {
				  xprintf(XVERBOSITY_LOG,
						   "libmmst: failed to send command 0x07\n");
				  return 0;
				}
				}
				/* session->current_pos = 0; */
			}
			break;
			
		case 0x1b:
			{
				if (!send_command (session, 0x1b, 0, 0, 0))
				{
					xprintf(XVERBOSITY_LOG,
							"libmmst: failed to send command\n");
					return 0;
				}
			}
			break;
			
		case 0x05:
			break;
		default:
			xprintf(XVERBOSITY_LOG,
					"unexpected mms command %02x\n", command);
			break;
		}
		session->buf_size = 0;
		break;
	  
	case MMS_PACKET_ASF_HEADER:
		xprintf(XVERBOSITY_LOG,
			  "libmmst: unexpected asf header packet\n");
		session->buf_size = 0;
		break;
	  
	case MMS_PACKET_ASF_PACKET:
		/* media packet */
		if ((header->packet_len > 10240) || 
			((int)header->packet_seq < 0) || ((int)header->packet_seq > 1000000))
		{
			uint8_t *head = session->buf_head;
			
			/* maybe need correct, this is invalidate */
			fprintf(stderr, "asf media packet invalidate %d, len=%d, seq=%d, 0x%02x\n",
				 header->packet_id_type, header->packet_len, header->packet_seq, header->flags);
			fprintf(stderr, "asf media packet %02x %02x %02x %02x : "\
				 "%02x %02x %02x %02x : %02x %02x %02x %02x\n",
				 head[0], head[1], head[2], head[3],
				 head[4], head[5], head[6], head[7],
				 head[0x08], head[0x09], head[0x0A], head[0x0B]);
			xutil_hexdump(session->buf_data, session->asf_packet_len + 32);
		}
/**		fprintf(stderr, "asf media packet detected, packet_len=%d, packet_seq=%d\n",
				 header->packet_len, header->packet_seq);
 */
		if ((int)header->packet_len > session->asf_packet_len)
		{
			xprintf(XVERBOSITY_LOG,
					"libmmst: invalid asf packet len: %d bytes\n", header->packet_len);
			return 0;
		}
		
		len = xio_tcp_read (session->s, session->buf, header->packet_len);
		if (len != (int)header->packet_len)
		{
			xprintf(XVERBOSITY_LOG,
					"libmmst: read failed\n");
			return 0;
		}
		if (session->asf_packet_len != header->packet_len)
		{
			/* explicit padding with 0 */
//			fprintf(stderr, "padding: %d bytes\n", session->asf_packet_len - header->packet_len);
			if (session->asf_packet_len > header->packet_len)
				memset(session->buf + header->packet_len, 0, session->asf_packet_len - header->packet_len);
		}
		session->buf_size = session->asf_packet_len;
		memcpy(session->buf_data, session->buf_head, 16);
		memcpy(session->buf_data + 16, session->buf, session->asf_packet_len);
		break;
	}
	
	lprintf("get media packet succ\n");
	
	return 1;
}
	

extern int
mmst_peek_header(mmst_t *session, char *data, int maxsize)
{
	int len;
	
	len = ((int)session->asf_header_len < maxsize) ? 	\
				 (int)session->asf_header_len : maxsize;
	
	memcpy(data, session->asf_header, len);
	return len;
}

extern int
mmst_read(mmst_t *session, char *data, int len)
{
	int n;
	int left;
	int total = 0;

	uint32_t stamp = 0;
	
	lprintf ("mmst_read: len: %d\n", len);
	
	while (total < len && !session->eos)
	{
/**		if (session->asf_header_read < session->asf_header_len)
		{
			left = session->asf_header_len - session->asf_header_read ;
			
			if ((len - total) < left)
				n = len - total;
			else
				n = left;
			xutil_fast_memcpy(&data[total], &session->asf_header[session->asf_header_read], n);
			
			session->asf_header_read += n;
			total += n;
			session->current_pos += n;
		}
		else
 */
		{
			left = session->buf_size - session->buf_read;
			if (left > 0)
			{
				session->stact = session->stamp;
				if (session->plast > 0)
				{
					session->pnum++;
					session->psize += session->plast;
					session->plast = 0;
				}
			}
			if (left == 0)
			{
				session->buf_read = 0;
				session->buf_size = 0;
				if (!get_media_packet (session))
				{
					xprintf(XVERBOSITY_LOG,
							"libmmst: get_media_packet failed\n");
					return total;
				}
				left = session->buf_size;
				if (left <= 0)
				{
					session->buf_read = 0;
					session->buf_size = 0;
					continue;
				}
				
				/* fixed size, skip session->package.packet_len */
				session->plast = session->asf_packet_len;
				mms_get_stamp(session->buf, session->plast, &stamp);
				if (session->stamp < stamp)
					session->stamp = stamp;
				
				if (session->hobject && (session->package.packet_len > 0))
				{
					if ((session->pstart != UINT_MAX) && 
						(session->package.packet_seq < session->pstart))
					{
						session->buf_read = 0;
						session->buf_size = 0;
						continue;
					}
					n = mms_fix_pack(session->buf, session->plast,
						 session->package.packet_len, NULL);
				}
			}
			
			if ((len - total) < left)
				n = len - total;
			else
				n = left;
			xutil_fast_memcpy(&data[total], &session->buf[session->buf_read], n);
			
			total += n;
			session->buf_read += n;
			session->current_pos += n;
		}
	}
	
	session->stact = session->stamp;
	
	lprintf ("%d bytes provided\n", total);
	
	return total;
}


extern void
mmst_close(mmst_t *session)
{
  if ((0 == session->refers) && (session->s != -1))
    close(session->s);
  if (session->url)
    free(session->url);
  if (session->proto)
    free(session->proto);
  if (session->host)
    free(session->host);
  if (session->user)
    free(session->user);
  if (session->password)
    free(session->password);
  if (session->uri)
    free(session->uri);

  free (session);
}

extern uint32_t
mmst_get_length(mmst_t *session)
{
  return (uint32_t)session->file_len;
}

extern off_t
mmst_get_current_pos(mmst_t *session)
{
  return (off_t)session->current_pos;
}


extern mmst_session_t *
mmst_session_start(int fd, const char *mrl)
{
	mmst_session_t *session = NULL;
	
	session = (mmst_session_t *)mmst_connect(fd, mrl, -1);
	return session;
}

extern int
mmst_session_play(mmst_session_t *session, uint32_t start, uint32_t length)
{
	return mmst_play(session, start, length);
}

extern int
mmst_session_read(mmst_session_t *session, char *data, int len)
{
	return mmst_read(session, data, len);
}

extern int
mmst_session_seq(mmst_session_t *session, unsigned int *seq)
{
	if ((NULL == session) || (session->package.packet_len <= 0))
		return -ENODATA;
	if (seq != NULL)
		*seq = session->package.packet_seq;
	return (int)session->package.packet_seq;
}

extern int
mmst_session_stamp(mmst_session_t *session, unsigned int *stamp)
{
	if (session->stact <= 0)
		return -ENODATA;
	
	if (stamp != NULL)
		*stamp = session->stact;
	
	return (int)session->pnum;
}

extern void *
mmst_session_package(mmst_session_t *session)
{
	if (NULL == session)	/* ( || (session->package.length <= 0)) */
		return NULL;
	return (void *)&session->package;
}

extern int
mmst_session_packouts(mmst_session_t *session, uint32_t *num, uint32_t *size)
{
	if (session->pnum <= 0)
		return -ENODATA;
	
	if (num != NULL)
		*num = session->pnum;
	if (size != NULL)
		*size = session->psize;
	return session->pnum;
}

extern void *
mmst_session_header(mmst_session_t *session)
{
	if ((NULL == session) || (NULL == session->hobject))
		return NULL;
	return (void *)session->hobject;
}

extern int
mmst_session_fix_header(mmst_session_t *session)
{
	return 0;
}

extern int
mmst_session_got_header(mmst_session_t *session, void **header)
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

extern int
mmst_session_peek_header(mmst_session_t *session, char *buf, int maxsize)
{
	int ret = -EINVAL;
	
	if (maxsize <= 0)
		return -EINVAL;
	if (session->asf_header_len <= 0)
		return -ENODATA;
	if (maxsize < session->asf_header_len)
		return -ENOBUFS;
	
	memcpy(buf, session->asf_header, session->asf_header_len);
	ret = session->asf_header_len;
	
	return ret;
}

extern void
mmst_session_end(mmst_session_t *session)
{
	mmst_close(session);
}


