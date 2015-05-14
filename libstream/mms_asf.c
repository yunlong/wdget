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

#include "xsysio.h"
#include "xsysutil.h"

#include "mms_asf.h"


typedef struct guid_item
{
	const char *name;
	const GUID  guid;
} guid_item;

static const guid_item guids[] =
{
    { "error",
    { 0x0,} },

    /* base ASF objects */
    { "header",
    { 0x75b22630, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c }} },

    { "data",
    { 0x75b22636, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c }} },

    { "simple index",
    { 0x33000890, 0xe5b1, 0x11cf, { 0x89, 0xf4, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xcb }} },

    { "index",
    { 0xd6e229d3, 0x35da, 0x11d1, { 0x90, 0x34, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xbe }} },

    { "media object index",
    { 0xfeb103f8, 0x12ad, 0x4c64, { 0x84, 0x0f, 0x2a, 0x1d, 0x2f, 0x7a, 0xd4, 0x8c }} },

    { "timecode index",
    { 0x3cb73fd0, 0x0c4a, 0x4803, { 0x95, 0x3d, 0xed, 0xf7, 0xb6, 0x22, 0x8f, 0x0c }} },

    /* header ASF objects */
    { "file properties",
    { 0x8cabdca1, 0xa947, 0x11cf, { 0x8e, 0xe4, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 }} },

    { "stream header",
    { 0xb7dc0791, 0xa9b7, 0x11cf, { 0x8e, 0xe6, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 }} },

    { "header extension",
    { 0x5fbf03b5, 0xa92e, 0x11cf, { 0x8e, 0xe3, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 }} },

    { "codec list",
    { 0x86d15240, 0x311d, 0x11d0, { 0xa3, 0xa4, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 }} },

    { "script command",
    { 0x1efb1a30, 0x0b62, 0x11d0, { 0xa3, 0x9b, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 }} },

    { "marker",
    { 0xf487cd01, 0xa951, 0x11cf, { 0x8e, 0xe6, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 }} },

    { "bitrate mutual exclusion",
    { 0xd6e229dc, 0x35da, 0x11d1, { 0x90, 0x34, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xbe }} },

    { "error correction",
    { 0x75b22635, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c }} },

    { "content description",
    { 0x75b22633, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c }} },

    { "extended content description",
    { 0xd2d0a440, 0xe307, 0x11d2, { 0x97, 0xf0, 0x00, 0xa0, 0xc9, 0x5e, 0xa8, 0x50 }} },

    { "stream bitrate properties", /* (http://get.to/sdp) */
    { 0x7bf875ce, 0x468d, 0x11d1, { 0x8d, 0x82, 0x00, 0x60, 0x97, 0xc9, 0xa2, 0xb2 }} },

    { "extended content encryption",
    { 0x298ae614, 0x2622, 0x4c17, { 0xb9, 0x35, 0xda, 0xe0, 0x7e, 0xe9, 0x28, 0x9c }} },

    { "padding",
    { 0x1806d474, 0xcadf, 0x4509, { 0xa4, 0xba, 0x9a, 0xab, 0xcb, 0x96, 0xaa, 0xe8 }} },

    /* stream properties object stream type */
    { "audio media",
    { 0xf8699e40, 0x5b4d, 0x11cf, { 0xa8, 0xfd, 0x00, 0x80, 0x5f, 0x5c, 0x44, 0x2b }} },

    { "video media",
    { 0xbc19efc0, 0x5b4d, 0x11cf, { 0xa8, 0xfd, 0x00, 0x80, 0x5f, 0x5c, 0x44, 0x2b }} },

    { "command media",
    { 0x59dacfc0, 0x59e6, 0x11d0, { 0xa3, 0xac, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 }} },

    { "JFIF media (JPEG)",
    { 0xb61be100, 0x5b4e, 0x11cf, { 0xa8, 0xfd, 0x00, 0x80, 0x5f, 0x5c, 0x44, 0x2b }} },

    { "Degradable JPEG media",
    { 0x35907de0, 0xe415, 0x11cf, { 0xa9, 0x17, 0x00, 0x80, 0x5f, 0x5c, 0x44, 0x2b }} },

    { "File Transfer media",
    { 0x91bd222c, 0xf21c, 0x497a, { 0x8b, 0x6d, 0x5a, 0xa8, 0x6b, 0xfc, 0x01, 0x85 }} },

    { "Binary media",
    { 0x3afb65e2, 0x47ef, 0x40f2, { 0xac, 0x2c, 0x70, 0xa9, 0x0d, 0x71, 0xd3, 0x43 }} },

    /* stream properties object error correction */
    { "no error correction",
    { 0x20fb5700, 0x5b55, 0x11cf, { 0xa8, 0xfd, 0x00, 0x80, 0x5f, 0x5c, 0x44, 0x2b }} },

    { "audio spread",
    { 0xbfc3cd50, 0x618f, 0x11cf, { 0x8b, 0xb2, 0x00, 0xaa, 0x00, 0xb4, 0xe2, 0x20 }} },


    /* mutual exclusion object exlusion type */
    { "mutex bitrate",
    { 0xd6e22a01, 0x35da, 0x11d1, { 0x90, 0x34, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xbe }} },

    { "mutex unknown", 
    { 0xd6e22a02, 0x35da, 0x11d1, { 0x90, 0x34, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xbe }} },

    /* header extension */
    { "reserved_1",
    { 0xabd3d211, 0xa9ba, 0x11cf, { 0x8e, 0xe6, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 }} },

    /* script command */
    { "reserved script command",
    { 0x4B1ACBE3, 0x100B, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 }} },

    /* marker object */
    { "reserved marker",
    { 0x4CFEDB20, 0x75F6, 0x11CF, { 0x9C, 0x0F, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB }} },

    /* various */
    /* Already defined (reserved_1)
    { "head2",
    { 0xabd3d211, 0xa9ba, 0x11cf, { 0x8e, 0xe6, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 }} },
    */
    { "audio conceal none",
    { 0x49f1a440, 0x4ece, 0x11d0, { 0xa3, 0xac, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 }} },

    { "codec comment1 header",
    { 0x86d15241, 0x311d, 0x11d0, { 0xa3, 0xa4, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 }} },

    { "asf 2.0 header",
    { 0xd6e229d1, 0x35da, 0x11d1, { 0x90, 0x34, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xbe }} },

};

