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
 * $Id: asfheader.h,v 1.3.2.1 2005/01/15 17:51:39 mroi Exp $
 *
 * demultiplexer for asf streams
 *
 * based on ffmpeg's
 * ASF compatible encoder and decoder.
 * Copyright (c) 2000, 2001 Gerard Lantau.
 *
 * GUID list from avifile
 * some other ideas from MPlayer
 */

#ifndef ASFHEADER_H
#define ASFHEADER_H


#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4200)
#endif

/*
 * define asf GUIDs (list from avifile)
 */
#define GUID_ERROR                              0

    /* base ASF objects */
#define GUID_ASF_HEADER                         1
#define GUID_ASF_DATA                           2
#define GUID_ASF_SIMPLE_INDEX                   3
#define GUID_INDEX                              4
#define GUID_MEDIA_OBJECT_INDEX                 5
#define GUID_TIMECODE_INDEX                     6

    /* header ASF objects */
#define GUID_ASF_FILE_PROPERTIES                7
#define GUID_ASF_STREAM_PROPERTIES              8
#define GUID_ASF_HEADER_EXTENSION               9
#define GUID_ASF_CODEC_LIST                    10
#define GUID_ASF_SCRIPT_COMMAND                11
#define GUID_ASF_MARKER                        12
#define GUID_ASF_BITRATE_MUTUAL_EXCLUSION      13
#define GUID_ASF_ERROR_CORRECTION              14
#define GUID_ASF_CONTENT_DESCRIPTION           15
#define GUID_ASF_EXTENDED_CONTENT_DESCRIPTION  16
#define GUID_ASF_STREAM_BITRATE_PROPERTIES     17
#define GUID_ASF_EXTENDED_CONTENT_ENCRYPTION   18
#define GUID_ASF_PADDING                       19
    
    /* stream properties object stream type */
#define GUID_ASF_AUDIO_MEDIA                   20
#define GUID_ASF_VIDEO_MEDIA                   21
#define GUID_ASF_COMMAND_MEDIA                 22
#define GUID_ASF_JFIF_MEDIA                    23
#define GUID_ASF_DEGRADABLE_JPEG_MEDIA         24
#define GUID_ASF_FILE_TRANSFER_MEDIA           25
#define GUID_ASF_BINARY_MEDIA                  26

    /* stream properties object error correction type */
#define GUID_ASF_NO_ERROR_CORRECTION           27
#define GUID_ASF_AUDIO_SPREAD                  28

    /* mutual exclusion object exlusion type */
#define GUID_ASF_MUTEX_BITRATE                 29
#define GUID_ASF_MUTEX_UKNOWN                  30

    /* header extension */
#define GUID_ASF_RESERVED_1                    31
    
    /* script command */
#define GUID_ASF_RESERVED_SCRIPT_COMMNAND      32

    /* marker object */
#define GUID_ASF_RESERVED_MARKER               33

    /* various */
/*
#define GUID_ASF_HEAD2                         27
*/
#define GUID_ASF_AUDIO_CONCEAL_NONE            34
#define GUID_ASF_CODEC_COMMENT1_HEADER         35
#define GUID_ASF_2_0_HEADER                    36

#define GUID_END                               37


/* asf stream types */
#define ASF_STREAM_TYPE_UNKNOWN           0
#define ASF_STREAM_TYPE_AUDIO             1
#define ASF_STREAM_TYPE_VIDEO             2
#define ASF_STREAM_TYPE_CONTROL           3
#define ASF_STREAM_TYPE_JFIF              4
#define ASF_STREAM_TYPE_DEGRADABLE_JPEG   5
#define ASF_STREAM_TYPE_FILE_TRANSFER     6
#define ASF_STREAM_TYPE_BINARY            7

#define ASF_MAX_NUM_STREAMS     23


#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID {          /* size is 16 */
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t  Data4[8];
} GUID;

#endif	/* #ifndef GUID_DEFINED */


