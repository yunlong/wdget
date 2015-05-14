#define USE_THREAD_H
#include "matrix.h"


namespace matrix {

/*************** class TMutex Object Impl ***************/
TMutex::TMutex(void)
{
	pthread_mutex_init( &m_mutex, NULL );
}

TMutex::~TMutex(void)
{
	pthread_mutex_destroy( &m_mutex );
}

void TMutex::Lock(void)
{
	pthread_mutex_lock( &m_mutex );
}

void TMutex::Unlock(void)
{
	pthread_mutex_unlock( &m_mutex );
}

/************** class TSemaphore Object Impl ***************/
TSemaphore::TSemaphore( UINT nCount )
{
    pthread_mutex_init( &m_mutex, NULL );
    pthread_cond_init( &m_cond, NULL );
    m_count = nCount;
}

TSemaphore::~TSemaphore( void )
{
    pthread_cond_destroy( &m_cond );
    pthread_mutex_destroy( &m_mutex );
}

void TSemaphore::Lock( void )
{
    pthread_mutex_lock( &m_mutex );
    while( m_count == 0 )
    {
        pthread_cond_wait( &m_cond, &m_mutex );
    }
    m_count--;
    pthread_mutex_unlock( &m_mutex );
}

void TSemaphore::Unlock( void )
{
    pthread_mutex_lock( &m_mutex );
    m_count++;
    pthread_mutex_unlock( &m_mutex );
    pthread_cond_signal( &m_cond );
}



/************** class TThread Object Impl ***************/
pthread_key_t g_keyself;
void* thread_start( void* pvoid )
{
    TThread* pth;
 	pthread_setspecific( g_keyself, pvoid );
    pth = (TThread*)pvoid;
    if( pth->Init() )
    {
        pth->Run();
    }
    pth->m_retval = pth->Exit();
    return NULL;
}

TThread::TThread( void )
{
	//
}

TThread::~TThread( void )
{
	//
}

void TThread::Create( void )
{
    pthread_create( &m_thread, NULL, thread_start, this );
}

bool TThread::Init( void )
{
    return false;
}

int TThread::Exit( void )
{
    return 0;
}

void TThread::Run( void )
{
	//
}

void TThread :: Suspend( void )
{
    pthread_cancel(m_thread);
}

void TThread :: JoinSelf(void)
{
    pthread_join(m_thread, NULL);
}

void TThread :: Resume( void )
{
    //Empty
}

TThread* TThread::This( void )
{
	return (TThread*)pthread_getspecific( g_keyself );
}

}