extern int
mms_get_guid(unsigned char *buffer, int offset)
{
	int i;
	GUID g;
	
	g.Data1 = LE_32(buffer + offset);
	g.Data2 = LE_16(buffer + offset + 4);
	g.Data3 = LE_16(buffer + offset + 6);
	
	for (i = 0; i < 8; i++)
	{
		g.Data4[i] = buffer[offset + 8 + i];
	}
	
	for (i = 1; i < GUID_END; i++)
	{
		if (!memcmp(&g, &guids[i].guid, sizeof(GUID)))
		{
			lprintf ("GUID: %s\n", guids[i].name);
			return i;
		}
	}
	
	lprintf ("unknown GUID: 0x%x, 0x%x, 0x%x, "
		 "{ 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx }\n",
		 g.Data1, g.Data2, g.Data3,
		 g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], 
		 g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
	return GUID_ERROR;
}

extern int
mms_search_guid(uint8_t *buffer, int maxsize, int ident)
{
	uint8_t *sdata = NULL;
	uint32_t *search = NULL;
	
	if ((ident <= GUID_ERROR) || (ident >= GUID_END))
		return -EINVAL;
	search = (uint32_t *)&guids[ident].guid;
	
	for (sdata = buffer; maxsize > sizeof(GUID); maxsize--, sdata++)
	{
		if (*(uint32_t *)sdata != *search)
			continue;
		if (0 == memcmp(sdata, (void *)search, sizeof(GUID)))
			return (sdata - buffer);
	}
	
	return -ENODATA;
}

extern int
mms_get_header(uint8_t *buffer, int maxsize, mmsf_header_t *header)
{
	int ret = -EINVAL;
	int guid = -1;
	uint8_t *sdata = NULL;
	
	int size = 0;
	int offset = 0;
	
	if (maxsize < 2 * sizeof(GUID))
		return -ENOBUFS;
	
	offset = 0;
	guid = mms_get_guid(buffer, offset);
	if (guid != GUID_ASF_HEADER)
		return -EINVAL;
	ret = offset + sizeof(mmsf_file_t) + sizeof(mmsf_prop_t);
	if (maxsize < ret)
		return -ENOBUFS;
	size = LE_32(buffer + sizeof(GUID));
	if (maxsize < size)
		return -ENOBUFS;
	sdata = buffer + offset;
	header->vfile = offset;
	memcpy(&header->file, sdata, sizeof(header->file));
	
	ret = mms_search_guid(buffer + sizeof(header->file),
		 maxsize - sizeof(header->file), GUID_ASF_FILE_PROPERTIES);
	if (ret < 0)
		return -EINVAL;
	offset = ret + sizeof(header->file);
	ret = offset + sizeof(mmsf_prop_t);
	if (maxsize < ret)
		return -ENOBUFS;
	
	sdata = buffer + offset;
	header->vprop = offset;
	memcpy(&header->prop, sdata, sizeof(header->prop));
	
	if ((header->prop.max_packet_size != (uint32_t)(-1)) && 
		(header->prop.min_packet_size < header->prop.max_packet_size))
		header->vsize = header->prop.max_packet_size;
	else
		header->vsize = header->prop.min_packet_size;
	
	size = (int)header->file.size;
	if ((size + sizeof(header->data)) > maxsize)
		return -ENODATA;
	ret = mms_get_guid(buffer, size);
	if (ret != GUID_ASF_DATA)
		return -ENODATA;
	
	sdata = buffer + size;
	header->vdata = size;
	memcpy(&header->data, sdata, sizeof(header->data));
	
	header->vdend = size + (int)header->data.size;
	if (header->vdend < (size + sizeof(header->data)))
		header->vdend = -1;
	
	header->type = (header->prop.flags & MMSF_FLAG_LIVE) ? 	\
		 MMS_STREAM_LIVE : ((header->prop.flags & MMSF_FLAG_SEEKABLE) ? 	\
		 MMS_STREAM_SEEKABLE : MMS_STREAM_UNKNOWN);
	
	size += sizeof(mmsf_data_t);
	ret = (size <= 0) ? 0 : size;
	
	return ret;
}

extern int
mms_got_header(uint8_t *buffer, int maxsize, mmsf_header_t **header)
{
	int ret = -EINVAL;
	mmsf_header_t *h = mms_new_header();
	
	ret = mms_get_header(buffer, maxsize, h);
	if (ret < 0)
	{
		goto errout;
	}
	if (header != NULL)
	{
		*header = h;
		h = NULL;
	}
	
/** ret = 0;
 */
errout:
	if (h != NULL)
		mms_free_header(h);
	
	return ret;
}

