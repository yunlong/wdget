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
 * $Id: mmsh.c,v 1.33.2.1 2005/01/15 17:51:41 mroi Exp $
 *
 * MMS over HTTP protocol
 *   written by Thibaut Mattern
 *   based on mms.c and specs from avifile
 *   (http://avifile.sourceforge.net/asf-1.0.htm)
 *
 * TODO:
 *   error messages
 *   http support cleanup, find a way to share code with input_http.c (http.h|c)
 *   http proxy support
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __GNUC__
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
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

/********** logging **********/
#define LOG_MODULE      "mmsh"
#define LOG_VERBOSE
/*
#define LOG
*/
#include "xsysio.h"
#include "mmsh.h"

#include "mms_asf.h"


/* #define USERAGENT "User-Agent: NSPlayer/7.1.0.3055\r\n" */
#define USERAGENT "User-Agent: NSPlayer/4.1.0.3856\r\n"
#define CLIENTGUID "Pragma: xClientGUID={c77e7400-738a-11d2-9add-0020af0a3278}\r\n"


#define MMSH_PORT                  80
#define MMSH_UNKNOWN                MMS_STREAM_UNKNOWN
#define MMSH_LIVE                   MMS_STREAM_LIVE
#define MMSH_SEEKABLE               MMS_STREAM_SEEKABLE

#define CHUNK_HEADER_LENGTH         4
#define EXT_HEADER_LENGTH           8
#define CHUNK_TYPE_RESET       0x4324
#define CHUNK_TYPE_DATA        0x4424
#define CHUNK_TYPE_END         0x4524
#define CHUNK_TYPE_ASF_HEADER  0x4824
#define CHUNK_SIZE              65536  /* max chunk size */
#define ASF_HEADER_SIZE          8192  /* max header size */

#define SCRATCH_SIZE             1024

static const char *mmsh_FirstRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Pragma: no-cache,rate=1.000000,stream-time=0,stream-offset=0:0,request-context=%u,max-duration=0\r\n"
    CLIENTGUID
    "Connection: Keep-Alive\r\n\r\n";
//  "Connection: Close\r\n\r\n";

static const char *mmsh_SeekableRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Pragma: no-cache,rate=1.000000,stream-time=%u,stream-offset=%u:%u,request-context=%u,max-duration=%u\r\n"
    CLIENTGUID
    "Pragma: xPlayStrm=1\r\n"
    "Pragma: stream-switch-count=%d\r\n"
    "Pragma: stream-switch-entry=%s\r\n" /*  ffff:1:0 ffff:2:0 */
    "Connection: Close\r\n\r\n";

static const char *mmsh_LiveRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Pragma: no-cache,rate=1.000000,request-context=%u\r\n"
    "Pragma: xPlayStrm=1\r\n"
    CLIENTGUID
    "Pragma: stream-switch-count=%d\r\n"
    "Pragma: stream-switch-entry=%s\r\n"
    "Connection: Close\r\n\r\n";

/* Unused requests */
#if 0
static const char* mmsh_PostRequest =
    "POST %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s\r\n"
    "Pragma: client-id=%u\r\n"
/*    "Pragma: log-line=no-cache,rate=1.000000,stream-time=%u,stream-offset=%u:%u,request-context=2,max-duration=%u\r\n"
 */
    "Pragma: Content-Length: 0\r\n"
    CLIENTGUID
    "\r\n";

static const char* mmsh_RangeRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Range: bytes=%Lu-\r\n"
    CLIENTGUID
    "Connection: Close\r\n\r\n";
#endif


struct mmsh_packet_header_s
{
  uint16_t  chunk_type;
  uint16_t  chunk_length;
  uint16_t  chunk_seq_number;
  
  uint16_t  ext_length;
  uint8_t   ext_header[EXT_HEADER_LENGTH];
};

typedef struct mmsh_packet_header_s mmsh_packet_header_t;


/**
 * mmsh specific types 
 */
struct mmsh_s
{
//  xine_stream_t *stream;

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
  
  char          str[SCRATCH_SIZE]; /* scratch buffer to built strings */
  
  int           stream_type;  /* seekable or broadcast */
  
  /* receive buffer */
  
  /* chunk */
  uint16_t      chunk_type;
  uint16_t      chunk_length;
  uint16_t      chunk_seq_number;
  uint8_t       buf[CHUNK_SIZE];
  
  int           buf_read;
  int           buf_size;
  
  uint8_t       asf_header[ASF_HEADER_SIZE];
  uint32_t      asf_header_len;
  uint32_t      asf_header_read;
  int           seq_num;
  int           num_stream_ids;
  int           stream_ids[ASF_MAX_NUM_STREAMS];
  int           stream_types[ASF_MAX_NUM_STREAMS];
  int           packet_length;
  int64_t       file_length;
  char          guid[37];
  uint32_t      bitrates[ASF_MAX_NUM_STREAMS];
  uint32_t      bitrates_pos[ASF_MAX_NUM_STREAMS];
  
  int           has_audio;
  int           has_video;
  
  off_t         current_pos;
  int           user_bandwidth;
  
  uint32_t      pnum;
  uint32_t      plast;
  uint32_t      psize;
  
  uint32_t      stact;
  uint32_t      stamp;
  
  mmsf_header_t *hobject;
  mmsh_packet_header_t package;
};


static int
send_command(mmsh_t *session, char *cmd)
{
  int length;
  
  //lprintf("send_command:\n%s\n", cmd);
//  fprintf(stderr, "-> %s\r\n", cmd);
  
  length = strlen(cmd);
  if (xio_tcp_write(session->s, cmd, length) != length) {
    xprintf (XLOG_MSG, _("libmmsh: send error\n"));
    return 0;
  }
  return 1;
}

static int
get_answer(mmsh_t *session)
{
  int done, len, linenum;
  char *features;

  lprintf ("get_answer\n");

  done = 0; len = 0; linenum = 0;
  session->stream_type = MMSH_UNKNOWN;

  while (!done) {

    if (xio_tcp_read(session->s, &(session->buf[len]), 1) != 1) {
      xprintf (XVERBOSITY_LOG,
               "libmmsh: alert: end of stream\n");
      return 0;
    }

    if (session->buf[len] == '\012') {

      session->buf[len] = '\0';
      len--;
      
      if ((len >= 0) && (session->buf[len] == '\015')) {
        session->buf[len] = '\0';
        len--;
      }
      
      linenum++;
      
      //lprintf("answer: >%s<\n", session->buf);
//	  fprintf(stderr, "<- %s\r\n", session->buf);
      
      if (linenum == 1)
      {
        int httpver, httpsub, httpcode;
        char httpstatus[51];

        if (sscanf((char *)session->buf, "HTTP/%d.%d %d %50[^\015\012]",
			&httpver, &httpsub,
            &httpcode, httpstatus) != 4)
        {
          llprintf (XLOG_MSG,
		    _("libmmsh: bad response format\n"));
          return 0;
        }

        if (httpcode >= 300 && httpcode < 400) {
          llprintf (XLOG_MSG,
		    _("libmmsh: 3xx redirection not implemented: >%d %s<\n"),
		    httpcode, httpstatus);
          return 0;
        }

        if (httpcode < 200 || httpcode >= 300) {
          llprintf (XLOG_MSG,
		    _("libmmsh: http status not 2xx: >%d %s<\n"),
		    httpcode, httpstatus);
          return 0;
        }
      } else {

        if (!strncasecmp((char *)session->buf, "Location: ", 10)) {
          llprintf (XLOG_MSG,
		    _("libmmsh: Location redirection not implemented\n"));
          return 0;
        }
        
        if (!strncasecmp((char *)session->buf, "Pragma:", 7)) {
          features = strstr((char *)session->buf + 7, "features=");
          if (features) {
            if (strstr(features, "seekable")) {
              lprintf("seekable stream\n");
              session->stream_type = MMSH_SEEKABLE;
            } else {
              if (strstr(features, "broadcast")) {
                lprintf("live stream\n");
                session->stream_type = MMSH_LIVE;
              }
            }
          }
        }
      }
      
      if (len == -1) {
        done = 1;
      } else {
        len = 0;
      }
    } else {
      len ++;
    }
  }
  if (session->stream_type == MMSH_UNKNOWN) {
    xprintf (XVERBOSITY_LOG,
             "libmmsh: unknown stream type\n");
    session->stream_type = MMSH_SEEKABLE; /* FIXME ? */
  }
  return 1;
}

static int
get_chunk_header(mmsh_t *session)
{
  uint8_t chunk_header[CHUNK_HEADER_LENGTH];
  uint8_t *extend_header = session->package.ext_header;
  
  int read_len;
  int ext_header_len;

  lprintf ("get_chunk_header\n");
  
  /* read chunk header */
  read_len = xio_tcp_read(session->s, chunk_header, CHUNK_HEADER_LENGTH);
  if (read_len != CHUNK_HEADER_LENGTH) {
    xprintf (XVERBOSITY_LOG,
             "libmmsh: chunk header read failed, %d != %d\n", read_len, CHUNK_HEADER_LENGTH);
    return 0;
  }
  session->chunk_type       = LE_16 (&chunk_header[0]);
  session->chunk_length     = LE_16 (&chunk_header[2]);
  
  switch (session->chunk_type)
  {
    case CHUNK_TYPE_DATA:
      ext_header_len = 8;
      break;
    case CHUNK_TYPE_END:
      ext_header_len = 4;
      break;
    case CHUNK_TYPE_ASF_HEADER:
      ext_header_len = 8;
      break;
    case CHUNK_TYPE_RESET:
      ext_header_len = 4;
      break;
    default:
      ext_header_len = 0;
  }
  session->package.ext_length = ext_header_len;
  
  /* read extended header */
  if (ext_header_len > 0)
  {
    read_len = xio_tcp_read(session->s, extend_header, ext_header_len);
    if (read_len != ext_header_len) {
      xprintf (XVERBOSITY_LOG,
               "extended header read failed, %d != %d\n", read_len, ext_header_len);
      return 0;
    }
  }
  
  switch (session->chunk_type)
  {
    case CHUNK_TYPE_DATA:
      session->chunk_seq_number = LE_32 (&extend_header[0]);
      lprintf ("chunk type:       CHUNK_TYPE_DATA\n");
      lprintf ("chunk length:     %d\n", session->chunk_length);
      lprintf ("chunk seq:        %d\n", session->chunk_seq_number);
      lprintf ("unknown:          %d\n", extend_header[4]);
      lprintf ("mmsh seq:         %d\n", extend_header[5]);
      lprintf ("len2:             %d\n", LE_16(&extend_header[6]));
      break;
    case CHUNK_TYPE_END:
      session->chunk_seq_number = LE_32 (&extend_header[0]);
      lprintf ("chunk type:       CHUNK_TYPE_END\n");
      lprintf ("continue: %d\n", session->chunk_seq_number);
      break;
    case CHUNK_TYPE_ASF_HEADER:
      lprintf ("chunk type:       CHUNK_TYPE_ASF_HEADER\n");
      lprintf ("chunk length:     %d\n", session->chunk_length);
      lprintf ("unknown:          %2X %2X %2X %2X %2X %2X\n",
               extend_header[0], extend_header[1], extend_header[2],
               extend_header[3], extend_header[4], extend_header[5]);
      lprintf ("len2:             %d\n", LE_16(&extend_header[6]));
      break;
    case CHUNK_TYPE_RESET:
      lprintf ("chunk type:       CHUNK_TYPE_RESET\n");
      lprintf ("chunk seq:        %d\n", session->chunk_seq_number);
      lprintf ("unknown:          %2X %2X %2X %2X\n",
               extend_header[0], extend_header[1], extend_header[2], extend_header[3]);
      break;
    default:
      lprintf ("unknown chunk:          %4X\n", session->chunk_type);
  }
  session->chunk_length -= ext_header_len;
  
  session->package.chunk_type = session->chunk_type;
  session->package.chunk_length = session->chunk_length;
  session->package.chunk_seq_number = session->chunk_seq_number;
  
  return 1;
}

static int
get_header(mmsh_t *session)
{
  int len = 0;

  lprintf("get_header\n");

  session->asf_header_len = 0;
  session->asf_header_read = 0;
  
  /* read chunk */
  while (1) {
    if (get_chunk_header(session)) {
      if (session->chunk_type == CHUNK_TYPE_ASF_HEADER) {
        if ((session->asf_header_len + session->chunk_length) > ASF_HEADER_SIZE) {
          xprintf (XVERBOSITY_LOG,
                   "libmmsh: the asf header exceed %d bytes\n", ASF_HEADER_SIZE);
          return 0;
        } else {
          len = xio_tcp_read(session->s, session->asf_header + session->asf_header_len,
                             session->chunk_length);
          session->asf_header_len += len;
          if (len != session->chunk_length) {
            return 0;
          }
        }
      } else {
        break;
      }
    } else {
      lprintf("get_chunk_header failed\n");
      return 0;
    }
  }

  if (session->chunk_type == CHUNK_TYPE_DATA) {
    /* read the first data chunk */
    len = xio_tcp_read(session->s, session->buf, session->chunk_length);
    if (len != session->chunk_length) {
      return 0;
    } else {
      return 1;
    }
  } else {
    /* unexpected packet type */
    return 0;
  }
}

static int
interp_header(mmsh_t *session)
{
  int ret = -EINVAL;
  int i;
  
  lprintf ("interp_header, header_len=%d\n", session->asf_header_len);
  
  session->packet_length = 0;

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
   * parse asf header
   */
  i = 30;
  while ((i + 24) < (int)session->asf_header_len)
  {
    int guid;
    uint64_t length;

    guid = mms_get_guid(session->asf_header, i);
    i += 16;
    length = LE_64(session->asf_header + i);
    i += 8;

    if ((i + length) >= session->asf_header_len)
      return -ENOBUFS;

    switch (guid)
    {
      case GUID_ASF_FILE_PROPERTIES:

        session->packet_length = LE_32(session->asf_header + i + 92 - 24);
        session->file_length   = LE_64(session->asf_header + i + 40 - 24);
        lprintf ("file object, file_length = %lld, packet length = %d",
		 session->file_length, session->packet_length);
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

          session->stream_types[stream_id] = type;
          session->stream_ids[session->num_stream_ids] = stream_id;
          session->num_stream_ids++;

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
            xprintf (XVERBOSITY_LOG,
                     "libmmsh: stream id %d, bitrate %d\n",
                     stream_id, session->bitrates[stream_id]);
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
      i += (int)length - 24;
    }
  }
  
  return session->asf_header_len;
}

const static char *const mmsh_proto_s[] = { "mms", "mmsh", NULL };

static int
mmsh_valid_proto(char *proto)
{
  int i = 0;

  lprintf("mmsh_valid_proto\n");

  if (!proto)
    return 0;

  while(mmsh_proto_s[i]) {
    if (!strcasecmp(proto, mmsh_proto_s[i])) {
      return 1;
    }
    i++;
  }
  return 0;
}

static void
mmsh_report_progress(mmsh_t *session, int p)
{
/**
  xine_event_t             event;
  xine_progress_data_t     prg;

  prg.description = _("Connecting MMS server (over http)...");
  prg.percent = p;

  event.type = XINE_EVENT_PROGRESS;
  event.data = &prg;
  event.data_length = sizeof (xine_progress_data_t);

  xine_event_send (stream, &event);
 */
}

/*
 * returns 1 on error
 */
static int
mmsh_tcp_connect(mmsh_t *session)
{
  int progress, res;
  
  if (!session->port) session->port = MMSH_PORT;
  
  /* 
   * try to connect 
   */
  lprintf("try to connect to %s on port %d \n", session->host, session->port);

  session->s = xio_tcp_connect (session->host, session->port);

  if (session->s == -1) {
    xprintf (XVERBOSITY_LOG,
             "libmmsh: failed to connect '%s'\n", session->host);
    return 1;
  }

  /* connection timeout 15s */
  progress = 0;
  do {
    mmsh_report_progress(session, progress);
    res = xio_select (session->s, XIO_WRITE_READY, 500);
    progress += 1;
  } while ((res == XIO_TIMEOUT) && (progress < 30));
  if (res != XIO_READY) {
    return 1;
  }
  lprintf ("connected\n");

  return 0;
}


static int
mmsh_connect_int(mmsh_t *session, int bandwidth)
{
/*
  int    i;
  int    video_stream = -1;
  int    audio_stream = -1;
  int    max_arate    = -1;
  int    min_vrate    = -1;
  int    min_bw_left  = 0;
  int    stream_id;
  int    bandwitdh_left;
  int    offset;
  char   stream_selection[10 * ASF_MAX_NUM_STREAMS];  * 10 chars per stream */
  
  /*
   * let the negotiations begin...
   */
  session->num_stream_ids = 0;
  
  /* first request */
  lprintf("first http request\n");
  
  snprintf(session->str, SCRATCH_SIZE, mmsh_FirstRequest, session->uri,
            session->host, session->port, 1);
  
  if (!send_command(session, session->str))
    goto fail;
  
  if (!get_answer(session)) 
    goto fail;
  
  get_header(session);
  //if (! ) goto fail;
  interp_header(session);
  //if ( <= 0) goto fail;
  mmsh_report_progress(session, 40);
  
/**
   * choose the best quality for the audio stream *
   * i've never seen more than one audio stream *
  for (i = 0; i < session->num_stream_ids; i++) {
    stream_id = session->stream_ids[i];
    switch (session->stream_types[stream_id]) {
      case ASF_STREAM_TYPE_AUDIO:
        if ((audio_stream == -1) || ((int)session->bitrates[stream_id] > max_arate)) {
          audio_stream = stream_id;
          max_arate = session->bitrates[stream_id];
        }
        break;
      default:
        break;
    }
  }
  
   * choose a video stream adapted to the user bandwidth *
  bandwitdh_left = bandwidth - max_arate;
  if (bandwitdh_left < 0) {
    bandwitdh_left = 0;
  }
  lprintf("bandwitdh %d, left %d\n", bandwidth, bandwitdh_left);

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
  
   * choose the stream with the lower bitrate *
  if ((video_stream == -1) && session->has_video) {
    for (i = 0; i < session->num_stream_ids; i++) {
      stream_id = session->stream_ids[i];
      switch (session->stream_types[stream_id]) {
        case ASF_STREAM_TYPE_VIDEO:
          if ((video_stream == -1) || 
              ((int)session->bitrates[stream_id] < min_vrate) ||
              (!min_vrate)) {
            video_stream = stream_id;
            min_vrate = session->bitrates[stream_id];
          }
          break;
        default:
          break;
      }
    }
  }
  
  lprintf("audio stream %d, video stream %d\n", audio_stream, video_stream);
  
  if ((0 == session->refers) && (session->s != -1))
  {
    close(session->s);
    session->s = -1;
  }
  
  * second request *
  lprintf("second http request\n");
  
  if (mmsh_tcp_connect(session)) {
    goto fail;
  }
  
   * stream selection string *
   * The same selection is done with mmst *
   * 0 means selected *
   * 2 means disabled *
  offset = 0;
  for (i = 0; i < session->num_stream_ids; i++) {
    int size;
    if ((session->stream_ids[i] == audio_stream) ||
        (session->stream_ids[i] == video_stream)) {
      size = snprintf(stream_selection + offset, sizeof(stream_selection) - offset,
                      "ffff:%d:0 ", session->stream_ids[i]);
    } else {
      xprintf (XVERBOSITY_LOG,
               "disabling stream %d\n", session->stream_ids[i]);
      size = snprintf(stream_selection + offset, sizeof(stream_selection) - offset,
                      "ffff:%d:2 ", session->stream_ids[i]);
    }
    if (size < 0) goto fail;
    offset += size;
  }
  
  switch (session->stream_type)
  {
    case MMSH_LIVE:
      snprintf(session->str, SCRATCH_SIZE, mmsh_LiveRequest, session->uri,
                session->host, session->port, 2,
                session->num_stream_ids, stream_selection);
      break;
    case MMSH_SEEKABLE:
      snprintf(session->str, SCRATCH_SIZE, mmsh_SeekableRequest, session->uri,
                session->host, session->port, 0, 0, 0, 2, 0,
                session->num_stream_ids, stream_selection);
      break;
  }
  
  if (!send_command (session, session->str))
    goto fail;
  
  lprintf("before read \n");
  
  if (!get_answer (session))
    goto fail;
  if (!get_header(session))
    goto fail;
  if (interp_header(session) <= 0)
    goto fail;
  session->buf_size = session->packet_length;
  
  for (i = 0; i < session->num_stream_ids; i++) {
    if ((session->stream_ids[i] != audio_stream) &&
        (session->stream_ids[i] != video_stream)) {
      lprintf("disabling stream %d\n", session->stream_ids[i]);

       * forces the asf demuxer to not choose session stream *
      if (session->bitrates_pos[session->stream_ids[i]]) {
        session->asf_header[session->bitrates_pos[session->stream_ids[i]]]     = 0;
        session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 1] = 0;
        session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 2] = 0;
        session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 3] = 0;
      }
    }
  }
 */
  
  return 1;
  
fail:
  return 0;
}

extern mmsh_t *
mmsh_connect(int fd, const char *url, int bandwidth)
{
  mmsh_t *session;
 
  if (!url)
    return NULL;
  
  session = (mmsh_t*) xutil_malloc (sizeof (mmsh_t));
  
//  session->stream          = stream;
  session->url             = strdup(url);
  session->s               = -1;
  session->refers          = 0;
  session->asf_header_len  = 0;
  session->asf_header_read = 0;
  session->num_stream_ids  = 0;
  session->packet_length   = 0;
  session->buf_read        = 0;
  session->buf_size        = 0;
  session->has_audio       = 0;
  session->has_video       = 0;
  session->current_pos     = 0;
  if (bandwidth <= 0)
    bandwidth = 10485800;
  session->user_bandwidth  = bandwidth;
  
  session->pnum = 0;
  session->psize = 0;
  session->plast = 0;
  session->stact = 0;
  session->stamp = 0;
  session->hobject = NULL;
  memset(&session->package, 0, sizeof(session->package));
  
  if (!xutil_parse_url (session->url, &session->proto, &session->host, &session->port,
                  &session->user, &session->password, &session->uri)) {
    llprintf (XLOG_MSG, _("invalid url\n"));
    goto fail;
  }
  
  mmsh_report_progress(session, 0);
  
  if (!mmsh_valid_proto(session->proto)) {
    llprintf (XLOG_MSG, _("unsupported protocol\n"));
    goto fail;
  }
  
  if (fd >= 0)
  {
    session->s = fd;
    fd = -1;
    session->refers = 1;
  }
  else
  {
    session->refers = 0;
    if (mmsh_tcp_connect(session))
      goto fail;
  }
  
  mmsh_report_progress(session, 30);
  
  if (!mmsh_connect_int(session, session->user_bandwidth))
    goto fail;
  
  mmsh_report_progress(session, 100);
  
  lprintf("mmsh_connect: passed\n" );
  
  return session;
  
fail:
  lprintf("mmsh_connect: failed\n" );
  
  if (session != NULL)
    mmsh_close(session);
/**
 * 
 if (session->s != -1)
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
  
  free(session);
 */
  lprintf("mmsh_connect: failed return\n" );
  return NULL;
}

extern int
mmsh_play(mmsh_t *session, uint32_t start, uint32_t length)
{
	int i = 0;
	int size = 0;
	int offset = 0;
	int stream_id = 0;
	char stream_selection[10 * ASF_MAX_NUM_STREAMS]; /* 10 chars per stream */
	
	int max_arate = -1;
	int min_vrate = -1;
	int video_stream = -1;
	int audio_stream = -1;
	int min_bw_left = 0;
	int bandwidth_left = 0;
	
	uint32_t stime = 0;
	uint32_t sduration = 0;
	
	uint32_t stamp = start;
	uint32_t stlow = stamp;
	uint32_t sthigh = 0;
	
	if (session->hobject != NULL)
	{
		size = (int)(session->hobject->file.size + sizeof(session->hobject->data));
		if (start < size)
			stlow = 0;
		else
			stlow -= size;
	}
	
	/* choose the best quality for the audio stream */
	/* i've never seen more than one audio stream */
	for (i = 0; i < session->num_stream_ids; i++)
	{
		stream_id = session->stream_ids[i];
		switch (session->stream_types[stream_id])
		{
		case ASF_STREAM_TYPE_AUDIO:
			if ((audio_stream == -1) || ((int)session->bitrates[stream_id] > max_arate))
			{
				audio_stream = stream_id;
				max_arate = session->bitrates[stream_id];
			}
			break;
		default:
			break;
		}
	}
	
	/* choose a video stream adapted to the user bandwidth */
	bandwidth_left = session->user_bandwidth - max_arate;
	if (bandwidth_left < 0)
		bandwidth_left = 0;
	
	lprintf("bandwidth %d, left %d\n", session->user_bandwidth, bandwidth_left);
	
	min_bw_left = bandwidth_left;
	for (i = 0; i < session->num_stream_ids; i++)
	{
		stream_id = session->stream_ids[i];
		switch (session->stream_types[stream_id])
		{
		case ASF_STREAM_TYPE_VIDEO:
			if (((bandwidth_left - (int)session->bitrates[stream_id]) < min_bw_left) &&
				 (bandwidth_left >= (int)session->bitrates[stream_id]))
			{
				video_stream = stream_id;
				min_bw_left = bandwidth_left - session->bitrates[stream_id];
			}
			break;
		default:
			break;
		}
	}
	
	/* choose the stream with the lower bitrate */
	if ((video_stream == -1) && session->has_video)
	{
		for (i = 0; i < session->num_stream_ids; i++)
		{
			stream_id = session->stream_ids[i];
			switch (session->stream_types[stream_id])
			{
			case ASF_STREAM_TYPE_VIDEO:
				if ((video_stream == -1) || 
					((int)session->bitrates[stream_id] < min_vrate) ||
					(!min_vrate))
				{
					video_stream = stream_id;
					min_vrate = session->bitrates[stream_id];
				}
				break;
			default:
				break;
			}
		}
	}
	
	lprintf("audio stream %d, video stream %d\n", audio_stream, video_stream);
	
	if ((0 == session->refers) && (session->s != -1))
	{
		close(session->s);
		session->s = -1;
	}
	session->refers = 0;
	if (mmsh_tcp_connect(session))
		goto fail;
	
	/* second request */
	lprintf("second http request\n");
	
	/* stream selection string */
	/* The same selection is done with mmst */
	/* 0 means selected */
	/* 2 means disabled */
	offset = 0;
	for (i = 0; i < session->num_stream_ids; i++)
	{
		if ((session->stream_ids[i] == audio_stream) ||
			(session->stream_ids[i] == video_stream))
		{
			size = snprintf(stream_selection + offset, sizeof(stream_selection) - offset,
				 "ffff:%d:0 ", session->stream_ids[i]);
		}
		else
		{
			xprintf(XVERBOSITY_LOG,
				    "disabling stream %d\n", session->stream_ids[i]);
			size = snprintf(stream_selection + offset, sizeof(stream_selection) - offset,
				 "ffff:%d:2 ", session->stream_ids[i]);
		}
		if (size < 0)
			goto fail;
		offset += size;
	}
	
	switch (session->stream_type)
	{
	case MMSH_LIVE:
		snprintf(session->str, SCRATCH_SIZE, mmsh_LiveRequest, session->uri,
		session->host, session->port, 2,
		session->num_stream_ids, stream_selection);
		break;
	case MMSH_SEEKABLE:
	default:
		snprintf(session->str, SCRATCH_SIZE, mmsh_SeekableRequest, session->uri,
			 session->host, session->port, stime, sthigh, stlow, 2, sduration,
			 session->num_stream_ids, stream_selection);
		break;
	}
	if (!send_command (session, session->str))
		goto fail;
	
	lprintf("before read \n");
	
	if (!get_answer(session))
		goto fail;
	if (!get_header(session))
		goto fail;
	if (interp_header(session) <= 0)
		goto fail;
	session->buf_size = session->packet_length;
	
	for (i = 0; i < session->num_stream_ids; i++)
	{
		if ((session->stream_ids[i] != audio_stream) &&
			(session->stream_ids[i] != video_stream))
		{
			lprintf("disabling stream %d\n", session->stream_ids[i]);
			
			/* forces the asf demuxer to not choose session stream */
			if (session->bitrates_pos[session->stream_ids[i]])
			{
				session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 0] = 0;
				session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 1] = 0;
				session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 2] = 0;
				session->asf_header[session->bitrates_pos[session->stream_ids[i]] + 3] = 0;
			}
		}
	}
	
	return 1;
	
