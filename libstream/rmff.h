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
 * $Id: rmff.h,v 1.5 2004/04/06 19:20:16 valtri Exp $
 *
 * some functions for real media file headers
 * adopted from joschkas real tools
 */

#ifndef HAVE_RMFF_H
#define HAVE_RMFF_H


#ifdef WIN32
#  include <winsock.h>
#else
#  include <unistd.h>
#  include <sys/time.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4200)
#endif


#define FOURCC_TAG( ch0, ch1, ch2, ch3 ) \
        (((long)(unsigned char)(ch3)       ) | \
        ( (long)(unsigned char)(ch2) << 8  ) | \
        ( (long)(unsigned char)(ch1) << 16 ) | \
        ( (long)(unsigned char)(ch0) << 24 ) )


#define RMF_TAG   FOURCC_TAG('.', 'R', 'M', 'F')
#define PROP_TAG  FOURCC_TAG('P', 'R', 'O', 'P')
#define MDPR_TAG  FOURCC_TAG('M', 'D', 'P', 'R')
#define CONT_TAG  FOURCC_TAG('C', 'O', 'N', 'T')
#define DATA_TAG  FOURCC_TAG('D', 'A', 'T', 'A')
#define INDX_TAG  FOURCC_TAG('I', 'N', 'D', 'X')
#define PNA_TAG   FOURCC_TAG('P', 'N', 'A',  0 )

#define MLTI_TAG  FOURCC_TAG('M', 'L', 'T', 'I')

/* prop flags */
#define PN_SAVE_ENABLED         0x01
#define PN_PERFECT_PLAY_ENABLED 0x02
#define PN_LIVE_BROADCAST       0x04


#pragma pack(push, 1)

/**
 * rm header data structs
 */
typedef struct {

  uint32_t object_id;
  uint32_t size;
  uint16_t object_version;

  uint32_t file_version;
  uint32_t num_headers;
} rmff_fileheader_t;

typedef struct {
  
  uint32_t object_id;
  uint32_t size;
  uint16_t object_version;
  
  uint32_t max_bit_rate;
  uint32_t avg_bit_rate;
  uint32_t max_packet_size;
  uint32_t avg_packet_size;
  uint32_t num_packets;
  uint32_t duration;
  uint32_t preroll;
  uint32_t index_offset;
  uint32_t data_offset;
  uint16_t num_streams;
  uint16_t flags;
  
} rmff_prop_t;

typedef struct {
  uint32_t  object_id;
  uint32_t  size;
  uint16_t  object_version;
  
  uint16_t  stream_number;
  uint32_t  max_bit_rate;
  uint32_t  avg_bit_rate;
  uint32_t  max_packet_size;
  uint32_t  avg_packet_size;
  uint32_t  start_time;
  uint32_t  preroll;
  uint32_t  duration;
  uint8_t   stream_name_size;
  char      *stream_name;
  uint8_t   mime_type_size;
  char      *mime_type;
  uint32_t  type_specific_len;
  char      *type_specific_data;

  int       mlti_data_size;
  char      *mlti_data;
  
  uint32_t  max_total_num;
  uint32_t  max_total_size;
  
} rmff_mdpr_t;

typedef struct {

  uint32_t  object_id;
  uint32_t  size;
  uint16_t  object_version;

  uint16_t  title_len;
  char      *title;
  uint16_t  author_len;
  char      *author;
  uint16_t  copyright_len;
  char      *copyright;
  uint16_t  comment_len;
  char      *comment;
  
} rmff_cont_t;

typedef struct {
  
  uint32_t object_id;
  uint32_t size;
  uint16_t object_version;

  uint32_t num_packets;
  uint32_t next_data_header; /* rarely used */
} rmff_data_t;

typedef struct {

  rmff_fileheader_t *fileheader;
  rmff_prop_t *prop;
  rmff_mdpr_t **streams;
  rmff_cont_t *cont;
  rmff_data_t *data;
} rmff_header_t;

typedef struct {

  uint16_t object_version;

  uint16_t length;
  uint16_t stream_number;
  uint32_t timestamp;
  uint8_t reserved;
  uint8_t flags;

  uint8_t datas[0];

} rmff_pheader_t;

