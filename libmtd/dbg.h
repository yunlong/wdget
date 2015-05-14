#ifndef __DBG_H__
#define __DBG_H__

namespace matrix {

void dbgout( const char* fmt, ... );

#ifdef NDEBUG
inline void dump_alloc_heaps( void ) {}
#else
void* operator new( size_t n, CPCHAR file, UINT line );
void* operator new[]( size_t n, CPCHAR file, UINT line );
void  operator delete( void* p );
void  operator delete[]( void* p );
void* operator new( size_t n );
void* operator new[]( size_t n );
#define new new(__FILE__,__LINE__)
void dump_alloc_heaps( void );
#endif

#define assert_or_ret(cond) { assert(cond); if( !(cond) ) return; }
#define assert_or_retv(val,cond) { assert(cond); if( !(cond) ) return (val); }

}

#endif 
