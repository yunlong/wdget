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

#ifndef __BITSET_H__
#define __BITSET_H__


namespace matrix {
/*
 * Simple bitset class.  Bits are numbered in ascending order such that
 * bit N is at byte N/8, value 2^(N%8).
 *
 *   Example: L=14, { 0x04, 0x02 } = 00000100 xx000010 = ( 2, 9 )
 *                                   |      |   |    |
 *                                   7      0  13    8
 */
class TBitSet
{
protected:
    size_t m_nAlloc;
    size_t m_nBitLen;
    PBYTE  m_buf;
public:
    TBitSet( void );
    TBitSet( const TBitSet& other );
    TBitSet( CPBYTE pbuf, size_t bitlen );
    ~TBitSet( void );
    
	TBitSet& operator=( const TBitSet& other );

    // Bit size operations
    size_t  getBitSize( void ) const;
    void    setBitSize( size_t bitlen );
    void    set( CPBYTE pbuf, size_t bitlen );

    // Byte size operations
    size_t  getSize( void ) const;
	
    PBYTE   getBuffer( void );
    CPBYTE  getBuffer( void ) const;
	
	size_t	getSetCnt(void);
	bool	IsEmpty(void);
	bool 	IsAllSet(void);

    // Bit manipulation
    void    zeroBits( void );
    void    setBit( size_t n );
    void    clearBit( size_t n );
    bool    testBit( size_t n );

};

typedef std::list<TBitSet*> TBitSetList;
/*
 * Reversed bitset class. Bytes are numbered in ascending order 
 * but the bit positions are reversed such that bit N is at byte N/8, 
 * value 2^(7-N%8).
 *
 *   Example: L=14, { 0x20, 0x40 } = 00100000 010000xx = ( 2, 9 )
 *                                   |      | |    |
 *                                   0      7 8   13
 */

class TRevBitSet : public TBitSet
{
public:
    // Bit manipulation
    void    setBit( size_t n );
    void    clearBit( size_t n );
    bool    testBit( size_t n );
};
typedef std::list<TRevBitSet*> TRevBitSetList;

}

#endif 
