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

#define USE_FILE_H
#include "matrix.h"

namespace matrix {

TFile::TFile( void ) : m_handle(INVALID_FILE)
{
    // Empty
}

TFile::~TFile( void )
{
    if( IsOpen() ) Close();
}

bool TFile::IsOpen( void )
{
    return ( INVALID_FILE != m_handle );
}

void TFile::Close( void )
{
    close( m_handle );
    m_handle = INVALID_FILE;
}

bool TFile::Open( string& strFile )
{
    m_handle = open( strFile.c_str(), O_NONBLOCK );
    return IsOpen();
}

bool TFile::Stat( struct stat* pst )
{
    return ( 0 == fstat( (int)m_handle, pst ) );
}

size_t TFile::Read( PVOID pbuf, size_t len )
{
    ssize_t nRead = read( m_handle, pbuf, len );
    if( nRead < 0 )
    {
        Close();
        nRead = 0;
    }
    return nRead;
}

size_t TFile::Write( CPVOID pbuf, size_t len )
{
    ssize_t nWritten = write( m_handle, pbuf, len );
    if( nWritten < 0 )
    {
        Close();
        nWritten = 0;
    }
    return nWritten;
}

bool TFile::Stat( string& strFile, struct stat* pst )
{
    return ( 0 == stat( strFile.c_str(), pst ) );
}

}