typedef struct {
  
  uint32_t object_id;
  uint32_t size;
  uint16_t object_version;
  
  uint32_t entry_count;
  uint16_t stream_number;
  uint32_t next_offset;
  
  struct {
    uint16_t version;
    uint32_t stamp;
    uint32_t offset;
    uint32_t package;       /* package number */
  } entries[0];
  
} rmff_sindex_t;

typedef struct {
  
  int roffs;                /* raw io offset */
  int scount;               /* stream count */
  int smaxpkts;             /* package max count */
  
  int   *smaxnum;           /* stream index maxsize entry count */
  rmff_sindex_t **sindex;   /* stream index data buffer */
  
} rmff_stable_t;



#define RMFF_HEADER_SIZE        (0x12)

#define RMFF_SITEM_SIZE         (sizeof(uint32_t) * 3 + sizeof(uint16_t))
#define RMFF_SINDEX_SIZE(n)     (sizeof(rmff_sindex_t) + (n * RMFF_SITEM_SIZE))


#pragma pack(pop)


/*
 * constructors for header structs
 */
extern rmff_fileheader_t *
rmff_new_fileheader(uint32_t num_headers);

extern rmff_prop_t *
rmff_new_prop (
    uint32_t max_bit_rate,
    uint32_t avg_bit_rate,
    uint32_t max_packet_size,
    uint32_t avg_packet_size,
    uint32_t num_packets,
    uint32_t duration,
    uint32_t preroll,
    uint32_t index_offset,
    uint32_t data_offset,
    uint16_t num_streams,
    uint16_t flags );

extern rmff_mdpr_t *
rmff_new_mdpr(
    uint16_t   stream_number,
    uint32_t   max_bit_rate,
    uint32_t   avg_bit_rate,
    uint32_t   max_packet_size,
    uint32_t   avg_packet_size,
    uint32_t   start_time,
    uint32_t   preroll,
    uint32_t   duration,
    const char *stream_name,
    const char *mime_type,
    uint32_t   type_specific_len,
    const char *type_specific_data );

extern rmff_cont_t *
rmff_new_cont(
    const char *title,
    const char *author,
    const char *copyright,
    const char *comment);

extern rmff_data_t *
rmff_new_dataheader(
    uint32_t num_packets, uint32_t next_data_header);

extern rmff_header_t *
rmff_scan_header(const char *data);

extern int
rmff_scan_pheader(rmff_pheader_t *h, int size, char *data);

extern rmff_header_t *
rmff_scan_header_stream(int fd);

extern rmff_pheader_t *
rmff_scan_pheader_stream(int fd);

extern rmff_pheader_t *
rmff_new_pheader(uint32_t stream_number, uint32_t timestamp, uint32_t data_size);

extern void
rmff_print_header(rmff_header_t *h);

extern void
rmff_fix_header(rmff_header_t *h);

extern int
rmff_get_header_size(rmff_header_t *h);
 
extern int
rmff_dump_header(rmff_header_t *h, char *buffer, int maxs);

extern void
rmff_dump_pheader(rmff_pheader_t *h, char *data);

extern void
rmff_free_header(rmff_header_t *h);

extern void
rmff_free_pheader(rmff_pheader_t *h);

extern int
rmff_output_header(rmff_header_t *h, int maxs, char *buffer);

extern rmff_sindex_t *
rmff_new_sindex(int count);
extern void
rmff_free_sindex(rmff_sindex_t *s);

extern int
rmff_dump_sindex(rmff_sindex_t *s, int maxsize, char *data);
extern int
rmff_init_sindex(rmff_sindex_t *s, rmff_prop_t *p/* = NULL */);

extern int
rmff_resize_sindex(rmff_sindex_t **s, int maxitem);

extern rmff_stable_t *
rmff_new_stable(int count);
extern void
rmff_free_stable(rmff_stable_t *t);

extern int
rmff_dump_stable(int fd, int offset, rmff_stable_t *t);
extern int
rmff_scan_stable(int fd, int offset, rmff_stable_t *t);

extern int
rmff_resize_stable(int streamid, int maxsize, rmff_stable_t *t);

extern int
rmff_growin_stable(rmff_stable_t *t, uint32_t offset,
	 uint32_t package, uint32_t pcount, rmff_pheader_t *h);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef HAVE_RMFF_H */


