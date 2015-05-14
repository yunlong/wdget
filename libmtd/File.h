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
#ifndef __FILE_H__
#define __FILE_H__

#define INVALID_FILE -1

namespace matrix {

typedef int fileobj_t;


class TFile 
{
private:
    fileobj_t  m_handle;
public:
    TFile( void );
    virtual ~TFile( void );

    virtual bool IsOpen( void );
    virtual void Close( void );
    bool Open( string& strFile );
    bool Stat( struct stat* pst );
    size_t Read( PVOID buf, size_t len );
    size_t Write( CPVOID buf, size_t len );

    bool Stat( string& strFile, struct stat* pst );

};

}

#endif 
