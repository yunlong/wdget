#ifndef __THREAD_H__
#define __THREAD_H__

namespace matrix {

class TMutex
{
private:
	pthread_mutex_t m_mutex;
public:
	TMutex(void);
	virtual ~TMutex(void);
	
	void Lock(void);
	void Unlock(void);	
};

class TSemaphore
{
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
    UINT            m_count;
public:
    TSemaphore( unsigned int nCount );
    virtual ~TSemaphore( void );
    void Lock( void );
    void Unlock( void );
};



/*
 * Initializing a C++ thread object must involve two steps: creating the
 * C++ object and creating the thread.  If we create the thread in the
 * ctor, we are courting disaster because the C++ object is not fully
 * constructed until after the ctor finishes.  If you think this is all
 * theoretical and doesn't apply in real life, consider this:
 *
 * - GNU g++ will not call a derived virtual function until after
 * the ctor is done -- it will *always* call the function defined in
 * the current class.  Been there, done that, spent hours debugging.
 *
 */

typedef pthread_t       threadobj_t;
class TThread
{
	friend void* thread_start( void* );
public:
    TThread( void );
    virtual ~TThread( void );
public:
	void Create( void );
    void Suspend( void );
    void JoinSelf( void );
    void Resume( void );


    TThread* This( void );

protected:
    threadobj_t m_thread;
    int         m_retval;
protected:
    virtual bool Init( void );
    virtual void Run( void );
    virtual int  Exit( void );

};

}

#endif 
