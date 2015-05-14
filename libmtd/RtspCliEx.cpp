/*
 *  dget - download get for linux by yunlong.lee
 *  @author           YunLong.Lee    <yunlong.lee@163.com>
 *  @version          0.5v
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */

#define USE_UTILS_H
#define USE_URI_H
#define USE_RTSPCLI_H

#include "matrix.h"

namespace matrix {


TRtspCliEx::TRtspCliEx(const string& sUrl)
{
    uri = new Uri();
    uri->init(sUrl);
    if(uri->port() == 0 )   
    	uri->port(RTSP_PORT);
	
	m_hsio = 0;
	m_hsize = 0;
	
	m_rnums = 0;	//Current Packet Num
	m_rsizes = 0;	//all of Packet size received

	m_sbegin = 0; 	///start stamp
	m_slength = 0; 	///Record (m_scurrent - m_sbegin) Stamp
	m_scurrent = 0; ///current stamp
	m_smaxsize = 0; ///duration stamp
	
	m_header = NULL;
	m_session = NULL;
}

TRtspCliEx::~TRtspCliEx()
{
	if(uri != NULL) delete uri;
	Close();
}

bool TRtspCliEx::Connect(void)
{
	int Ref = 0;
	int ret;
	int msock;
	
correct:

	if(Ref > 5)
		return false;	
	m_session = rtsp_session_start(msock, uri->unparse().c_str() );
	if (m_session == NULL)
	{
		Ref++;
		goto correct;
	}
	
	NSR_PROCESS_RETURN(NULL == m_session, false);
	
	m_header = (rmff_header_t *)rtsp_session_header((rtsp_session_t *)m_session);
	
	if (m_header == NULL)
		m_smaxsize = 0;
	else
	{
		m_smaxsize = m_header->prop->duration;
	}
	
	return true;
}

void TRtspCliEx :: Disconnect (void)
{
}

bool TRtspCliEx::CheckBreak()
{
	return true;
}

int TRtspCliEx::Recv(char *buffer, off_t nLen, const char * s, void* param)
{
	int ret = -EINVAL;
	int nBytes = 0;
	
	char szBuffer[RTSP_BUF_SIZE] = { 0 };
		
	off_t nums;
	off_t sizes;
	
	NSR_PROCESS_ERROUT(NULL == m_session, -ENETRESET);
	
	ret = rtsp_session_read(m_session, buffer, nLen);
	NSR_PROCESS_ERROR(ret < 0);
	nBytes = ret;
	
	ret = rtsp_session_stamp(m_session, &m_scurrent);
	if (ret > 0)
	{
		if (m_scurrent < m_sbegin)
			m_scurrent = m_sbegin;
		m_slength = m_scurrent - m_sbegin;
	}
	
	ret = rtsp_session_packouts(m_session, (uint32_t *)&nums, (uint32_t *)&sizes);	
	if (ret >= 0)
	{
		m_rnums = nums;
		m_rsizes = sizes;
	}
	if (nBytes <= 0)
	{
		nums = 0;
		sizes = 0;
	}
	
	if (param != NULL)
		*((off_t *)param) = m_slength;
	ret = nBytes;
	
errout:
	return ret;

}

int TRtspCliEx::Close()
{
	m_rnums = 0;
	m_rsizes = 0;
	
	if (m_session != NULL)
	{
		rtsp_session_end(m_session);
		m_session = NULL;
	}

	Disconnect();
	return 0;
}

int TRtspCliEx::SendRequest(off_t offset, off_t length, off_t buflen,  
												char *response, int flag)
{
	int ret = -EINVAL;
	rmff_header_t *h = NULL;
	
	m_hsio = 0;
	m_hsize = 0;
	
	NSR_PROCESS_ERROUT(NULL == m_session, -ENODATA);
	
	h = (rmff_header_t *)rtsp_session_header(m_session);
	NSR_PROCESS_ERROUT(NULL == h, -ENETRESET);
	
	if (0 == m_smaxsize)
	{
		if ((NULL == h) || (NULL == h->prop))
			m_smaxsize = MAX_SIZE;
		else
			m_smaxsize = h->prop->duration;
	}
	
	if (flag != 0)
	{		
		if ((m_smaxsize > 0) && (offset + length >= m_smaxsize))
		{
			length = m_smaxsize - offset;
			if (length <= 0) 
				length = 0;
		}
		
		ret = rtsp_session_play(m_session, offset, length);
		NSR_PROCESS_ERROR(ret < 0);	
	}
	
	ret = rmff_output_header(h, sizeof(m_hbuffer), m_hbuffer);
	NSR_PROCESS_ERROUT(ret <= 0, -EPROTO);
	m_hsio = 0;
	m_hsize = ret;
	
	if (m_header != NULL)
	{
		//
	}
	m_header = h;
	h = NULL;

errout:
	return ret;
}

int TRtspCliEx::Size(off_t *size)
{
	int ret = -1;
	char response[RTSP_BUF_SIZE];
	
	off_t hdrstart, hdrlen;

	ret	= SendRequest(0, 0, RTSP_BUF_SIZE, response, 0);
	NSR_PROCESS_ERROR(ret < 0);
		
	if ((NULL == m_header) || (NULL == m_header->prop))
		m_smaxsize = MAX_SIZE;
	else
	{
		m_smaxsize = m_header->prop->duration;
	}
		
	if (size != NULL)
	{
//	*size = m_header->prop->num_packets * m_header->prop->avg_packet_size;
	*size = (off_t)m_header->prop->duration * m_header->prop->avg_bit_rate / 8000;
//	*size = GetHdrLen();
	}
	ret = 0;
	
errout:
	return ret;
}

int TRtspCliEx::PostProcess(off_t *offset, off_t *length, void **param)
{
	int ret = -EINVAL;
	rmff_header_t *h = NULL;
	
	NSR_PROCESS_ERROUT(NULL == m_session, 0);
	
	ret = rtsp_session_fix_header(m_session);
	NSR_PROCESS_ERROR(ret < 0);

	h = (rmff_header_t *)rtsp_session_header(m_session);
	NSR_PROCESS_ERROUT(NULL == h, -ENETRESET);
	NSR_PROCESS_ERROUT((m_rnums <= 0) || (m_rsizes <= 0), 0);
	
	ret = rmff_output_header(h, sizeof(m_hbuffer), m_hbuffer);
	NSR_PROCESS_ERROUT(ret <= 0, -EPROTO);
	
	if (m_hsize > 0)
	{
		if (ret != m_hsize)
		{
			m_hsize = ret;
		}
	}
	
	m_hsio = 0;
	m_hsize = ret;
	
	if (param != NULL)
		*param = m_hbuffer;
	if (length != NULL)
		*length = (off_t)m_hsize;
	if (offset != NULL)
		*offset = 0;
	
	ret = m_hsize;
errout:
	return ret;

}

int TRtspCliEx::PreProcess(off_t offset, off_t length, void *context)
{	
	int ret = -1;
	char response[RTSP_BUF_SIZE];
	
	rmff_header_t *h = m_header;
	
	/** correct the offset/length to millisecond unit(s) */	
	offset = offset * 8000 / m_header->prop->avg_bit_rate;
	length = length * 8000 / m_header->prop->avg_bit_rate;
	
	ret = SendRequest(offset, length, RTSP_BUF_SIZE, response, 1);
	if (ret <= 0)
	{
		printf("thread:  rtsp->Request return: %d\n", ret);
		Close();
		if (0 == ret)
			ret = -ENODATA;
		goto errout;
	}
	
	m_hsio = 0;
	m_sbegin = offset;
	m_slength = 0;

errout:
	return ret;
}

}
