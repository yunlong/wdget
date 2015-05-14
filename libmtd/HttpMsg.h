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

#ifndef __HTTP_MSG_H__
#define __HTTP_MSG_H__

namespace matrix {

class THttpHdr
{
private:
	string m_strKey;
	string m_strVal;
public:
	THttpHdr( void );
	THttpHdr( const string& strKey );
	THttpHdr( const string& strKey, const string& strVal );

	const string&  GetKey( void ) const;
	const string&  GetVal( void ) const;
	void  SetVal( const string& strVal );

};

typedef std::list<THttpHdr*> THttpHdrList;

class THttpMsg
{
private:
	unsigned int   m_nHttpVer;    // HTTP version (hiword.loword)
	THttpHdrList    m_listHdrs;
public:
	virtual ~THttpMsg( void );
	THttpMsg( void );
	THttpMsg( THttpMsg& other );

	// Total header length for key/val pairs (incl. ": " and CRLF)
	// but NOT separator CRLF
	size_t GetAllHdrLen( void ) ; 
	size_t GetHdrLen( unsigned int nIndex ) ; 
	void ClearAllHdr(void);

	void GetHttpVer( unsigned int* puMajor, unsigned int* puMinor ) const;
	void SetHttpVer( unsigned int uMajor, unsigned int uMinor );

	size_t GetHdrCount( void ) const;
	string   GetHdr( const string& strKey ) ; 
	THttpHdr* GetHdr( unsigned int nIndex ) ; 
	void      SetHdr( const string& strKey, const string& strVal );
	void      SetHdr( const THttpHdr& hdrNew );
	void    ShowAllHttpHdr(void);

	THttpHdrList& GetAllHdr(void) { return m_listHdrs; }

};

}

#endif