fail:
	return 0;
}

/**
 * @returned value:
 *  0: error
 *  1: data packet read
 *  2: new header read
 */
static int
get_media_packet(mmsh_t *session)
{
	int len = 0;
	
	lprintf("get_media_packet: session->packet_length: %d\n", session->packet_length);
	
	if (! get_chunk_header(session))
		return 0;
	
	switch (session->chunk_type)
	{
	case CHUNK_TYPE_END:
		/* session->chunk_seq_number:
		 *     0: stop
		 *     1: a new stream follows
		 */
		if (session->chunk_seq_number == 0)
			return 0;
		
		if ((0 == session->refers) && (session->s != -1))
		{
			close(session->s);
			session->s = -1;
		}
		session->refers = 0;
		if (mmsh_tcp_connect(session))
			return 0;
		
		if (!mmsh_connect_int(session, session->user_bandwidth))
			return 0;
		/* mmsh_connect_int reads the first data packet */
		/* session->buf_size is set by mmsh_connect_int */    
		return 2;
		
	case CHUNK_TYPE_DATA:
		/* nothing to do */
		break;
		
	case CHUNK_TYPE_RESET:
		/* next chunk is an ASF header */
		if (session->chunk_length != 0)
		{
			/* that's strange, don't know what to do */
			return 0;
		}
		if (!get_header(session))
			return 0;
		if (interp_header(session) <= 0)
			return 0;
		session->buf_size = session->packet_length;
		return 2;
		
	default:
		xprintf (XVERBOSITY_LOG,
				 "libmmsh: unexpected chunk type\n");
		return 0;
	}
	
	len = xio_tcp_read (session->s, session->buf, session->chunk_length);
	
	if (len == session->chunk_length)
	{
/**		int tag = 0;
		int chunk = session->chunk_length;
		
		if (chunk > 6)
		{
			tag = ((uint8_t *)session->buf)[3];
			if ((0x40 == tag) || (0x41 == tag))
			{
				((uint8_t *)session->buf)[3] = (uint8_t)0x09;
				((uint8_t *)session->buf)[5] = (uint8_t)(tag & 0x0F);
				memmove(session->buf + 6, session->buf + 7, chunk - 7);
				
				((uint8_t *)session->buf)[chunk - 1] = (uint8_t)0x00;
			}
		}
 */
		/* explicit padding with 0 */
		if (session->chunk_length > session->packet_length)
		{
			xprintf (XVERBOSITY_LOG,
					 "libmmsh: chunk_length(%d) > packet_length(%d)\n",
					 session->chunk_length, session->packet_length);
			return 0;
		}
		if (session->packet_length > session->chunk_length)
		{
			memset(session->buf + session->chunk_length,
				 0, session->packet_length - session->chunk_length);
		}
		session->buf_size = session->packet_length;
		return 1;
	}
	else
	{
		xprintf (XVERBOSITY_LOG,
			   "libmmsh: read error, %d != %d\n", len, session->chunk_length);
		return 0;
	}
}

