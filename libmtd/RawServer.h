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

#ifndef __RAW_SERVER_H__
#define __RAW_SERVER_H__


#define MIN_SPARE_SOCKETS 256
#define MAX_SPARE_SOCKETS 1024

namespace matrix {

typedef std::list<TSocket*> TSocketList;

class TRawServer : public TThread
{
private:
	//////////////////////////////////
	int 			m_timeout;
	int 			m_interval;
	int 			m_maxconns;
	//////////////////////////////////	
	TSocketList 	m_sparesockets;
	//////////////////////////////////	
	TSocket* 		m_listensocket;
	UINT16			m_listenport;
	//////////////////////////////////
	TPoller* 		m_poller;
	TCrawler*		m_crawler;
protected:
    int         m_argc;
    char**      m_argv;
public:
    TRawServer( int argc, char** argv );
    TRawServer( void );
    ~TRawServer( void );

	void handleEvent(TPollEvent * pevents, int n);
	UINT16 getListenPort(void);

	void closeSocket(TSocket * skp);
	void cleanSpareSockets(void);

    void Run( void );
    bool Init( void );
    int  Exit( void );
    void Daemonize( void );
public:
	bool m_running;
	void start(void);
	void stop(void);


};
extern TRawServer* g_pApp;
}

#endif 