extern mmsf_header_t *
mms_new_header(void)
{
	mmsf_header_t *header;
	header = (mmsf_header_t *)malloc(sizeof(mmsf_header_t));
	
	if (NULL == header)
		return NULL;
	
	memset(header, 0, sizeof(mmsf_header_t));
	header->type = MMS_STREAM_UNKNOWN;
	header->size = 0;
	header->heads = NULL;
	
	return header;
}

extern void
mms_free_header(mmsf_header_t *h)
{
	if (! h)
		return;
	
	if (h->heads != NULL)
	{
		free((void *)h->heads);
		h->heads = NULL;
	}
	h->size = 0;
	
	free((void *)h);
}

extern int
mms_dump_header(mmsf_header_t *h, char *buffer, int maxsize)
{
	int ret = -EINVAL;
	int size = 0;
	
	size = sizeof(mmsf_data_t) + 	\
		 sizeof(mmsf_file_t) + sizeof(mmsf_prop_t);
	if (h->size < size)
		return -ENODATA;
	if (maxsize < h->size)
		return -ENOBUFS;
	
	size = h->size;
	memcpy(buffer, h->heads, size);
	
	if (h->vfile >= 0)
		memcpy(buffer + h->vfile, &h->file, sizeof(mmsf_file_t));
	
	if ((h->vprop > 0) && (h->vprop + sizeof(mmsf_prop_t) <= size))
		memcpy(buffer + h->vprop, &h->prop, sizeof(mmsf_prop_t));
	if ((h->vdata > 0) && (h->vdata + sizeof(mmsf_data_t) <= size))
		memcpy(buffer + h->vdata, &h->data, sizeof(mmsf_data_t));
	
	ret = h->size;
	
	return ret;
}


extern mmsf_header_t *
mms_scan_header_stream(int fd)
{
	int ret = -EINVAL;
	int pos = -1;
	
	int maxs = 0;
	int size = 0;
	uint8_t *buffer = NULL;
	mmsf_header_t *header = NULL;
	
	ret = lseek(fd, 0, SEEK_CUR);
	if (ret < 0)
	{
		if (errno > 0)
			ret = -errno;
		goto errout;
	}
	pos = ret;
	
	maxs = MMSF_SHEADER_SIZE;
	buffer = (uint8_t *)malloc(maxs);
	if (NULL == buffer)
	{
		ret = -ENOMEM;
		goto errout;
	}
	
	ret = read(fd, buffer, maxs);
	if (ret <= 0)
		return NULL;
	size = ret;
	ret = mms_got_header(buffer, size, &header);
	if (ret < 0)
	{
		goto errout;
	}
	header->size = size;
	header->heads = buffer;
	buffer = NULL;
	
	pos += ret;
	header->size = ret;
	
	ret = lseek(fd, pos, SEEK_SET);
	if (ret < 0)
		goto errout;
	
	pos = -1;
	ret = header->size;
	
/**	ret = 0;
 */
errout:
	if (pos > 0)
	{
		pos = lseek(fd, pos, SEEK_SET);
	}
	if (ret < 0)
	{
		errno = ret;
/**		if (header != NULL)
			mms_free_header(header);
		header = NULL; */
	}
	
	if (buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
	}
	
	return header;
}

/**
 * @see rmff_scan_pheader_stream
 */
extern mmsf_phinfo_t *
mms_scan_pheader_stream(int fd, int maxsize)
{
	int ret = -EINVAL;
	mmsf_phinfo_t *h = NULL;
	
	if (maxsize < 0)
		maxsize = MMSF_SPACKET_MINS;
	
	h = mms_new_phinfo((uint32_t)maxsize);
	if (NULL == h)
		return h;
	
	ret = read(fd, h->rdatas, maxsize);
	if (ret <= 0)
	{
		if (0 == ret)
			errno = ENODATA;
		mms_free_phinfo(h);
		return NULL;
	}
	h->rlen = ret;
	
	ret = mms_scan_phinfo(h);
	
	return h;
}

