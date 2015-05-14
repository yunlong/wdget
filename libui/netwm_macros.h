#ifndef _NET_MACROS_H_
#define _NET_MACROS_H_

#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 > 2)
#define KDE_NO_EXPORT __attribute__ ((visibility("hidden")))
#define KDE_EXPORT __attribute__ ((visibility("visible")))
#else
#define KDE_NO_EXPORT
#define KDE_EXPORT
#endif


#ifdef __GNUC__
#define KDE_PACKED __attribute__((__packed__))
#else
#define KDE_PACKED
#endif

#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
# define KDE_DEPRECATED __attribute__ ((deprecated))
#else
# define KDE_DEPRECATED
#endif

#endif // _NET_MACROS_H_
