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

#define USE_URI_H
#define USE_UTILS_H
#define USE_MMSCLI_H

#include "matrix.h"

namespace matrix {

TMmsCliEx :: TMmsCliEx(const string& sUrl)
{
	uri = new Uri();
    uri->init(sUrl);
    if(uri->port() == 0 )  
        uri->port(MMS_PORT);

	IsMmsh = false;
	IsMmst	= true;
	
	m_hsio = 0;
	m_hsize = 0;
	
	m_rnums = 0;
	m_rsizes = 0;
	
	m_sbegin = 0;
	m_slength = 0;
	m_scurrent = 0;
	m_smaxsize = 0;
	
	m_sindex = 0;
	m_sthrow = 0;
	
	m_header = NULL;
	m_session = NULL;
}

TMmsCliEx :: ~TMmsCliEx()
{
	if(uri != NULL) delete uri;
	Disconnect();
}

void TMmsCliEx :: Disconnect (void)
{
}

bool TMmsCliEx::Connect()
{
	
	int Ref = 0;
	int msock = -1;	

/*
	"mmsh:"
	"mmst:"
	"mms:"
*/	
	IsMmst = true;

correct:
	Close();
		
	if(Ref > MAXRETRYCONN)
		return false;	
	
	if (IsMmst == true)
	{
		m_session = (void *)mmst_session_start(msock,  uri->unparse().c_str() );

		if (m_session == NULL)
		{
			Ref++;
			goto correct;
		}
		NSR_PROCESS_RETURN(NULL == m_session, false);
		
		m_header = (mmsf_header_t *)mmst_session_header((mmst_session_t *)m_session);
		goto getsize;
	}
	
	if(IsMmsh == true )
	{
		m_session = (void *)mmsh_session_start(msock,  uri->unparse().c_str() );
		
		if (m_session == NULL)
		{
			Ref++;
			goto correct;
		}
		NSR_PROCESS_RETURN(NULL == m_session, false);
		
		m_header = (mmsf_header_t *)mmsh_session_header((mmsh_session_t *)m_session);
	}
	
getsize:

	if (m_header == NULL)
		m_smaxsize = 0;
	else
	{
		m_smaxsize = m_header->prop.duration;
	}	
	
	return true;
}


int TMmsCliEx::Recv(char *buffer, off_t nLen, void* param)
{
	int ret = -EINVAL;
	int nBytes = 0;
	
	uint32_t seq;
	uint32_t nums;
	uint32_t sizes;
	
	mmsf_header_t *h = NULL;
	h = (mmsf_header_t *)m_header;

	NSR_PROCESS_ERROUT((! m_session) || (! m_header), -ENETRESET);
	
	if (m_hsio < m_hsize)
	{
		nBytes = (int)nLen;
		if (nBytes > (m_hsize - m_hsio))
			nBytes = (m_hsize - m_hsio);
		memcpy(buffer, m_hbuffer + m_hsio, nBytes);
		m_hsio += nBytes;
		goto okout;
	}
	
correct:
	seq = MAX_SIZE;
	
	if (IsMmsh == true)
	{
		ret = mmsh_session_read((mmsh_session_t *)m_session, buffer, nLen);
		NSR_PROCESS_ERROR(ret < 0);

		nBytes = ret;
		
		ret = mmsh_session_stamp((mmsh_session_t *)m_session, &m_scurrent);
		if (ret > 0)
		{
			if (m_scurrent < m_sbegin)
				m_scurrent = m_sbegin;
			m_slength = m_scurrent - m_sbegin;
		}
		ret = mmsh_session_packouts((mmsh_session_t *)m_session,
			 (uint32_t *)&nums, (uint32_t *)&sizes);
		if (ret >= 0)
		{
			m_rnums = nums;
			m_rsizes = sizes;
		}
	}
	else
	{
		ret = mmst_session_read( (mmst_session_t *)m_session, buffer, nLen);
		NSR_PROCESS_ERROR(ret < 0);
		nBytes = ret;
		
		ret = mmst_session_stamp((mmst_session_t *)m_session, &m_scurrent);
		if (ret > 0)
		{
			if (m_scurrent < m_sbegin)
				m_scurrent = m_sbegin;
			m_slength = m_scurrent - m_sbegin;
		}
		ret = mmst_session_packouts((mmst_session_t *)m_session, (uint32_t *)&nums, 
												 (uint32_t *)&sizes);
		if (ret >= 0)
		{
			m_rnums = nums;
			m_rsizes = sizes;
		}
	}
	
	if ((m_sthrow > 0) && (nBytes > 0))
	{
		if (m_sthrow >= (uint32_t)nBytes)
		{
			m_sthrow -= (uint32_t)nBytes;
			goto correct;
		}
		nBytes -= (int)m_sthrow;
		memmove(buffer, buffer + m_sthrow, nBytes);
		m_sthrow = 0;
	}
	
okout:
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

int TMmsCliEx::Close()
{
	m_rnums = 0;
	m_rsizes = 0;
	
	if (m_session != NULL)
	{
		if (IsMmsh == true)
			mmsh_session_end((mmsh_session_t *)m_session);
		else
			mmst_session_end((mmst_session_t *)m_session);
		m_session = NULL;
	}
	Disconnect();

	return 0;
}


int TMmsCliEx::SendRequest( off_t offset, off_t length,  off_t buflen, 
												char *response, int flag)
{
	int ret = -EINVAL;
	mmsf_header_t *h = NULL;
	
	m_hsio = 0;
	m_hsize = 0;
	
	NSR_PROCESS_ERROUT(NULL == m_session, -ENODATA);
	
	if (IsMmsh == true)
	{
		h = (mmsf_header_t *)mmsh_session_header((mmsh_session_t *)m_session);
		NSR_PROCESS_ERROUT(NULL == h, -ENETRESET);
		if (0 == m_smaxsize)
		{
			if ( NULL == h )
				m_smaxsize = MAX_SIZE;
			else
			{
				m_smaxsize = h->prop.duration;
			}
		}
		
		if (flag != 0)
		{
			if ((m_smaxsize > 0) && (offset + length >= m_smaxsize))
			{
				length = m_smaxsize - offset;
				if ((int)length <= 0)
					length = 0;
			}
			ret = mmsh_session_play( (mmsh_session_t *)m_session, offset, length);
			NSR_PROCESS_ERROR(ret < 0);
		}
		
		ret = mmsh_session_peek_header( (mmsh_session_t *)m_session, m_hbuffer, sizeof(m_hbuffer));
		NSR_PROCESS_ERROUT(ret <= 0, -EPROTO);
		m_hsio = 0;
		m_hsize = ret;
		goto okout;
	}
	
	h = (mmsf_header_t *)mmst_session_header((mmst_session_t *)m_session);
	NSR_PROCESS_ERROUT(NULL == h, -ENETRESET);
	
	if (0 == m_smaxsize)
	{
		
		if ((NULL == h) )
			m_smaxsize = MAX_SIZE;
		else
			m_smaxsize = h->prop.duration;
	}
	
	if (flag != 0)
	{
		if ((m_smaxsize > 0) && (offset + length >= m_smaxsize))
		{
			length = m_smaxsize - offset;
			if ((int)length <= 0)
				length = 0;
		}
		ret = mmst_session_play((mmst_session_t *)m_session, offset, length);
		NSR_PROCESS_ERROR(ret < 0);
	}
	
	ret = mmst_session_peek_header( (mmst_session_t *)m_session, m_hbuffer, sizeof(m_hbuffer));
	NSR_PROCESS_ERROUT(ret <= 0, -EPROTO);
	m_hsio = 0;
	m_hsize = ret;
	
okout:
	m_header = h;
	h = NULL;
	
errout:
	return ret;
}

int TMmsCliEx :: Size(off_t * size)
{
	int ret = -1;
	char response[MMS_BUF_SIZE];
	
	uint32_t fsize = 0;
	uint32_t hsize = 0;
	mmsf_header_t *h = NULL;
	
	ret = SendRequest(0, 0, MMS_BUF_SIZE, response, 0);
	NSR_PROCESS_ERROR(ret < 0);
	h = m_header;
	
	if (NULL == h)
		m_smaxsize = MAX_SIZE;
	else
	{
		m_smaxsize = h->prop.duration;
/*
 *		if ((h->prop.sendtime > 0) && (h->prop.sendtime < h->prop.duration))
 * 			m_smaxsize = h->prop.sendtime; 
 */
		fsize = (uint32_t)h->prop.fsize;
		hsize = (uint32_t)h->file.size;
	}
	
	if (size != NULL)
		*size = ((fsize > hsize) && (fsize != MAX_SIZE)) ? fsize : MAX_SIZE;
	
	ShowMsg("Mms File Size is %ul\n",  fsize);
	ret = 0;
	
errout:
	return ret;
}

int TMmsCliEx::PostProcess(off_t *offset, off_t *length, void **param)
{
	int ret = -EINVAL;
 	mmsf_header_t *h = NULL;
 	
 	NSR_PROCESS_ERROUT(NULL == m_session, 0);
 
 	if (IsMmsh == true)
 	{
 		ret = mmsh_session_fix_header((mmsh_session_t *)m_session);
 		NSR_PROCESS_ERROR(ret < 0);
 		NSR_PROCESS_ERROUT(ret <= 0, 0);
 
 		h = (mmsf_header_t *)mmsh_session_header((mmsh_session_t *)m_session);
 		NSR_PROCESS_ERROUT(NULL == h, -ENETRESET);
 		NSR_PROCESS_ERROUT((m_rnums <= 0) || (m_rsizes <= 0), 0);
 		
 		ret = mmsh_session_peek_header(
 			 (mmsh_session_t *)m_session, m_hbuffer, sizeof(m_hbuffer));
 		NSR_PROCESS_ERROUT(ret <= 0, -EPROTO);
 	}
 	else
 	{
 		ret = mmst_session_fix_header((mmst_session_t *)m_session);
 		NSR_PROCESS_ERROR(ret < 0);
 		NSR_PROCESS_ERROUT(ret <= 0, 0);
 		
 		h = (mmsf_header_t *)mmst_session_header((mmst_session_t *)m_session);
 		NSR_PROCESS_ERROUT(NULL == h, -ENETRESET);
 		NSR_PROCESS_ERROUT((m_rnums <= 0) || (m_rsizes <= 0), 0);
 		
 		ret = mmst_session_peek_header((mmst_session_t *)m_session, m_hbuffer, sizeof(m_hbuffer));
 		NSR_PROCESS_ERROUT(ret <= 0, -EPROTO);
 	}
 	
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
 		*length = (uint32_t)m_hsize;
 	if (offset != NULL)
 		*offset = 0;
 	ret = m_hsize;
 	
errout:
 	return ret;
}

int TMmsCliEx::PreProcess(off_t offset, off_t length, void *context)
{
	int ret = -1;
	char response[MMS_BUF_SIZE];
	
	mmsf_header_t *h = NULL;
	
	uint32_t sio = 0;
	uint32_t spos = 0;
	uint32_t sindex = MAX_SIZE;
	uint32_t sthrow = 0;
	uint32_t smaxsize = MAX_SIZE;
	
	uint32_t stamp = MAX_SIZE;
	uint64_t stotal = MAX_SIZE;
	
	if ((offset <= 0) && (length <= 0))
		goto tryout;
	if ((m_smaxsize <= 0) || (m_smaxsize == MAX_SIZE))
	{
		return -ECONNABORTED;
	}
	
	if (offset <= 0)
		m_sbegin = 0;
	
	/* correct the offset/length to millisecond unit(s) */
	NSR_PROCESS_ERROUT((! m_session) || (! m_header), -ENODATA);
	h = (mmsf_header_t *)m_header;
	
	if (offset < (uint32_t)(h->file.size + sizeof(h->data)))
		sio = offset;
	else
	{
		sio = MAX_SIZE;
		spos = offset - (uint32_t)(h->file.size + sizeof(h->data));
	}
	
	stotal = (uint32_t)(m_smaxsize / 10000);
	smaxsize = h->prop.packets;
	
	sindex = spos / h->prop.max_packet_size;
	sthrow = spos % h->prop.max_packet_size;
	
	stamp = (sindex * stotal / smaxsize);
	
/*	
 * offset = stamp;
 * length = 0;
 */
	if (IsMmsh == true)
	{
		/* nothing done */
	}
	else
	{
		offset = sindex;
		if (length > 0)
		{
			length += (h->prop.max_packet_size - 1);
			length /= h->prop.max_packet_size;
		}
	}
	
tryout:
	ret = SendRequest(offset, length, MMS_BUF_SIZE, response, 1);
	if (ret <= 0)
	{
		printf("thread:  mms->Request return: %d\n", ret);
		Close();
		if (0 == ret)
			ret = -ENODATA;
		goto errout;
	}
	m_hsio = 0;
	
	m_sbegin = offset;
	m_slength = 0;
	
	if ((MAX_SIZE == sio) || (sio > (uint32_t)m_hsize))
		m_hsio = m_hsize;
	else
		m_hsio = (int)sio;
	
	m_sindex = sindex;
	m_sthrow = sthrow;
	m_scurrent = stamp;
	
	ret = 0;
	
errout:
	return ret;
}

}

