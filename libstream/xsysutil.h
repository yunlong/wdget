/*
 * Copyright (C) 2000-2004 the xine project
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * $Id: xineutils.h,v 1.96 2004/12/12 13:51:30 valtri Exp $
 *
 */

#ifndef _XSYS_UTIL_H
#define _XSYS_UTIL_H


#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
//#include <pthread.h>

//#ifdef XINE_COMPILE
//#  include <inttypes.h>
//#  include "attributes.h"
//#  include "compat.h"
//#  include "xmlparser.h"
//#  include "xine_buffer.h"
//#  include "configfile.h"
//#else
#  ifdef WIN32
#    include <winsock.h>
#  else
#    include <sys/time.h>
#  endif
//#  include <xine/os_types.h>
//#  include <xine/attributes.h>
//#  include <xine/compat.h>
//#  include <xine/xmlparser.h>
//#  include <xine/xine_buffer.h>
//#  include <xine/configfile.h>
//#endif

#include <stdio.h>
#include <string.h>


/*
 * Allocate and clean memory size_t 'size', then return the pointer
 * to the allocated memory.
 */
#if !defined(__GNUC__) || __GNUC__ < 3
extern void *
xutil_malloc(size_t size);
#else
extern void *
xutil_malloc(size_t size) __attribute__ ((__malloc__));
#endif

/*
 * Same as above, but memory is aligned to 'alignement'.
 * **base is used to return pointer to un-aligned memory, use
 * this to free the mem chunk
 */
extern void *
xutil_malloc_aligned(size_t alignment, size_t size, void **base);

extern void
xutil_mfree(void *base);


extern void *
xutil_fast_memcpy(void * to, const void * from, size_t n);

extern void
xutil_usec_sleep(unsigned usec);


/*
 * Some string functions
 */

extern void
xutil_strdupa(char *dest, char *src);

#define xutil_strdupa(d, s) 	\
	do {                                            \
		(d) = NULL;                                 \
		if ((s) != NULL)        \
		{                       \
			(d) = (char *) alloca(strlen((s)) + 1); \
				strcpy((d), (s));                   \
		}                                           \
	} while(0)

/* compatibility macros */
#define xutil_strpbrk(S, ACCEPT) strpbrk((S), (ACCEPT))
#define xutil_strsep(STRINGP, DELIM) strsep((STRINGP), (DELIM))
#define xutil_setenv(NAME, VAL, XX) setenv((NAME), (VAL), (XX))

extern char *
xutil_strndup(const char *src, size_t n);


/* print a hexdump of the given data */
void xutil_hexdump (const char *buf, int length);

/*
 * Optimization macros for conditions
 * Taken from the FIASCO L4 microkernel sources
 */
#if !defined(__GNUC__) || __GNUC__ < 3
#  define EXPECT_TRUE(x)  (x)
#  define EXPECT_FALSE(x) (x)
#else
#  define EXPECT_TRUE(x)  __builtin_expect((x),1)
#  define EXPECT_FALSE(x) __builtin_expect((x),0)
#endif

#define x_abort()      \
  do {                                                                \
    fprintf(stderr, "abort: %s:%d: %s: Aborting.\n",                  \
            __FILE__, __LINE__, __FUNC__);                            \
    abort();                                                          \
  } while(0)