static int
mms_fil_pack(int bufsize, uint8_t *buffer,
	 int flag1, int flag2, int segflag, int hlen,
	 int plen, int pseq, int ppad, int time, int duration)
{
	int ret = -EINVAL;
	int bits = 0;
	int size = 0;
	int flags = flag1;
	const uint8_t segtype = 0x5d;
	
	uint8_t *p = buffer;
	uint8_t *p_end = buffer + bufsize;
	uint8_t *p_flag = buffer + 0;
	uint8_t *p_pack = buffer + 0;
	
	if (plen < 0)
		plen = 0;
	if (ppad < 0)
		ppad = 0;
	
	if (bufsize < (4 * sizeof(uint64_t)))
	{
		ret = -ENOBUFS;
		goto errout;
	}
	p[0] = (uint8_t)flags;
	p[1] = (uint8_t)segtype;
	
	// skip ECC data if present by testing bit 7 of flags
	// 1xxxbbbb -> ecc data present, skip bbbb byte(s)
	// 0xxxxxxx -> payload parsing info starts
	if (flags & 0x80)
	{
		memset(p + 1, 0, (flags & 0x0f));
		p += (flags & 0x0f) + 1;
		
		flags = flag2;
		p[0] = (uint8_t)flags;
		p[1] = (uint8_t)segtype;
		p_flag = p;
	}
	
	//if (segtype != 0x5d) printf("Warning! packet[4] != 0x5d  \n");
	p += 2;	// skip flags & segtype
	
	bits = 0;
	flags &= (uint8_t)~(3 << 5);
	if (plen > USHRT_MAX)
		bits = 3;
	else if (plen > UCHAR_MAX)
		bits = 2;
	else if (plen > 0)
		bits = 1;
	flags |= (bits << 5);
	switch (bits)	// Write packet size (plen):
	{
	case 3: *(uint32_t *)p = LE_32(&plen); p += 4; break;	// dword
	case 2: *(uint16_t *)p = LE_16(&plen); p += 2; break;	// word
	case 1: *p = (uint8_t)plen; p++; break;			// byte
	default:
		break;
	}
	
	bits = 0;
	flags &= (uint8_t)~(3 << 1);
	if (pseq > USHRT_MAX)
		bits = 3;
	else if (pseq > UCHAR_MAX)
		bits = 2;
	else if (pseq > 0)
		bits = 1;
	flags |= (bits << 1);
	switch (bits)	// Write sequence:
	{
	case 3: *(uint32_t *)p = LE_32(&pseq); p += 4; break;	// dword
	case 2: *(uint16_t *)p = LE_16(&pseq); p += 2; break;	// word
	case 1: *p = (uint8_t)pseq; p++; break;			// byte
	default:
		break;
	}
	p_pack = p;
	
	bits = 0;
	flags &= (uint8_t)~(3 << 3);
	if (ppad > USHRT_MAX)
		bits = 3;
	else if (ppad > UCHAR_MAX)
		bits = 2;
	else if (ppad > 0)
		bits = 1;
	flags |= (bits << 3);
	switch (bits)	// Write padding size (padding):
	{
	case 3: *(uint32_t *)p = LE_32(&ppad); p += 4; break;	// dword
	case 2: *(uint16_t *)p = LE_16(&ppad); p += 2; break;	// word
	case 1: *p = (uint8_t)ppad; p++; break;			// byte
	default:
		break;
	}
	
	if ((p + sizeof(uint64_t)) > p_end)
	{
		ret = -ENOBUFS;
		goto errout;
	}
	
	// Write time & duration:
	*(uint32_t *)p = LE_32(&time); p+=4;
	*(uint16_t *)p = LE_16(&duration); p+=2;
	
	// Write payload flags:
	if (flags & 0x01)
	{
		*p = segflag;
		++p;
	}
	size = p - buffer;;
	
	if (hlen > size)
	{
		int pbits = 0;
		int psize = ppad + (hlen - size);
		
		if (psize > USHRT_MAX)
			pbits = 3;
		else if (psize > UCHAR_MAX)
			pbits = 2;
		else if (psize > 0)
			pbits = 1;
		if (pbits == bits)
		{
			switch (bits)	// Re-Write padding size (padding):
			{
			case 3: *(uint32_t *)p_pack = LE_32(&psize); break;	// dword
			case 2: *(uint16_t *)p_pack = LE_16(&psize); break;	// word
			case 1: *p_pack = (uint8_t)psize; break;			// byte
			default:
				break;
			}
		}
	}
	*p_flag = (uint8_t)flags;
	ret = size;
	
errout:
	return ret;
}


