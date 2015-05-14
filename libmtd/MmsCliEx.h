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

#ifndef __MMSTS_H__
#define __MMSTS_H__	

#define MAXRETRYCONN  5

namespace matrix {

class TMmsCliEx 
{
private:

	Uri * uri;
	
	int   m_hsio;       	/* header input already */
	int   m_hsize;      	/* header whole raw size */
	char  m_hbuffer[4096];
	
	bool IsMmsh;  			// true : mmsh://   
	bool IsMmst;   			// false: mmst://
	
	uint32_t  m_rnums;
	uint32_t  m_rsizes;
	
	uint32_t  m_sbegin;
	uint32_t  m_slength;
	uint32_t  m_scurrent;
	uint32_t  m_smaxsize;
	
	uint32_t  m_sindex;
	uint32_t  m_sthrow;
	
	mmsf_header_t *m_header;
	
	void *m_session;
	
public:
	TMmsCliEx(const string& sUrl);
	~TMmsCliEx();
	
	bool Connect(void);
	void Disconnect (void);
	
	int SendRequest(off_t offset, off_t length,	off_t buflen, 
											char *response, int flag);

	int PostProcess(off_t *offset, off_t *length, void **param);
	int PreProcess(off_t offset, off_t length, void* param);
	int Recv(char* buffer, off_t nLen, void* param = NULL);
	int Size(off_t *size);
	int Close();
};

}

#endif	