#define MMS_STREAM_UNKNOWN      0
#define MMS_STREAM_LIVE         1
#define MMS_STREAM_SEEKABLE     2


/**
 * @see mmsf_prop_t.flags
 */
#define MMSF_FLAG_LIVE          0x000001	/* Broadcast Flag, for Live stream */
#define MMSF_FLAG_SEEKABLE      0x000002	/* Seekable Flag, for Persist stream */

/**
 * max sub-stream info of package to i/o
 */
#define MMSF_SUBS_MAXITEM       31

#define MMSF_SITEM_SIZE         (sizeof(uint32_t) + sizeof(uint16_t))
#define MMSF_SINDEX_SIZE(n)     (sizeof(mmsf_sindex_t) + (n * MMSF_SITEM_SIZE))

#define MMSF_SINDEX_TIME(n)     ((n) * 10000000)
#define MMSF_SINDEX_TINTERVAL   (MMSF_SINDEX_TIME(1))

#define MMSF_SHEADER_MINS       (2048)
#define MMSF_SHEADER_SIZE       (8192)
#define MMSF_SPACKET_MINS       (64)
#define MMSF_SPACKET_SIZE       (65536)     /* USHRT_MAX */
#define MMSF_SPACKET_STREAM     (4)


#pragma pack(push, 1)

/**
 * rm header data structs
 */
typedef struct mmsf_file_s
{
	GUID     gid;
	uint64_t size;
	
	uint32_t blocks;
	
	uint8_t  reserv1;           /* reserved, value 0x01 */
	uint8_t  reserv2;           /* reserved, value 0x02 */
	
} mmsf_file_t;

typedef struct mmsf_prop_s
{
	GUID     gid;
	uint64_t size;
	
	GUID     ident;             /* file global ident, temporary */
	
	uint64_t fsize;
	uint64_t fcreate;
	
	uint64_t packets;
	
	uint64_t duration;
	uint64_t sendtime;
	
	uint64_t preroll;
	uint32_t flags;
	
	uint32_t min_packet_size;
	uint32_t max_packet_size;
	
	uint32_t max_bit_rate;
	
} mmsf_prop_t;

typedef struct mmsf_data_s
{
	GUID     gid;
	uint64_t size;
	
	GUID     ident;             /* file id, same as mmsf_prop_t.ident */
	uint64_t packets;
	
	uint8_t  reserv1;           /* reserved, value 0x01 */
	uint8_t  reserv2;           /* reserved, value 0x01 */
	
} mmsf_data_t;


/**
 * asf/wmv summary information from header
 */
typedef struct mmsf_header_s
{
	int      type;
	int      size;      /* header size */
	
	int      vsize;     /* packet size */
	
	int      vfile;     /* offset of file */
	int      vprop;     /* offset of prop */
	int      vdata;     /* offset of data */
	int      vdend;     /* offset of data ending */
	
	mmsf_file_t file;
	mmsf_prop_t prop;
	mmsf_data_t data;
	
	uint8_t *heads;
	
} mmsf_header_t;

typedef struct mmsf_phinfo_s
{
	int   fid;          /* package fire stream id, -1 for not used */
	
	int   flag1;
	int   flag2;
	int   flags;        /* package segflag */
	int   fkeys;        /* key frame bit masks */
	int   fsubs;        /* sub stream valid bit masks */
	
	int   hlen;         /* header length */
	int   plen;         /* packet length info */
	int   ppad;         /* packet pad size */
	int   pseq;         /* packet sequence */
	int   psub;         /* packet subs count */
	
	uint32_t duration;  /* packet time duration, 100ms */
	uint32_t timestamp; /* packet time stamp offset, in ms */
	
	/** sub stream info */
	
	struct {
		
		short seqkey;   /* sub-stream keyframe flag for audio/video */
		short streamid; /* stream id type, is audio/video [1/2] */
		
		int   seqat;    /* sub-stream offset, in package */
		int   seqlen;   /* sub-stream length */
		int   seqtype;  /* sub-stream sequence type, 0x01/0x08/... */
		
		int   seqid;    /* sub-stream sequence id for audio/video */
		int   seqstamp; /* sub-stream sequence stamp/offset for audio/video */
		
	} subs[MMSF_SUBS_MAXITEM];
	
	int      rlen;      /* raw length */
	int      rsize;     /* raw maxsize */
	uint8_t  rdatas[0]; /* raw data buffer */
	
} mmsf_phinfo_t, mmsf_pheader_t;

