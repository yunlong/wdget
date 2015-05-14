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


#ifndef __BUFFER_H__
#define __BUFFER_H__


namespace matrix {

class TBuffer
{
protected:
    size_t m_nAlloc;
    size_t m_nLen;
    PBYTE  m_buf;

public:
    TBuffer( void );
    TBuffer( const TBuffer& other );
    TBuffer( size_t len );
    TBuffer( CPBYTE pbuf, size_t len );
    ~TBuffer( void );

    TBuffer& operator=( const TBuffer& other );
    BYTE  operator[]( size_t n ) const;
    BYTE& operator[]( size_t n );

    void   Clear( void );

    void   Set( CPBYTE pbuf, size_t len );
    size_t GetSize( void ) const;
    void   SetSize( size_t len );
    PBYTE  GetBuffer( void );
    CPBYTE GetBuffer( void ) const;
    void   SetBuffer( CPBYTE pbuf );

};


class TString
{
protected:
    PCHAR   m_sz;
public:
    TString( void );
    TString( const TString& other );
    TString( CPCHAR sz );
    TString( CPCHAR buf, size_t len );
    TString( char c, UINT nrep = 1 );
    ~TString( void );

    TString& operator=( const TString& other );
    TString& operator=( CPCHAR sz );

    void  Set( CPCHAR buf, size_t len );

    UINT    GetLength( void ) const;
    bool    IsEmpty( void ) const;
    int     Compare( const TString& other ) const;
    int     CompareNoCase( const TString& other ) const;

    char    GetAt( UINT pos ) const;
    char&   GetAt( UINT pos );
    void    SetAt( UINT pos, char c );
    void    Append( CPCHAR sz );
    void    ToLower( void );
    void    ToUpper( void );
    void    DeQuote( void );

    CPCHAR Find( char c, UINT pos = 0 ) const;

    inline operator CPCHAR( void ) const 
	{ 
		return m_sz; 
	}
	
    inline char  operator[]( int pos ) const 
	{ 
		return GetAt( pos ); 
	}

};


inline bool operator==( const TString& lhs, const TString& rhs ) 
{ 
	return (lhs.Compare( rhs ) == 0); 
}

inline bool operator==( const TString& lhs, CPCHAR rhs )         
{ 
	return (lhs.Compare( rhs ) == 0); 
}

inline bool operator==( CPCHAR lhs, const TString& rhs )         
{ 
	return (rhs.Compare( lhs ) == 0); 
}

inline bool operator!=( const TString& lhs, const TString& rhs ) 
{ 
	return (lhs.Compare( rhs ) != 0); 
}

inline bool operator!=( const TString& lhs, CPCHAR rhs )         
{ 
	return (lhs.Compare( rhs ) != 0); 
}

inline bool operator!=( CPCHAR lhs, const TString& rhs )         
{ 
	return (rhs.Compare( lhs ) != 0); 
}

inline bool operator<=( const TString& lhs, const TString& rhs ) 
{ 
	return (lhs.Compare( rhs ) <= 0); 
}

inline bool operator<=( const TString& lhs, CPCHAR rhs )         
{ 
	return (lhs.Compare( rhs ) <= 0); 
}

inline bool operator<=( CPCHAR lhs, const TString& rhs )         
{ 
	return (rhs.Compare( lhs ) > 0); 
}

inline bool operator>=( const TString& lhs, const TString& rhs ) 
{ 
	return (lhs.Compare( rhs ) >= 0); 
}

inline bool operator>=( const TString& lhs, CPCHAR rhs )         
{ 
	return (lhs.Compare( rhs ) >= 0); 
}

inline bool operator>=( CPCHAR lhs, const TString& rhs )         
{ 
	return (rhs.Compare( lhs ) < 0); 
}

inline bool operator<( const TString& lhs, const TString& rhs ) 
{ 
	return (lhs.Compare( rhs ) < 0); 
}

inline bool operator<( const TString& lhs, CPCHAR rhs )         
{ 
	return (lhs.Compare( rhs ) < 0); 
}

inline bool operator<( CPCHAR lhs, const TString& rhs )         
{ 
	return (rhs.Compare( lhs ) >= 0); 
}

inline bool operator>( const TString& lhs, const TString& rhs ) 
{ 
	return (lhs.Compare( rhs ) > 0); 
}

inline bool operator>( const TString& lhs, CPCHAR rhs )         
{ 
	return (lhs.Compare( rhs ) > 0); 
}

inline bool operator>( CPCHAR lhs, const TString& rhs )         
{ 
	return (rhs.Compare( lhs ) <= 0); 
}

typedef std::list<TString> TStringList;

}

#endif 
