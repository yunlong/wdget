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

#define USE_THREAD_H
#define USE_HTTPMSG_H
#define USE_SOCKET_H
#define USE_POLLER_H
#define USE_THREAD_H
#define USE_URL_H
#define USE_HTTPCLI_H
#define USE_HASHTABLE_H
#define USE_CRAWLER_H
#define USE_RAWSERVER_H
#define USE_DBG_H
#define USE_LOGGER_H
#include "matrix.h"

namespace matrix {

extern pthread_key_t g_keyself;
TRawServer* g_pApp = NULL;

/******************** TRawServer ************************/
TRawServer::TRawServer( int argc, char** argv ) : m_argc(argc), m_argv(argv)
{
	g_pApp = this;
#ifdef _UNIX
    pthread_key_create( &g_keyself, NULL );
    pthread_setspecific( g_keyself, this );
#endif
	m_running = false;
	m_crawler = new TCrawler();
}

TRawServer::TRawServer( void ) : m_maxconns(1024)
{
	g_pApp = this;
#ifdef _UNIX
    pthread_key_create( &g_keyself, NULL );
    pthread_setspecific( g_keyself, this );
#endif
	m_running = false;
	m_crawler = new TCrawler();
}

TRawServer::~TRawServer( void )
{
#ifdef _UNIX
    pthread_key_delete( g_keyself );
#endif

	if(m_poller != NULL) delete m_poller;
	m_listensocket->closeSocket();

	cleanSpareSockets();
	Logger::closeLog();
	delete m_crawler;
}

void TRawServer :: handleEvent(TPollEvent * pevents, int n)
{
	for( int i = 0; i < n; i++ )
	{
		int err = SOCKERR_NONE;	

		TSocket * skp = (TSocket *)(pevents[i].data.ptr);
		UINT32 SelectFlags = skp->getSelectFlags();
		waitevents_t wevt = pevents[i].events;

		if( (wevt & (EPOLLIN|EPOLLERR)) && (SelectFlags & SF_ACCEPT) == SF_ACCEPT)
		{
			wevt = XPOLLACC;
		}

		if( (wevt & (EPOLLOUT|EPOLLERR)) &&	(SelectFlags & SF_CONNECT) == SF_CONNECT)
		{
			socklen_t errlen = sizeof(err);
			getsockopt(skp->getHandle(), SOL_SOCKET, SO_ERROR, &err, &errlen);
			wevt = XPOLLCNX;
		}

		if(wevt & EPOLLERR)
		{
			wevt = SelectFlags;
		}

		if( WAIT_EVENT_READ( wevt ) )
		{
			cout << "WAIT EVENT READ" << endl;
			skp->handleRead();
			skp->setMaskRead(false);
			skp->setMaskWrite(true);
			m_poller->modSocket(skp);
		}
		else if( WAIT_EVENT_WRITE( wevt ) )
		{
			cout << "WAIT EVENT WRITE" << endl;
			skp->handleWrite();
		//	pevents[i].data.ptr = NULL;
			m_poller->delSocket(skp);
		//	delete skp;

			if( m_sparesockets.size() >= MAX_SPARE_SOCKETS )
			{
				delete skp;
			}
			else
			{	
				skp->closeSocket();
				skp->setMaskRead(false);
				skp->setMaskWrite(false);
				m_sparesockets.push_back(skp);
			}
			
		}
		else if( WAIT_EVENT_ACCEPT( wevt ) )
		{
			cout << "WAIT EVENT ACCEPT" << endl;
			string ipaddr; UINT16 port;
			sockobj_t sock = dynamic_cast<ListenSocket*>(m_listensocket)->Accept(ipaddr, port);
			if(INVALID_SOCKET != sock )
			{
				TSocket *skp = NULL;
				
				if( !m_sparesockets.empty() )
				{   
					skp = m_sparesockets.front();
					m_sparesockets.pop_front();
				} 
				else
				{
					skp = new TcpSocket();
				}
				
				skp->attach(sock);
				skp->setMaskRead(true);
				m_poller->addSocket(skp);
				LOG(1)("connection from %s:%d\n", ipaddr.c_str(), port);
				cout <<"connection from " << ipaddr << ":"<< port << endl;
			} 
		}
		else if( WAIT_EVENT_CONNECT( wevt ) )
		{
			cout << "WAIT EVENT CONNECT" << endl;
		}
		else if( WAIT_EVENT_EXCEPT( wevt ) )
		{
			cout << "WAIT EVENT EXCEPT" << endl;
		}
	}
}

void TRawServer::Run( void )
{
    srand( time(&curTime) );
    Logger::open("/var/log/crowdspider.log", 1);

	LOG(1)("crowdspider is now started.\n");

    if( Init() )
    {
        bool running = true;
        while( running )
        {
            int rc = m_poller->poll(200);

            if( rc < 0 )
            {
                dbgout("poll failed:error=%i (%s)",errno,strerror(errno));
                break;
            }

            if( rc == 0 ) {}

            if( rc > 0 )
            {
				TPollEvent * pevents = m_poller->getPollEvents();
				handleEvent(pevents, rc);
            }
        }
    }

    Exit();
	LOG(1)("crowdspider is now stoped.\n");
}

bool TRawServer::Init( void )
{
	m_poller = new TPoller;
	m_poller->start();
	
	for( int i = 0; i < MIN_SPARE_SOCKETS; i++ )
	{
		TSocket * skp = new TcpSocket();
		m_sparesockets.push_back(skp); 
	}

	m_listensocket = new ListenSocket();
	UINT16 port = 9080;
	for( ; port < 9089; port++)
	{
		if( dynamic_cast<ListenSocket*>(m_listensocket)->Listen(port) == true)
		{       
			m_listenport = port;
			m_listensocket->setMaskRead(true);
			m_poller->addSocket(m_listensocket);
			break;
		}
	}

	if(port == 9089 ) 
	{
		LOG(1)( "no listen port is availble !!!\n" );
		m_listensocket->closeSocket();
		delete m_listensocket;
		return false;
	}

	LOG(1)("Listening on port %hu\n", m_listenport );
        
//    Daemonize();

    return true;
}

int TRawServer::Exit( void )
{
	pthread_cancel(m_thread);
	m_crawler->stop();
    return 0;
}

UINT16 TRawServer :: getListenPort(void)
{
	return m_listenport;
}

void TRawServer :: closeSocket(TSocket * skp)
{
	m_poller->delSocket(skp);
	skp->closeSocket();
	delete skp;
}

void TRawServer::cleanSpareSockets(void)
{
	while(!m_sparesockets.empty())
	{   
		TSocket* skp = m_sparesockets.front();
		delete skp;
		m_sparesockets.pop_front();
	}   
}

/* Become process lead and detach from tty Code lifted from fetchmail 5.0.0 */
void TRawServer::Daemonize( void )
{
  /* Ignore BSD terminal stop signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    /* Change to root dir so we don't hold any mount points open */
    if( chdir( "/" ) !=  0 )
    {
        dbgout( "chdir failed (%s)", strerror(errno) );
    }

    /* 
	 * In case we were not started in the background, fork and let the parent exit.  
	 * Guarantees that the child is not a process group leader 
	 */
    pid_t childpid;
    if( (childpid = fork()) < 0 )
    {
        dbgout( "fork failed (%s)", strerror(errno) );
        return;
    }
    else if( childpid > 0 )
    {
        exit( 0 );  /* parent */
    }


    /* 
	 * Make ourselves the leader of a new process group with no controlling terminal 
     * POSIX makes this soooo easy to do 
	 */
    if( setsid() < 0 )
    {
        dbgout( "setsid failed (%s)", strerror(errno) );
        return;
    }

    /* lose controlling tty */
    signal( SIGHUP, SIG_IGN );
    if( (childpid = fork()) < 0 )
    {
        dbgout( "fork failed (%s)", strerror(errno) );
        return;
    }
    else if( childpid > 0 )
    {
        exit( 0 );    /* parent */
    }
}
void TRawServer :: start(void)
{
	m_running = true;
	m_crawler->start();
	Create();
}

void TRawServer :: stop(void)
{
	m_running = false;
}

}