typedef struct mmsf_sindex_s
{
	GUID     gid;
	uint64_t size;
	
	GUID     ident;             /* file id, same as mmsf_prop_t.ident */
	uint64_t interval;          /* 10000000(based 100-ns) for 1 second */
	
	uint32_t max_packet_count;  /* maximum package count of the entries */
	
	uint32_t entry_count;
	
	struct {
		uint32_t    start;      /* package start number */
		uint16_t    count;      /* package count of this item */
	} entries[0];
	
} mmsf_sindex_t;

typedef struct mmsf_stable_s
{
	int roffs;                /* raw io offset */
	int scount;               /* stream count */
	int smaxpkts;             /* package max count */
	
	int   *smaxnum;           /* stream index maxsize entry count */
	mmsf_sindex_t **sindex;   /* stream index data buffer */
	
} mmsf_stable_t;


#pragma pack(pop)


extern int
mms_get_guid(uint8_t *buffer, int offset);


extern int
mms_get_header(uint8_t *buffer, int maxsize, mmsf_header_t *header);
extern int
mms_got_header(uint8_t *buffer, int maxsize, mmsf_header_t **header);

extern mmsf_header_t *
mms_new_header(void);
extern void
mms_free_header(mmsf_header_t *h);

extern int
mms_dump_header(mmsf_header_t *h, char *buffer, int maxsize);


extern mmsf_header_t *
mms_scan_header_stream(int fd);

extern mmsf_phinfo_t *
mms_scan_pheader_stream(int fd, int maxsize);


extern int
mms_fix_pack(uint8_t *buffer, int maxsize,
	 int rawsize, mmsf_phinfo_t *phinfo);

extern int
mms_get_stamp(uint8_t *buffer, int maxsize, uint32_t *stamp);

extern int
mms_search_stamp(uint8_t *buffer, int maxsize);


extern mmsf_phinfo_t *
mms_new_phinfo(uint32_t data_size);
extern void
mms_free_phinfo(mmsf_phinfo_t *h);

extern int
mms_get_phinfo(uint8_t *buffer, int maxsize, mmsf_phinfo_t *phinfo);

extern int
mms_scan_phinfo(mmsf_phinfo_t *h);

extern mmsf_pheader_t *
mms_new_pheader(uint32_t data_size);
extern void
mms_free_pheader(mmsf_pheader_t *h);

extern int
mms_scan_pheader(mmsf_pheader_t *h, int size, char *data);

extern mmsf_sindex_t *
mms_new_sindex(int count);
extern void
mms_free_sindex(mmsf_sindex_t *s);

extern int
mms_dump_sindex(mmsf_sindex_t *s, int maxsize, char *data);
extern int
mms_init_sindex(mmsf_sindex_t *s, mmsf_prop_t *p/* = NULL */);

extern int
mms_resize_sindex(mmsf_sindex_t **s, int maxitem);

extern mmsf_stable_t *
mms_new_stable(int count);
extern void
mms_free_stable(mmsf_stable_t *t);

extern int
mms_dump_stable(int fd, int offset, mmsf_stable_t *t);
extern int
mms_scan_stable(int fd, int offset, mmsf_stable_t *t);

extern int
mms_resize_stable(int streamid, int maxsize, mmsf_stable_t *t);

extern int
mms_growin_stable(mmsf_stable_t *t, uint32_t offset,
	 uint32_t package, uint32_t count, mmsf_phinfo_t *h);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef ASFHEADER_H */