extern int
mms_fix_pack(uint8_t *buffer, int maxsize,
	 int rawsize, mmsf_phinfo_t *phinfo)
{
	int ret = -EINVAL;
	
	uint8_t *p = buffer;
	uint8_t *p_end = buffer + maxsize;
	uint8_t flags = p[0];
	uint8_t segflag = 0;
	uint8_t segtype = p[1];
	int plen = 0;
	int padding = 0;
	int keyfixed = 0;
	int keyframe = 0;
	int sequence = 0;
	uint32_t time = 0;
	uint32_t duration = 0;
	
	int i = 0;
	int seg = -1;
	int segs =1;
	int verbose = 0;
	uint8_t segsizetype = 0x80;
	
	if (0 == rawsize)
		rawsize = maxsize;
	if (maxsize <= 0)
	{
		ret = -ENODATA;
		goto errout;
	}
	if (verbose > 1)
	{
		for (i = 0; i < 16; i++)
			printf(" %02X", buffer[i]);
		printf("\n");
	}
	
	// skip ECC data if present by testing bit 7 of flags
	// 1xxxbbbb -> ecc data present, skip bbbb byte(s)
	// 0xxxxxxx -> payload parsing info starts
	if (flags & 0x80)
	{
		p += (flags & 0x0f) + 1;
		flags = p[0];
		segtype = p[1];
	}
	
	//if (segtype != 0x5d) printf("Warning! packet[4] != 0x5d  \n");
	p += 2;	// skip flags & segtype
	
	// Read packet size (plen):
	switch ((flags >> 5) & 3)
	{
	case 3: plen = LE_32(p); p += 4; break;	// dword
	case 2: plen = LE_16(p); p += 2; break;	// word
	case 1: plen = p[0]; p++; break;		// byte
	default:
		//plen==0 is handled later
		//mp_msg(MSGT_DEMUX,MSGL_V,"Invalid plen type! assuming plen=0\n");
		plen = 0;
		break;
	}
	
	// Read sequence:
	switch ((flags >> 1) & 3)
	{
	case 3: sequence = LE_32(p); p += 4; break;	// dword
	case 2: sequence = LE_16(p); p += 2; break;	// word
	case 1: sequence = p[0]; p++; break;		// byte
	default:
		sequence = 0;
		break;
	}
	// Read padding size (padding):
	switch ((flags >> 3) & 3)
	{
	case 3: padding = LE_32(p); p += 4; break;	// dword
	case 2: padding = LE_16(p); p += 2; break;	// word
	case 1: padding = p[0]; p++; break;			// byte
	default:
		padding = 0;
		break;
	}
	
	if (((flags >> 5) & 3) != 0)
	{
		// Explicit (absoulte) packet size
		// mp_dbg(MSGT_DEMUX,MSGL_DBG2,"Explicit packet size specified: %d  \n",plen);
		if (plen > maxsize)
		{
			//mp_msg(MSGT_DEMUX,MSGL_V,"Warning! plen>packetsize! (%d > %d)\n",plen,maxsize);
		}
	}
	else
	{
		// Padding (relative) size
		plen = maxsize - padding;
	}
	
	// Read time & duration:
	time = LE_32(p); p+=4;
	duration = LE_16(p); p+=2;
	
	// Read payload flags:
	if (flags & 0x01)
	{
		// multiple sub-packets
		segflag = p[0];
		++p;
		segs = segflag & 0x3F;
		segsizetype = segflag >> 6;
	}
/**	if (rawsize > 0)
	fprintf(stderr, "  flag 0x%02X, "\
		 "segs %d, seq %4d, len %4d, pad %2d, time %8d, dur %4d",
		 flags, segs, sequence, plen, padding, time, duration);
 */
	
	if (phinfo != NULL)
	{
		phinfo->flag1 = *buffer;
		phinfo->flag2 = flags;
		phinfo->flags = segflag;
		phinfo->fkeys = 0;
		phinfo->fsubs = 0;
		
		phinfo->hlen = p - buffer;
		phinfo->plen = plen;
		phinfo->ppad = padding;
		phinfo->pseq = sequence;
		phinfo->psub = segs;
		
		phinfo->duration = duration;
		phinfo->timestamp = time;
	}
	
	/* start correct stream from network to persist */
	if ((((flags >> 5) & 3) == 2) && 
		(padding <= 0) && ((maxsize - rawsize) > 0) && 
		(rawsize > 0) && ((maxsize - rawsize) <= SHRT_MAX))
	{
		int size = 0;
		uint8_t hbuffer[64] = { 0 };
		
		uint8_t *p_last = p;
		
		ret = mms_fil_pack(sizeof(hbuffer), hbuffer,
			 *buffer, flags, segflag, p - buffer, 0, sequence,
			 padding + (maxsize - rawsize), time, duration);
		if ((ret > 0) && (ret <= (p - buffer)))
			size = ret;
		
		if (size <= 0)
			p = p_last;
		else
		{
			memcpy(buffer, hbuffer, size);
			if ((p_last - buffer) > size)
			{
				memmove(buffer + size, p_last, p_end - p_last);
				p = buffer + size + (p_end - p_last);
				memset(p, 0, (p_last - buffer) - size);
			}
			p = buffer + size;
			keyfixed = size;
		}
	}
//	if (rawsize > 0)
//	fprintf(stderr, "%s\n", (keyfixed != 0) ? ", fixed" : "");
	
	for (seg = 0; seg < segs; seg++)
	{
		//ASF_segmhdr_t* sh;
		uint8_t streamno;
		uint32_t x;	// offset or timestamp
		uint32_t pos;
		uint32_t seq;
		uint32_t rlen;
		//
		int len;
		unsigned int time2 = 0;
		
		if (p >= p_end)
			return -ENODATA;
		
		if (verbose > 1)
		{
			printf("seg %d:", seg);
			for (i = 0; i < 16; i++)
				printf(" %02X", p[i]);
			printf("\n");
		}
		pos = p - buffer;
		streamno = (p[0] & 0x7F);
		keyframe = ((p[0] & 0x80) != 0);
		p++;
		
		// Read media object number (seq):
		switch ((segtype >> 4) & 3)
		{
		case 3: seq = LE_32(p); p += 4; break;	// dword
		case 2: seq = LE_16(p); p += 2; break;	// word
		case 1: seq = p[0]; p++; break;			// byte
		default:
			seq = 0;
			break;
		}
		
		// Read offset or timestamp:
		switch ((segtype >> 2) & 3)
		{
		case 3: x = LE_32(p); p += 4; break;	// dword
		case 2: x = LE_16(p); p += 2; break;	// word
		case 1: x = p[0]; p++; break;			// byte
		default:
			x = 0;
			break;
		}
		// Read replic.data len:
		switch ((segtype) & 3)
		{
		case 3: rlen = LE_32(p); p += 4; break;	// dword
		case 2: rlen = LE_16(p); p += 2; break;	// word
		case 1: rlen = p[0]; p++; break;		// byte
		default:
			rlen = 0;
			break;
		}
		// printf("### rlen=%d   \n",rlen);
		
		switch (rlen)
		{
		case 0x01:
			// 1 = special, means grouping
			//printf("grouping: %02X  \n",p[0]);
			++p;	// skip PTS delta
			break;
		default:
			if (rlen >= 8)
			{
				p += 4; // skip object size
				time2 = LE_32(p); // read PTS
				p += rlen - 4;
			}
			else
			{
				//mp_msg(MSGT_DEMUX,MSGL_V,"unknown segment type (rlen): 0x%02X  \n",rlen);
				time2 = 0; // unknown
				p += rlen;
			}
		}
		
		if (flags & 0x01)
		{
			// multiple segments
			switch (segsizetype)
			{
			case 3: len = LE_32(p); p += 4; break;	// dword
			case 2: len = LE_16(p); p += 2; break;	// word
			case 1: len = p[0]; p++; break;			// byte
			default:
				len = plen - (p-buffer); // ???
				break;
			}
		}
		else
		{
			// single segment
			len = plen - (p - buffer);
		}
		if ((len < 0) || ((p + len) > p_end))
		{
			//mp_msg(MSGT_DEMUX,MSGL_V,"ASF_parser: warning! segment len=%d\n",len);
		}
		//mp_dbg(MSGT_DEMUX,MSGL_DBG4,"  seg #%d: streamno=%d  seq=%d  type=%02X  len=%d\n",seg,streamno,seq,rlen,len);
/**		if (rawsize > 0)
		fprintf(stderr, "    seg# %2d: "\
			 "stream %2d, seq %4d, type %02X, len %4d, stamp %6u%s\n",
			 seg, streamno, seq, rlen, len, x,
			 (keyframe != 0) ? ", keyframe" : "");
 */
		
		if ((phinfo != NULL) && (seg < MMSF_SUBS_MAXITEM))
		{
			phinfo->subs[seg].seqkey = keyframe;
			phinfo->subs[seg].seqtype = rlen;
			phinfo->subs[seg].streamid = streamno;
			phinfo->subs[seg].seqat = (int)pos;
			phinfo->subs[seg].seqlen = (int)len;
			
			phinfo->subs[seg].seqid = seq;
			phinfo->subs[seg].seqstamp = x;
			
			if (keyframe != 0)
				phinfo->fkeys |= (1 << seg);
			phinfo->fsubs |= (1 << seg);
		}
		
		switch (rlen)
		{
		case 0x01:
			// GROUPING:
			//printf("ASF_parser: warning! grouping (flag=1) not yet supported!\n",len);
			//printf("  total: %d  \n",len);
			while (len > 0)
			{
				int len2 = p[0];
				p++;
				//printf("  group part: %d bytes\n",len2);
				//demux_asf_read_packet(demux,p,len2,streamno,seq,x,duration,-1,keyframe);
				p += len2;
				len -= len2+1;
				++seq;
			}
			if (len != 0)
			{
				//mp_msg(MSGT_DEMUX,MSGL_V,"ASF_parser: warning! groups total != len\n");
			}
			break;
		default:
			// NO GROUPING:
			//printf("fragment offset: %d  \n",sh->x);
			//demux_asf_read_packet(demux,p,len,streamno,seq,time2,duration,x,keyframe);
			p += len;
			break;
		}
	}
	
	ret = maxsize;
	
errout:
	return ret;
}

