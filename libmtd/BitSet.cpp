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

#define USE_BITSET_H
#include "matrix.h"

namespace matrix { 

/*
 *   n = ((n >>  1) & 0x55555555) | ((n <<  1) & 0xaaaaaaaa);
 *   n = ((n >>  2) & 0x33333333) | ((n <<  2) & 0xcccccccc);
 *   n = ((n >>  4) & 0x0f0f0f0f) | ((n <<  4) & 0xf0f0f0f0);
 *   n = ((n >>  8) & 0x00ff00ff) | ((n <<  8) & 0xff00ff00);
 *   n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000);
 *
 *       -- C code which reverses the bits in a word.
 *
 * Everyone should have 'fortune' in their login script.  :-)
 */

// A large portion of this is copied from CBuffer

#define WORD_SIZE(bits) ( ((bits+31)/32)*4 )
 
/***************** TBitSet************************/

TBitSet::TBitSet( void ) : m_nAlloc(0), m_nBitLen(0), m_buf(NULL)
{
    //
}

TBitSet::TBitSet( const TBitSet& other ) : m_nAlloc(0), 
										m_nBitLen(0),m_buf(NULL)
{
    if( other.m_nBitLen )
    {
        m_nAlloc = WORD_SIZE(other.m_nBitLen);
        m_nBitLen = other.m_nBitLen;
        m_buf = new BYTE[ m_nAlloc ];
        memcpy( m_buf, other.m_buf, m_nAlloc );
    }
}

TBitSet::TBitSet( CPBYTE pbuf, size_t bitlen )
{
    m_nAlloc = WORD_SIZE(bitlen);
    m_nBitLen = bitlen;
    m_buf = new BYTE[ m_nAlloc ];
    memcpy( m_buf, pbuf, m_nAlloc );
}

TBitSet::~TBitSet( void )
{
    delete[] m_buf;
}

TBitSet& TBitSet::operator=( const TBitSet& other )
{
    m_nAlloc = m_nBitLen = 0;
    delete[] m_buf; m_buf = NULL;
    if( other.m_nBitLen )
    {
        m_nAlloc = WORD_SIZE(other.m_nBitLen);
        m_nBitLen = other.m_nBitLen;
        m_buf = new BYTE[ m_nAlloc ];
        memcpy( m_buf, other.m_buf, m_nAlloc );
    }

    return *this;
}

size_t TBitSet::getBitSize( void ) const
{
    return m_nBitLen;
}

void TBitSet::setBitSize( size_t bitlen )
{
    if( bitlen )
    {
        size_t len = WORD_SIZE(bitlen);
        if( len > m_nAlloc )
        {
            BYTE* pbuf = new BYTE[ len ];
            memcpy( pbuf, m_buf, m_nAlloc );
            delete[] m_buf;
            m_nAlloc = len;
            m_buf = pbuf;
        }
        m_nBitLen = bitlen;
    }
    else
    {
        m_nAlloc = m_nBitLen = 0;
        delete[] m_buf; m_buf = NULL;
    }
}

void TBitSet::set( CPBYTE pbuf, size_t bitlen )
{
    if( bitlen )
    {
        size_t len = WORD_SIZE(bitlen);
        if( len > m_nAlloc )
        {
            delete[] m_buf;
            m_buf = new BYTE[ len ];
            m_nAlloc = len;
        }
        memcpy( m_buf, pbuf, len );
        m_nBitLen = bitlen;
    }
    else
    {
        m_nAlloc = m_nBitLen = 0;
        delete[] m_buf; m_buf = NULL;
    }
}

size_t TBitSet::getSize( void ) const
{
    return WORD_SIZE(m_nBitLen);
}

PBYTE TBitSet::getBuffer( void )
{
    return m_buf;
}

CPBYTE TBitSet::getBuffer( void ) const
{
    return m_buf;
}

size_t TBitSet::getSetCnt(void)
{
	size_t i = 0, cnt = 0 ;
	while( i < m_nBitLen )
	{
		if(testBit(i++))
			cnt++;
	}
	return cnt;
}

bool TBitSet :: IsEmpty(void)
{
	size_t i = 0;
	while( i < m_nBitLen )
	{
		if(testBit(i++)) 
			return false;
	}
	return true;
}

bool TBitSet :: IsAllSet(void)
{
	size_t i = 0;
	while( i < m_nBitLen )
	{
		if(testBit(i++)) 
			continue;
		else
			return false;
	}
	return true;
}

void TBitSet::zeroBits( void )
{
    if( m_nAlloc )
    {
        memset( m_buf, 0, m_nAlloc );
    }
}

void TBitSet::setBit( size_t n )
{
    m_buf[n/8] |= (1 << (n&7));
}

void TBitSet::clearBit( size_t n )
{
    m_buf[n/8] &= ~(1 << (n&7));
}

bool TBitSet::testBit( size_t n )
{
    return ( ( m_buf[n/8] & (1 << (n&7)) ) != 0 );
}

/******************* TRevBitSet********************/

void TRevBitSet::setBit( size_t n )
{
    m_buf[n/8] |= (1 << (7-n&7));
}

void TRevBitSet::clearBit( size_t n )
{
    m_buf[n/8] &= ~(1 << (7-n&7));
}

bool TRevBitSet::testBit( size_t n )
{
    return ( ( m_buf[n/8] & (1 << (7-n&7)) ) != 0 );
}

}