extern int
mmsh_peek_header(mmsh_t *session, char *data, int maxsize)
{
  int len;

  lprintf("mmsh_peek_header\n");

  len = ((int)session->asf_header_len < maxsize) ? 	\
	          (int)session->asf_header_len : maxsize;

  memcpy(data, session->asf_header, len);
  return len;
}

extern int
mmsh_read(mmsh_t *session, char *data, int len)
{
	int n;
	int left;
	int total = 0;
	
	int ptype = 0;
	uint32_t stamp = 0;
	
	lprintf ("mmsh_read: len: %d\n", len);
	
	while (total < len)
	{
/**		if (session->asf_header_read < session->asf_header_len)
		{
			left = session->asf_header_len - session->asf_header_read ;
			
			if ((len - total) < left)
				n = len - total;
			else
				n = left;
			xutil_fast_memcpy (&data[total], &session->asf_header[session->asf_header_read], n);
			
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
				ptype = get_media_packet (session);
				if (ptype == 0)
				{
					xprintf(XVERBOSITY_LOG,
							"libmmsh: get_media_packet failed\n");
					return total;
				}
				else if (ptype == 2)
				{
					continue;
				}
				left = session->buf_size;
				if (left <= 0)
					continue;
				
				/* fixed size, skip session->package.packet_len */
				session->plast = session->packet_length;
				mms_get_stamp(session->buf, session->plast, &stamp);
				if (session->stamp < stamp)
					session->stamp = stamp;
				
				if (session->hobject && (session->package.chunk_length > 0))
				{
					n = mms_fix_pack(session->buf, session->plast,
						 session->package.chunk_length, NULL);
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
mmsh_close(mmsh_t *session)
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
  if (session)
    free (session);
}


extern uint32_t
mmsh_get_length(mmsh_t *session)
{
  return (uint32_t)session->file_length;
}

extern off_t
mmsh_get_current_pos(mmsh_t *session)
{
  return (off_t)session->current_pos;
}


extern mmsh_session_t *
mmsh_session_start(int fd, const char *mrl)
{
	mmsh_session_t *session = NULL;
	
	session = (mmsh_session_t *)mmsh_connect(fd, mrl, -1);
	return session;
}

extern int
mmsh_session_play(mmsh_session_t *session, uint32_t start, uint32_t length)
{
	return mmsh_play(session, start, length);
}

extern int
mmsh_session_read(mmsh_session_t *session, char *data, int len)
{
	return mmsh_read(session, data, len);
}

extern int
mmsh_session_seq(mmsh_session_t *session, unsigned int *seq)
{
	if ((NULL == session) || (session->package.chunk_length <= 0))
		return -ENODATA;
	if (seq != NULL)
		*seq = session->package.chunk_seq_number;
	return (int)session->package.chunk_seq_number;
}

extern int
mmsh_session_stamp(mmsh_session_t *session, unsigned int *stamp)
{
	if (session->stact <= 0)
		return -ENODATA;
	
	if (stamp != NULL)
		*stamp = session->stact;
	
	return (int)session->pnum;
}

extern void *
mmsh_session_package(mmsh_session_t *session)
{
	if (NULL == session)	/* ( || (session->package.length <= 0)) */
		return NULL;
	return (void *)&session->package;
}

extern int
mmsh_session_packouts(mmsh_session_t *session, uint32_t *num, uint32_t *size)
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
mmsh_session_header(mmsh_session_t *session)
{
	if ((NULL == session) || (NULL == session->hobject))
		return NULL;
	return (void *)session->hobject;
}

extern int
mmsh_session_fix_header(mmsh_session_t *session)
{
	return 0;
}

extern int
mmsh_session_got_header(mmsh_session_t *session, void **header)
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
mmsh_session_peek_header(mmsh_session_t *session, char *buf, int maxsize)
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
mmsh_session_end(mmsh_session_t *session)
{
	mmsh_close(session);
}


