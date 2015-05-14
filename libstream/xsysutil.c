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
 * $Id: utils.c,v 1.37.2.1 2005/04/20 17:21:17 mroi Exp $
 *
 */
#define	_POSIX_PTHREAD_SEMANTICS 1	/* for 5-arg getpwuid_r on solaris */

/*
#define LOG
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xsysio.h"

#ifdef WIN32
#  include <winsock.h>
#else
#  include <unistd.h>
#  include <pwd.h>
#  include <sys/time.h>
#endif

#include <errno.h>
#include <time.h>

#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#if HAVE_UCONTEXT_H
#include <ucontext.h>
#endif

#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

#if defined(__CYGWIN__) || defined(WIN32)
#include <windows.h>
#endif

typedef struct {
  char                    *language;     /* name of the locale */
  char                    *encoding;     /* typical encoding */
  char                    *spu_encoding; /* default spu encoding */
  char                    *modifier;
} lang_locale_t;


/*
 * information about locales used in xine
 */
static lang_locale_t lang_locales[] = {
  { "af_ZA",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "ar_AE",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_BH",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_DZ",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_EG",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_IN",    "utf-8",       "utf-8",       NULL       },
  { "ar_IQ",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_JO",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_KW",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_LB",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_LY",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_MA",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_OM",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_QA",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_SA",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_SD",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_SY",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_TN",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "ar_YE",    "iso-8859-6",  "iso-8859-6",  NULL       },
  { "be_BY",    "cp1251",      "cp1251",      NULL       },
  { "bg_BG",    "cp1251",      "cp1251",      NULL       },
  { "br_FR",    "iso-8859-1",  "iso-88591",   NULL       },
  { "bs_BA",    "iso-8859-2",  "cp1250",      NULL       },
  { "ca_ES",    "iso-8859-1",  "iso-88591",   NULL       },
  { "ca_ES",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "cs_CZ",    "iso-8859-2",  "cp1250",      NULL       },
  { "cy_GB",    "iso-8859-14", "iso-8859-14", NULL       },
  { "da_DK",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "de_AT",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "de_AT",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "de_BE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "de_BE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "de_CH",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "de_DE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "de_DE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "de_LU",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "de_LU",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "el_GR",    "iso-8859-7",  "iso-8859-7",  NULL       },
  { "en_AU",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_BW",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_CA",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_DK",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_GB",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_HK",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_IE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_IE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "en_IN",    "utf-8",       "utf-8",       NULL       },
  { "en_NZ",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_PH",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_SG",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_US",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_ZA",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "en_ZW",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_AR",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_BO",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_CL",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_CO",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_CR",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_DO",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_EC",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_ES",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_ES",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "es_GT",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_HN",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_MX",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_NI",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_PA",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_PE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_PR",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_PY",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_SV",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_US",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_UY",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "es_VE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "et_EE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "eu_ES",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "eu_ES",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "fa_IR",    "utf-8",       "utf-8",       NULL       },
  { "fi_FI",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fi_FI",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "fo_FO",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fr_BE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fr_BE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "fr_CA",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fr_CH",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fr_FR",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fr_FR",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "fr_LU",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "fr_LU",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "ga_IE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "ga_IE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "gl_ES",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "gl_ES",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "gv_GB",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "he_IL",    "iso-8859-8",  "iso-8859-8",  NULL       },
  { "hi_IN",    "utf-8",       "utf-8",       NULL       },
  { "hr_HR",    "iso-8859-2",  "cp1250",      NULL       },
  { "hu_HU",    "iso-8859-2",  "cp1250",      NULL       },
  { "id_ID",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "is_IS",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "it_CH",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "it_IT",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "it_IT",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "iw_IL",    "iso-8859-8",  "iso-8859-8",  NULL       },
  { "ja_JP",    "euc-jp",      "euc-jp",      NULL       },
  { "ja_JP",    "ujis",        "ujis",        NULL       },
  { "japanese", "euc",         "euc",         NULL       },
  { "ka_GE",    "georgian-ps", "georgian-ps", NULL       },
  { "kl_GL",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "ko_KR",    "euc-kr",      "euc-kr",      NULL       },
  { "ko_KR",    "utf-8",       "utf-8",       NULL       },
  { "korean",   "euc",         "euc",         NULL       },
  { "kw_GB",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "lt_LT",    "iso-8859-13", "iso-8859-13", NULL       },
  { "lv_LV",    "iso-8859-13", "iso-8859-13", NULL       },
  { "mi_NZ",    "iso-8859-13", "iso-8859-13", NULL       },
  { "mk_MK",    "iso-8859-5",  "cp1251",      NULL       },
  { "mr_IN",    "utf-8",       "utf-8",       NULL       },
  { "ms_MY",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "mt_MT",    "iso-8859-3",  "iso-8859-3",  NULL       },
  { "nb_NO",    "ISO-8859-1",  "ISO-8859-1",  NULL       },
  { "nl_BE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "nl_BE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "nl_NL",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "nl_NL",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "nn_NO",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "no_NO",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "oc_FR",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "pl_PL",    "iso-8859-2",  "cp1250",      NULL       },
  { "pt_BR",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "pt_PT",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "pt_PT",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "ro_RO",    "iso-8859-2",  "cp1250",      NULL       },
  { "ru_RU",    "iso-8859-5",  "cp1251",      NULL       },
  { "ru_RU",    "koi8-r",      "cp1251",      NULL       },
  { "ru_UA",    "koi8-u",      "cp1251",      NULL       },
  { "se_NO",    "utf-8",       "utf-8",       NULL       },
  { "sk_SK",    "iso-8859-2",  "cp1250",      NULL       },
  { "sl_SI",    "iso-8859-2",  "cp1250",      NULL       },
  { "sq_AL",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "sr_YU",    "iso-8859-2",  "cp1250",      NULL       },
  { "sr_YU",    "iso-8859-5",  "cp1251",      "cyrillic" },
  { "sv_FI",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "sv_FI",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "sv_SE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "ta_IN",    "utf-8",       "utf-8",       NULL       },
  { "te_IN",    "utf-8",       "utf-8",       NULL       },
  { "tg_TJ",    "koi8-t",      "cp1251",      NULL       },
  { "th_TH",    "tis-620",     "tis-620",     NULL       },
  { "tl_PH",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "tr_TR",    "iso-8859-9",  "iso-8859-9",  NULL       },
  { "uk_UA",    "koi8-u",      "cp1251",      NULL       },
  { "ur_PK",    "utf-8",       "utf-8",       NULL       },
  { "uz_UZ",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "vi_VN",    "tcvn",        "tcvn",        NULL       },
  { "vi_VN",    "utf-8",       "utf-8",       NULL       },
  { "wa_BE",    "iso-8859-1",  "iso-8859-1",  NULL       },
  { "wa_BE",    "iso-8859-15", "iso-8859-15", "euro"     },
  { "yi_US",    "cp1255",      "cp1255",      NULL       },
  { "zh_CN",    "gb18030",     "gb18030",     NULL       },
  { "zh_CN",    "gb2312",      "gb2312",      NULL       },
  { "zh_CN",    "gbk",         "gbk",         NULL       },
  { "zh_HK",    "big5-hkscs",  "big5-hkscs",  NULL       },
  { "zh_TW",    "big-5",       "big-5",       NULL       },
  { "zh_TW",    "euc-tw",      "euc-tw",      NULL       },
  { NULL,       NULL,          NULL,          NULL       }
};


extern void *
xutil_malloc(size_t size)
{
  void *ptr;

  /* prevent xine_xmalloc(0) of possibly returning NULL */
  if (!size)
    size++;
  if ((ptr = calloc(1, size)) == NULL)
  {
    fprintf(stderr, "%s: malloc() failed: %s.\n", __FUNC__, strerror(errno));
    return NULL;
  }

  return ptr;
}

extern void *
xutil_malloc_aligned(size_t alignment, size_t size, void **base)
{
  char *ptr;
  
  ptr = (char *)xutil_malloc (size+alignment);
  *base = (void *)ptr;
  
  while ((size_t) ptr % alignment)
    ptr++;
  
  return ptr;
}

extern void
xutil_mfree(void *base)
{
	if (base != NULL)
		free(base);
}


extern void *
xutil_fast_memcpy(void * to, const void * from, size_t n)
{
	return memcpy(to, from, n);
}

#ifndef BUFSIZ
#define BUFSIZ 256
#endif

/**
 * a thread-safe usecond sleep
 */
extern void
xutil_usec_sleep(unsigned usec)
{
#if HAVE_NANOSLEEP
  /* nanosleep is prefered on solaris, because it's mt-safe */
  struct timespec ts, remaining;

  ts.tv_sec =   usec / 1000000;
  ts.tv_nsec = (usec % 1000000) * 1000;
  while (nanosleep (&ts, &remaining) == -1 && errno == EINTR)
    ts = remaining;
#else
#  if WIN32
  Sleep(usec / 1000);
#  else
  usleep(usec);
#  endif
#endif
}

extern char *
xutil_strndup(const char *src, size_t n)
{
  char *dest;

  if (n < 0)
    n = strlen(src);
  
  dest = (char *)xutil_malloc(n + 2);
  if (dest != NULL)
  {
    memcpy(dest, src, n);
    dest[n] = '\0';
  }
  return dest;
}

/**
 * print a hexdump of length bytes from the data given in buf
 */
extern void
xutil_hexdump (const char *buf, int length)
{
  int i,j;
  unsigned char c;

  /* printf ("Hexdump: %i Bytes\n", length);*/
  for(j=0; j<69; j++)
    printf ("-");
  printf ("\n");

  j=0;
  while(j<length) {
    printf ("%04X ",j);
    for (i=j; i<j+16; i++) {
      if( i<length )
        printf ("%02X ", (unsigned char) buf[i]);
      else
        printf("   ");
    }
    for (i=j;i<(j+16<length?j+16:length);i++) {
      c=buf[i];
      if ((c>=32) && (c<127))
        printf ("%c", c);
      else
        printf (".");
    }
    j=i;
    printf("\n");
  }

  for(j=0; j<69; j++)
    printf("-");
  printf("\n");
}


extern int
xutil_parse_url (char *url, char **proto, char **host, int *port,
	 char **user, char **password, char **uri)
{
  char   *start      = NULL;
  char   *authcolon  = NULL;
  char	 *at         = NULL;
  char	 *portcolon  = NULL;
  char   *slash      = NULL;
  char   *semicolon  = NULL;
  char   *end        = NULL;
  char   *strtol_err = NULL;

  if (!url)      x_abort();
  if (!proto)    x_abort();
  if (!user)     x_abort();
  if (!password) x_abort();
  if (!host)     x_abort();
  if (!port)     x_abort();
  if (!uri)      x_abort();

  *proto    = NULL;
  *port     = 0;
  *user     = NULL;
  *host     = NULL;
  *password = NULL;
  *uri      = NULL;

  /* proto */  
  start = strstr(url, "://");
  end  = start + strlen(start) - 1;
  if (!start || (start == url))
    goto error;
  
  *proto = xutil_strndup(url, start - url);
  
  /* user:password */
  start += 3;
  at = strchr(start, '@');
  slash = strchr(start, '/');

  /* stupid Nullsoft URL scheme */
  semicolon = strchr(start, ';');
  if (semicolon && (!slash || (semicolon < slash)))
    slash = semicolon;
  
  if (at && slash && (at > slash))
    at = NULL;
  
  if (at) {
    authcolon = strchr(start, ':');
    if(authcolon && authcolon < at) {
      *user = xutil_strndup(start, authcolon - start);
      *password = xutil_strndup(authcolon + 1, at - authcolon - 1);
      if ((authcolon == start) || (at == (authcolon + 1))) goto error;
    } else {
      /* no password */
      *user = xutil_strndup(start, at - start);
      if (at == start) goto error;
    }
    start = at + 1;
  }

  /* host:port (ipv4) */
  /* [host]:port (ipv6) */
  if (*start != '[')
  {
    /* ipv4*/
    portcolon = strchr(start, ':');
    if (slash) {
      if (portcolon && portcolon < slash) {
        *host = xutil_strndup(start, portcolon - start);
        if (portcolon == start) goto error;
        *port = strtol(portcolon + 1, &strtol_err, 10);
        if ((strtol_err != slash) || (strtol_err == portcolon + 1))
          goto error;
      } else {
        *host = xutil_strndup(start, slash - start);
        if (slash == start) goto error;
      }
    } else {
      if (portcolon) {
        *host = xutil_strndup(start, portcolon - start);
        if (portcolon < end) {
          *port = strtol(portcolon + 1, &strtol_err, 10);
          if (*strtol_err != '\0') goto error;
        } else {
          goto error;
        }
      } else {
        if (*start == '\0') goto error;
        *host = strdup(start);
      }
    }
  } else {
    /* ipv6*/
    char *hostendbracket;

    hostendbracket = strchr(start, ']');
    if (hostendbracket != NULL) {
      if (hostendbracket == start + 1) goto error;
      *host = xutil_strndup(start + 1, hostendbracket - start - 1);

      if (hostendbracket < end) {
        /* Might have a trailing port */
        if (*(hostendbracket + 1) == ':') {
          portcolon = hostendbracket + 1;
          if (portcolon < end) {
            *port = strtol(portcolon + 1, &strtol_err, 10);
            if ((*strtol_err != '\0') && (*strtol_err != '/')) goto error;
          } else {
            goto error;
          }
        }
      }
    } else {
      goto error;
    }
  }

  /* uri */
  start = slash;
  if (start) {
    /* handle crazy Nullsoft URL scheme */
    if (*start == ';') {
      /* ";stream.nsv" => "/;stream.nsv" */
      *uri = (char *)malloc(strlen(start) + 2);
      *uri[0] = '/';
      strcpy(*uri + 1, start);
    } else {
      *uri = strdup(start);
    }
  } else {
    *uri = strdup("/");
  }
  
  return 1;
  
error:
  if (*proto) {
    free (*proto);
    *proto = NULL;
  }
  if (*user) {
    free (*user);
    *user = NULL;
  }
  if (*password) {
    free (*password);
    *password = NULL;
  }
  if (*host) {
    free (*host);
    *host = NULL;
  }
  if (*port) {
    *port = 0;
  }
  if (*uri) {
    free (*uri);
    *uri = NULL;
  }
  return 0;  
}


static char *
_get_lang(void) {
    char *lang;
    
    if(!(lang = getenv("LC_ALL")))
      if(!(lang = getenv("LC_MESSAGES")))
        lang = getenv("LANG");

  return lang;
}

static const lang_locale_t *
_get_first_lang_locale(char *lcal)
{
  const lang_locale_t *llocale;

  if(lcal && strlen(lcal)) {
    llocale = &*lang_locales;
    
    while(llocale->language) {
      if(!strncmp(lcal, llocale->language, strlen(lcal)))
	return llocale;
      
      llocale++;
    }
  }
  return NULL;
}

/**
 * get encoding of current locale
 */
extern char *
xutil_system_encoding(void)
{
  char *codeset = NULL;
  
#ifdef HAVE_LANGINFO_CODESET
  codeset = nl_langinfo(CODESET);
#endif
  /*
   * guess locale codeset according to shell variables
   * when nl_langinfo(CODESET) isn't available or workig
   */
  if (!codeset || strstr(codeset, "ANSI") != 0) {
    char *lang = _get_lang();

    codeset = NULL;

    if(lang) {
      char *lg, *enc, *mod;

      lg = strdup(lang);

      if((enc = strchr(lg, '.')) && (strlen(enc) > 1)) {
        enc++;

        if((mod = strchr(enc, '@')))
          *mod = '\0';

        codeset = strdup(enc);
      }
      else {
        const lang_locale_t *llocale = _get_first_lang_locale(lg);

        if(llocale && llocale->encoding)
          codeset = strdup(llocale->encoding);
      }

      free(lg);
    }
  } else
    codeset = strdup(codeset);

  return codeset;
}

/**
 * guess default encoding of subtitles
 */
extern const char *
xine_guess_spu_encoding(void)
{
  char *lang = _get_lang();

  if (lang) {
    const lang_locale_t *llocale;
    char *lg, *enc;

    lg = strdup(lang);

    if ((enc = strchr(lg, '.'))) *enc = '\0';
    llocale = _get_first_lang_locale(lg);
    free(lg);
    if (llocale && llocale->spu_encoding) return llocale->spu_encoding;
  }

  return "iso-8859-1";
}

#ifdef _MSC_VER
void xutil_printf(int verbose, const char *fmt, ...)
{
  va_list ap;
  char message[256];
  
//if (xine && xine->verbosity >= verbose) {
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);
    printf("%s", message);
//    xine_log(XLOG_TRACE, "%s", message);
//}
}
#endif

extern int
xutil_message(int type, ...)
{
  char                   *explanation;
  int                     size;
  int                     n;
  va_list                 ap;
  char                   *s;
  char                   *data;
  char                   *params;
  char                   *args[1024];

  static char            *std_explanation[] = {
    "",
    "Warning:",
    "Unknown host:",
    "Unknown device:",
    "Network unreachable",
    "Connection refused:",
    "File not found:",
    "Read error from:",
    "Error loading library:",
    "Encrypted media stream detected",
    "Security message:",
    "Audio device unavailable",
    "Permission error",
  };
  
  if ((type >= 0) && 
      (type < (int)(sizeof(std_explanation)/sizeof(std_explanation[0]))))
  {
    explanation = std_explanation[type];
    size = strlen(explanation)+1;
  } else {
    explanation = NULL;
    size = 0;
  }
  
  n = 0;
  va_start(ap, type);
  while (((s = va_arg(ap, char *)) != NULL) && (n < 1024)) {
    size += strlen(s) + 1;
    args[n] = s;
    n++;
  }
  va_end(ap);
  
  args[n] = NULL;
  
  size += sizeof(void *) + 1;
  data = (char *)xutil_malloc(size);
  
  if (explanation) {
    strcpy(data, explanation);
    params = data + strlen(explanation);
	*params++ = '\n';
  } else {
    params = data;
  }
  *params = '\0';
  
  for (n = 0; args[n]; n++) {
    strcpy(params, args[n]);
    params += strlen(args[n]);
	*params++ = '\n';
  }
  *params = '\0';
  
  lprintf("%s", data);
  
  xutil_mfree(data);
  
  return n;
}