#ifdef NDEBUG
#define x_assert(exp)  \
  do {                                                                \
    if (!(exp))                                                       \
      fprintf(stderr, "assert: %s:%d: %s: Assertion `%s' failed.\n",  \
              __FILE__, __LINE__, __FUNC__, #exp);                    \
  } while(0)
#else
#define x_assert(exp)  \
  do {                                                                \
    if (!(exp)) {                                                     \
      fprintf(stderr, "assert: %s:%d: %s: Assertion `%s' failed.\n",  \
              __FILE__, __LINE__, __FUNC__, #exp);                    \
      abort();                                                        \
    }                                                                 \
  } while(0)
#endif


/****** logging with **********************************/

/*
 * log constants
 */

#define XLOG_MSG       0 /* warnings, errors, ... */
#define XLOG_PLUGIN    1
#define XLOG_TRACE     2
#define XLOG_NUM       3 /* # of log buffers defined */


#if !defined(_)
# define _(v)            v
#endif

#if !defined(__FUNC__)
# if defined(__FUNCTION__)
#  define __FUNC__        __FUNCTION__
# else
#  define __FUNC__        "unknown func"
# endif
#endif	/* #if !defined(__FUNC__) */

#ifndef LOG_MODULE
  #define LOG_MODULE __FILE__
#endif /* LOG_MODULE */

#define LOG_MODULE_STRING printf("%s: ", LOG_MODULE );

#ifdef LOG_VERBOSE
  #define LONG_LOG_MODULE_STRING                                            \
    printf("%s: (%s:%d) ", LOG_MODULE, __FUNC__, __LINE__ );
#else
  #define LONG_LOG_MODULE_STRING  LOG_MODULE_STRING
#endif /* LOG_VERBOSE */

#ifdef LOG
  #ifdef __GNUC__
    #define lprintf(fmt, args...)                                           \
      do {                                                                  \
        LONG_LOG_MODULE_STRING                                              \
        printf(fmt, ##args);                                                \
      } while(0)
  #else /* __GNUC__ */
    #ifdef _MSC_VER
      #define lprintf(fmtargs)                                              \
        do {                                                                \
          LONG_LOG_MODULE_STRING                                            \
          printf("%s", fmtargs);                                            \
        } while(0)
    #else /* _MSC_VER */
      #define lprintf(fmt, ...)                                             \
        do {                                                                \
          LONG_LOG_MODULE_STRING                                            \
          printf(__VA_ARGS__);                                              \
        } while(0)
    #endif  /* _MSC_VER */
  #endif /* __GNUC__ */
#else /* LOG */
  #ifdef __GNUC__
    #define lprintf(fmt, args...)     do {} while(0)
  #else
  #ifdef _MSC_VER
void __inline lprintf(const char * fmt, ...) {};
  #else
    #define lprintf(...)              do {} while(0)
  #endif /* _MSC_VER */
  #endif /* __GNUC__ */
#endif /* LOG */

#ifdef __GNUC__
  #define llprintf(cat, fmt, args...)                                       \
    do{                                                                     \
      if(cat){                                                              \
        LONG_LOG_MODULE_STRING                                              \
        printf( fmt, ##args );                                              \
      }                                                                     \
    }while(0)
#else
#ifdef _MSC_VER
extern void
xutil_printf(int verbose, const char *fmt, ...);

    #define llprintf  xutil_printf
/*  #define llprintf(cat, fmtargs)                                            \
    do{                                                                     \
      if(cat){                                                              \
        LONG_LOG_MODULE_STRING                                              \
        printf( "%s", fmtargs );                                            \
      }                                                                     \
    }while(0)
 */
#else
  #define llprintf(cat, ...)                                                \
    do{                                                                     \
      if(cat){                                                              \
        LONG_LOG_MODULE_STRING                                              \
        printf( __VA_ARGS__ );                                              \
      }                                                                     \
    }while(0)
#endif /* _MSC_VER */
#endif /* __GNUC__ */

#ifdef  __GNUC__
  #define xprintf(verbose, fmt, args...)                                    \
    do {                                                                    \
        lprintf(XLOG_TRACE, fmt, ##args);                                   \
    } while(0)
#else
#ifdef _MSC_VER
  #define xprintf xutil_printf
#else
  #define xprintf(verbose, ...)                                             \
    do {                                                                    \
        llprintf(XLOG_TRACE, __VA_ARGS__);                                  \
    } while(0)
#endif /* _MSC_VER */
#endif /* __GNUC__ */


#define XMSG_NO_ERROR               0  /* (messages to UI)   */
#define XMSG_GENERAL_WARNING        1  /* (warning message)  */
#define XMSG_UNKNOWN_HOST           2  /* (host name)        */
#define XMSG_UNKNOWN_DEVICE         3  /* (device name)      */
#define XMSG_NETWORK_UNREACHABLE    4  /* none               */
#define XMSG_CONNECTION_REFUSED     5  /* (host name)        */
#define XMSG_FILE_NOT_FOUND         6  /* (file name or mrl) */
#define XMSG_READ_ERROR             7  /* (device/file/mrl)  */
#define XMSG_LIBRARY_LOAD_ERROR     8  /* (library/decoder)  */
#define XMSG_ENCRYPTED_SOURCE       9  /* none               */
#define XMSG_SECURITY               10 /* (security message) */
#define XMSG_AUDIO_OUT_UNAVAILABLE  11 /* none               */
#define XMSG_PERMISSION_ERROR       12 /* (file name or mrl) */


extern int
xutil_message(int type, ...);

/* verbosity settings */
#define XVERBOSITY_NONE             0
#define XVERBOSITY_LOG              1
#define XVERBOSITY_DEBUG            2
 
#define xmfree   xutil_mfree
#define xmalloc  xutil_malloc
#undef xmalloc  
#define xmalloc  xutil_malloc_aligned
#define xmessage xutil_message

extern int
xutil_parse_url(char *url, char **proto, char **host, int *port,
    char **user, char **password, char **uri);

extern char *
xutil_system_encoding(void);

/*
 * guess default encoding for the subtitles
 */
extern const char *
xutil_guess_spu_encoding(void);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef _XSYS_UTIL_H */