extern int
mms_get_stamp(uint8_t *buffer, int maxsize, uint32_t *stamp)
{
	int ret = -EINVAL;	
	int pos = -1;
	uint32_t msecond = 0;
	
	ret = mms_search_stamp(buffer, maxsize);
	if (ret < 0)
		goto errout;
	pos = ret;
	msecond = LE_32(buffer + pos);
	
	if (stamp != NULL)
		*stamp = msecond;
	ret = pos;
	
errout:
	return ret;
}

extern int
mms_search_stamp(uint8_t *buffer, int maxsize)
{
	int pos = -1;
	
	/* timecode position varies with type of chunk */
	if (maxsize <= (8 + sizeof(uint32_t)))
		return -ENOBUFS;
	
	switch (buffer[3])
	{
	case 0x00:
	case 0x01:
		pos =  5;
		break;
	case 0x08:
	case 0x09:
		pos =  6;
		break;
	case 0x10:
	case 0x11:
		pos =  7;
		break;
	case 0x48:
	case 0x49:
		pos =  8;
		break;
	case 0x50:
	case 0x51:
		pos =  9;
		break;
	default:
		pos = -1;
		break;
	}
	if (pos <= 0)
		return  -ENOENT;
	
	if ((pos + sizeof(uint32_t)) > maxsize)
		return -ENODATA;
	
	return pos;
}


extern mmsf_phinfo_t *
mms_new_phinfo(uint32_t data_size)
{
	int size = sizeof(mmsf_phinfo_t) + data_size;
	
	mmsf_phinfo_t *phinfo = (mmsf_phinfo_t *)malloc(size);
	
	memset(phinfo, 0, sizeof(mmsf_phinfo_t));
	phinfo->fid = -1;
	
	phinfo->rlen = 0;
	phinfo->rsize = (int)data_size;
	
	return phinfo;
}

extern void
mms_free_phinfo(mmsf_phinfo_t *h)
{
	if (!h) return;
	
	free(h);
}

extern int
mms_get_phinfo(uint8_t *buffer, int maxsize, mmsf_phinfo_t *phinfo)
{
	return mms_fix_pack(buffer, maxsize, -1, phinfo);
}

extern int
mms_scan_phinfo(mmsf_phinfo_t *h)
{
	if ((!h) || (h->rlen <= 0))
		return -ENODATA;
	
	return mms_fix_pack(h->rdatas, h->rlen, -1, h);
}

