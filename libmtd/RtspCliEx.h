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

#ifndef __RTSPCLIEX_H__
#define __RTSPCLIEX_H__

#define  STR_LEN         (1024 * 4)

namespace matrix {

class TRtspCliEx 
{
private:

	Uri * uri;
//	int m_sock;
//	bool UseProxy;
	
	int   m_hsio;       /* header input already */
	int   m_hsize;      /* header whole raw size */
	char  m_hbuffer[4096];
	
	uint32_t  m_rnums;
	uint32_t  m_rsizes;
	
	off_t  m_sbegin;
	off_t  m_slength;
	uint32_t  m_scurrent;
	off_t  m_smaxsize;

	rmff_header_t *m_header;
	rtsp_session_t *m_session;
	
public:

	TRtspCliEx(const string& sUrl);
	~TRtspCliEx();

//	int GetSock() { return m_sock; }
	
	bool Connect(void);
	void Disconnect (void);
	bool CheckBreak();
	
	int PostProcess(off_t *offset, off_t *length, void **param);
	int PreProcess(off_t offset, off_t length, void* param);
	int Recv(char* buffer, off_t nLen, const char * s, void* param = NULL);
	int Size(off_t *size);
	
	off_t GetHdrLen() { return m_hsize; }
	
	int Close();
	int	SendRequest(off_t offset, off_t length, off_t buflen, 
												char *response, int flag);
	

};

}
#endif
