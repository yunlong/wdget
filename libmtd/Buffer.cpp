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



#define USE_BUFFER_H
#define USE_DBG_H

#include "matrix.h"

namespace matrix {

/********** TBuffer **************/

TBuffer::TBuffer( void ) : m_nAlloc(0), m_nLen(0), m_buf(NULL)
{
    //
}

TBuffer::TBuffer(const TBuffer& other) : m_nAlloc(0),m_nLen(0),m_buf(NULL)
{
    if( other.m_nLen )
    {
        m_nAlloc = m_nLen = other.m_nLen;
        m_buf = new BYTE[ other.m_nLen ];
        memcpy( m_buf, other.m_buf, m_nAlloc );
    }
}

TBuffer::TBuffer( size_t len )
{
    m_buf = new BYTE[ len ];
    m_nAlloc = m_nLen = len;
}

TBuffer::TBuffer( CPBYTE pbuf, size_t len )
{
    m_buf = new BYTE[ len ];
    m_nAlloc = m_nLen = len;
    memcpy( m_buf, pbuf, len );
}

TBuffer::~TBuffer( void )
{
    if( m_buf ) memset( m_buf, 0xDD, m_nAlloc );
    delete[] m_buf;
}

TBuffer& TBuffer::operator=( const TBuffer& other )
{
    m_nAlloc = m_nLen = 0;
    delete[] m_buf; m_buf = NULL;
    if( other.m_nLen )
    {
        m_nAlloc = m_nLen = other.m_nLen;
        m_buf = new BYTE[ other.m_nLen ];
        memcpy( m_buf, other.m_buf, m_nAlloc );
    }

    return *this;
}

BYTE TBuffer::operator[]( size_t n ) const
{
    return m_buf[n];
}

BYTE& TBuffer::operator[]( size_t n )
{
    return m_buf[n];
}

void TBuffer::Clear( void )
{
    m_nLen = m_nAlloc = 0;
    delete[] m_buf; m_buf = NULL;
}

void TBuffer::Set( CPBYTE pbuf, size_t len )
{
    if( len )
    {
        if( len > m_nAlloc )
        {
            delete[] m_buf;
            m_buf = new BYTE[ len ];
            m_nAlloc = len;
        }
        memcpy( m_buf, pbuf, len );
        m_nLen = len;
    }
    else
    {
        m_nAlloc = m_nLen = 0;
        delete[] m_buf; m_buf = NULL;
    }
}

size_t TBuffer::GetSize( void ) const
{
    return m_nLen;
}

void TBuffer::SetSize( size_t len )
{
    if( len )
    {
        if( len > m_nAlloc )
        {
            BYTE* pbuf = new BYTE[ len ];
            memcpy( pbuf, m_buf, m_nAlloc );
            delete[] m_buf;
            m_nAlloc = len;
            m_buf = pbuf;
        }
        m_nLen = len;
    }
    else
    {
        m_nAlloc = m_nLen = 0;
        delete[] m_buf; m_buf = NULL;
    }
}

PBYTE TBuffer::GetBuffer( void )
{
    return m_buf;
}

CPBYTE TBuffer::GetBuffer( void ) const
{
    return m_buf;
}

void TBuffer::SetBuffer( CPBYTE pbuf )
{
    memcpy( m_buf, pbuf, m_nLen );
}

/*********************** TString ********************/

TString::TString( void )
{
    m_sz = new char[1];
    m_sz[0] = '\0';
}

TString::TString( const TString& other )
{
    m_sz = new char[ strlen(other.m_sz) + 1 ];
    strcpy( m_sz, other.m_sz );
}

TString::TString( CPCHAR sz )
{
    m_sz = new char[ strlen(sz) + 1 ];
    strcpy( m_sz, sz );
}

TString::TString( CPCHAR buf, size_t len )
{
    m_sz = new char[ len + 1 ];
    memcpy( m_sz, buf, len );
    m_sz[len] = '\0';
}

TString::TString( char c, UINT nrep )
{
    m_sz = new char[ nrep + 1 ];
    memset( m_sz, c, nrep );
    m_sz[nrep] = '\0';
}

TString::~TString( void )
{
    if( m_sz ) memset( m_sz, 0xDD, strlen(m_sz)+1 );
    delete[] m_sz;
}

TString& TString::operator=( const TString& other )
{
    delete[] m_sz;
    m_sz = new char[ strlen(other.m_sz) + 1 ];
    strcpy( m_sz, other.m_sz );

    return *this;
}

TString& TString::operator=( CPCHAR sz )
{
    delete[] m_sz;
    m_sz = new char[ strlen(sz) + 1 ];
    strcpy( m_sz, sz );

    return *this;
}

void TString::Set( CPCHAR buf, size_t len )
{
    delete[] m_sz;
    m_sz = new char[ len + 1 ];
    memcpy( m_sz, buf, len );
    m_sz[len] = '\0';
}

UINT TString::GetLength( void ) const
{
    return strlen( m_sz );
}

bool TString::IsEmpty( void ) const
{
    return ( m_sz[0] == '\0' );
}

int TString::Compare( const TString& other ) const
{
    return strcmp( m_sz, other.m_sz );
}

int TString::CompareNoCase( const TString& other ) const
{
    return strcasecmp( m_sz, other.m_sz );
}

char TString::GetAt( UINT pos ) const
{
    return m_sz[pos];
}

char& TString::GetAt( UINT pos )
{
    return m_sz[pos];
}

void TString::SetAt( UINT pos, char c )
{
    m_sz[pos] = c;
}

void TString::Append( CPCHAR sz )
{
    if( *sz )
    {
        PCHAR sznew = new char[ strlen(m_sz) + strlen(sz) + 1 ];
        strcpy( sznew, m_sz );
        strcat( sznew, sz );
        delete[] m_sz;
        m_sz = sznew;
    }
}

void TString::ToLower( void )
{
    char* p = m_sz;
    while( *p )
    {
        *p = tolower( *p );
        p++;
    }
}

void TString::ToUpper( void )
{
    assert_or_ret( m_sz );
    char* p = m_sz;
    while( *p )
    {
        *p = toupper( *p );
        p++;
    }
}

void TString::DeQuote( void )
{
    assert_or_ret( m_sz );

    PCHAR p = m_sz;
    PCHAR q = m_sz;
    while( *p )
    {
        if( *p == '\\' && *(p+1) )
        {
            switch( *(p+1) )
            {
            case 'n':   p++; *p = '\n'; break;
            case 'r':   p++; *p = '\r'; break;
            case 't':   p++; *p = '\t'; break;
            case '"':   p++; *p = '"';  break;
            case '\\':  p++;            break;
            default:
                break;
            }
        }
        *q++ = *p++;
    }
    *q = '\0';
}

CPCHAR TString::Find( char c, UINT pos ) const
{
    return strchr( m_sz+pos, c );
}

}