extern mmsf_pheader_t *
mms_new_pheader(uint32_t data_size)
{
	return mms_new_phinfo(data_size);
}

extern void
mms_free_pheader(mmsf_pheader_t *h)
{
	mms_free_phinfo(h);
	return;
}

extern int
mms_scan_pheader(mmsf_pheader_t *h, int size, char *data)
{
	return mms_fix_pack(data, size, -1, h);
}

extern mmsf_sindex_t *
mms_new_sindex(int count)
{
	int size = 0;
	mmsf_sindex_t *n = NULL;
	
	size = mms_resize_sindex(&n, count);
	if (NULL == n)
		return NULL;
	return n;
}

extern void
mms_free_sindex(mmsf_sindex_t *s)
{
	if (! s)
		return;
	free(s);
}

extern int
mms_dump_sindex(mmsf_sindex_t *s, int maxsize, char *data)
{
	int i = 0;
	int sio = 0;
	int size = 0;
	int count = 0;
	mmsf_sindex_t *w = NULL;
	
	int max_count = 0;
	
	size = sizeof(mmsf_sindex_t);
	if (s->entry_count > 0)
	{
		count = s->entry_count;
		size = MMSF_SINDEX_SIZE(count);
	}
	if (NULL == data)
		return size;
	else if (maxsize < size)
		return -ENOBUFS;
	
	memcpy(data, s, size);
	w = (mmsf_sindex_t *)data;
	
	w->size = LE_32(&s->size);
	w->interval = LE_64(&s->interval);
	
	w->entry_count = LE_32(&s->entry_count);
//	w->max_packet_count = LE_16(&s->max_packet_count);
//	w->next_offset = LE_32(&s->next_offset);
	
	for (i = 0; i < count; i++)
	{
		if (max_count < (int)s->entries[i].count)
			max_count = (int)s->entries[i].count;
		w->entries[i].start = LE_32(&s->entries[i].start);
		w->entries[i].count = LE_16(&s->entries[i].count);
//		w->entries[i].offset = BE_32(&s->entries[i].offset);
//		w->entries[i].package = BE_32(&s->entries[i].package);
		sio++;
	}
	if (s->max_packet_count < max_count)
		s->max_packet_count = max_count;
	w->max_packet_count = LE_16(&s->max_packet_count);
	
	size = sio;
	
	return size;
}

extern int
mms_init_sindex(mmsf_sindex_t *s, mmsf_prop_t *p/* = NULL */)
{
	memcpy(&s->gid, &guids[GUID_ASF_SIMPLE_INDEX].guid, sizeof(GUID));
	s->size = sizeof(mmsf_sindex_t);
	
	if (NULL == p)
		memset(&s->ident, 0, sizeof(GUID));
	else
		memcpy(&s->ident, &p->ident, sizeof(GUID));
	s->interval = MMSF_SINDEX_TINTERVAL;
	
	s->max_packet_count = 0;
	s->entry_count = 0;
	
	return 0;
}

extern int
mms_resize_sindex(mmsf_sindex_t **s, int maxitem)
{
	int size = 0;
	int count = 0;
	mmsf_sindex_t *m = *s;
	mmsf_sindex_t *n = NULL;
	
	size = sizeof(mmsf_sindex_t);
	if (maxitem > 0)
		size += maxitem * MMSF_SITEM_SIZE;
	if (m != NULL)
		count = m->entry_count;
	n = (mmsf_sindex_t *)realloc(m, size);
	if (NULL == n)
		return -ENOMEM;
	
	if (NULL == m)
	{
		memset(n, 0, size);
		
		mms_init_sindex(n, NULL);
	}
	else
	{
		size = maxitem - count;
		if (size < 0)
			size = 0;
		else
		{
			size *= MMSF_SITEM_SIZE;
			memset(&n->entries[count], 0, size);
		}
	}
	*s = n;
	return size;
}

extern mmsf_stable_t *
mms_new_stable(int count)
{
	int size = 0;
	mmsf_stable_t *n = NULL;
	
	n = (mmsf_stable_t *)malloc(sizeof(mmsf_stable_t));
	if (NULL == n)
		return n;
	memset(n, 0, sizeof(mmsf_stable_t));
	n->roffs = -1;
	n->scount = count;
	
	size = ((count + 1) * sizeof(int));
	n->smaxnum = (int *)malloc(size);
	if (NULL == n->smaxnum)
	{
		mms_free_stable(n);
		return NULL;
	}
	memset(n->smaxnum, 0, size);
	
	size = ((count + 1) * sizeof(mmsf_sindex_t *));
	n->sindex = (mmsf_sindex_t **)malloc(size);
	if (NULL == n->sindex)
	{
		mms_free_stable(n);
		return NULL;
	}
	memset(n->sindex, 0, size);
	
	return n;
}

extern void
mms_free_stable(mmsf_stable_t *t)
{
	int i;
	
	if (! t)
		return;
	t->roffs = -1;
	
	if (t->sindex != NULL)
	{
		for (i = 0; i < t->scount; t++)
		{
			mms_free_sindex(t->sindex[i]);
			t->sindex[i] = NULL;
			
			if (t->smaxnum != NULL)
				t->smaxnum[i] = 0;
		}
		free(t->sindex);
		t->sindex = NULL;
	}
	if (t->smaxnum != NULL)
	{
		free(t->smaxnum);
		t->smaxnum = NULL;
	}
	
	free(t);
}

extern int
mms_dump_stable(int fd, int offset, mmsf_stable_t *t)
{
	int ret = -EINVAL;
	int i = 0;
	int next = 0;
	int size = 0;
	
	int sio = 0;
	int smaxs = 0;
	uint8_t *strbufs = NULL;
	uint8_t *strdata = NULL;
	
	if (offset < 0)
		offset = t->roffs;
	if (offset < 0)
		ret = lseek(fd, 0, SEEK_CUR);
	else
		ret = lseek(fd, offset, SEEK_SET);
	if (ret < 0)
	{
		if (errno > 0)
			ret = -errno;
		goto errout;
	}
	t->roffs = ret;
	offset = t->roffs;
	
	next = offset;
	for (i = 0; i < t->scount; i++)
	{
		if (NULL == t->sindex[i])
			continue;
		size = MMSF_SINDEX_SIZE(t->sindex[i]->entry_count);
		
		if ((i + 1) >= t->scount)
			next = 0;
		else
			next += size;
//		t->sindex[i]->next_offset = next;
		if (smaxs < size)
			smaxs = size;
	}
	
	strbufs = malloc(smaxs + 0x40);
	if (NULL == strbufs)
	{
		ret = -ENOMEM;
		goto errout;
	}
	next = offset;
	strdata = strbufs;
	
	for (i = 0; i < t->scount; i++)
	{
		if (NULL == t->sindex[i])
			continue;
		size = MMSF_SINDEX_SIZE(t->sindex[i]->entry_count);
		
		t->sindex[i]->size = size;
		ret = mms_dump_sindex(t->sindex[i], smaxs, (char *)strdata);
		if (ret < t->sindex[i]->entry_count)
		{
			size = MMSF_SINDEX_SIZE(ret);
			if ((i + 1) >= t->scount)
				next = 0;
			else
				next += size;
//			t->sindex[i]->next_offset = next;
		}
		if (size > 0)
		{
			ret = write(fd, strdata, size);
			if (ret < 0)
			{
				if (errno > 0)
					ret = -errno;
				goto errout;
			}
			sio += ret;
		}
	}
	ret = sio;
	
errout:
	if (strbufs != NULL)
	{
		free(strbufs);
		strbufs = NULL;
		strdata = NULL;
	}
	
	return ret;
}

extern int
mms_scan_stable(int fd, int offset, mmsf_stable_t *t)
{
	return -ENOSYS;
}

extern int
mms_resize_stable(int streamid, int maxsize, mmsf_stable_t *t)
{
	int ret = -EINVAL;
	
	if (streamid < 0)
		streamid = 0;
	else if (streamid >= t->scount)
		return -ENOENT;
	
	if (t->smaxnum[streamid] > maxsize)
	{
		goto okout;
	}
	ret = mms_resize_sindex(&t->sindex[streamid], maxsize);
	if (ret < 0)
		goto errout;
	
	if (t->smaxnum != NULL)
		t->smaxnum[streamid] = maxsize;
	
okout:
	ret = 0;
	
errout:
	return ret;
}

extern int
mms_growin_stable(mmsf_stable_t *t, uint32_t offset,
	 uint32_t package, uint32_t pcount, mmsf_phinfo_t *h)
{
	int ret = -EINVAL;
	int count = -1;
	int index = -1;
	int maxsize = -1;
	int streamid = -1;
	int streamio = -1;
	
	streamid = h->fid;
	if (streamid < 0)
		streamid = 0;
/**	else if (streamid >= t->scount)
 *		return -ENOENT;
 *
 * MMS simple index table always has streamid 0 for stream 2(video)
 */
	streamio = streamid;
	streamid = 0;
	
	if ((t->sindex[streamid] != NULL) && 
		(t->sindex[streamid]->entry_count < t->smaxnum[streamid]))
	{
		/* already has buffer to save */
	}
	else
	{
		if ((t->smaxpkts > 0) && 
			(t->smaxnum[streamid] >= t->smaxpkts))
		{
			ret = ENOSPC;
			goto errout;
		}
		if ((t->sindex[streamid] != NULL) && 
			(t->sindex[streamid]->entry_count > 0))
		{
			count = t->sindex[streamid]->entry_count;
			maxsize = count / 3;
		}
		if (maxsize < 0x01000)
			maxsize = 0x01000;
		maxsize = (maxsize + 0x0FF) & 0x7FFFFF00;
		maxsize += (count > 0) ? count : 0;
		
		ret = mms_resize_stable(streamid, maxsize, t);
		if (ret < 0)
			goto errout;
		if (count <= 0)
		{
			t->sindex[streamid]->entry_count = 0;
//			t->sindex[streamid]->stream_number = streamid;
		}
	}
	index = t->sindex[streamid]->entry_count;
	
	t->sindex[streamid]->entries[index].start = package;
	t->sindex[streamid]->entries[index].count = pcount;
	
	if (t->sindex[streamid]->max_packet_count < pcount)
		t->sindex[streamid]->max_packet_count = pcount;
	
/**	fprintf(stderr, "%s stamp: %4u.%03u, offset: %8u, size: %4u, seq: %4u\n",
		 (streamio > 1) ? "video" : "audio",
		 h->timestamp/1000, h->timestamp%1000, offset, h->plen, package);
 */
	t->sindex[streamid]->entry_count++;
	ret = index;
	
errout:
	return ret;
}


