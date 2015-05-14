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

#ifndef __UTILS_H__
#define __UTILS_H__


namespace matrix {

#define MAX_LINE_LEN 8192
using namespace std;

typedef struct {
	int       read_fd;        /* caller's descriptor to read from */
	char      *read_ptr;      /* caller's buffer to read into */
	size_t    read_maxlen;    /* max #bytes to read */

	/* next three are used internally by the function */
	int       rl_cnt;         /* initialize to 0 */
	char      *rl_bufptr;     /* initialize to rl_buf */
	char      rl_buf[MAX_LINE_LEN];
} rline_t;

extern void readLineExInit(int fd, char *ptr, size_t maxlen, rline_t *rptr);
extern ssize_t readcEx(rline_t *rptr, char *ptr);
extern size_t readLineEx(rline_t *rptr);

extern int safe_open(const char *pathname,  int flags);
extern ssize_t safe_write(int fd,  const void *buf,  size_t count);
extern ssize_t safe_read(int fd,  void *buf,  size_t count);
extern ssize_t full_write(int fd, const void *buf, size_t len);
//this will rollover ~ every 49.7 days
extern unsigned int getTickCount(void);
extern std::string shaString(char* data, size_t len);
extern bool createDir(const char* path);
extern std::string extractFileName(const char* fullPath);
extern std::string extractFilePath(const char* fullPath);

extern std::string second2Str(unsigned int second);
extern std::string byteCount2Str(int64_t count);
extern int64_t htonll(int64_t number);
extern int64_t ntohll(int64_t number);
extern std::string getPeerStr(const char* ip, unsigned short port);
extern string UrlEncode( CPCHAR sz );
extern std::string UrlDecode( CPCHAR sz );
extern std::string Base64Encode( CPCHAR sz );
extern std::string Base64Decode( CPCHAR sz );
extern size_t StrLen(const char *str) ;
extern int StrCmp(const char * s1, const char * s2);
extern char * StrCpy(char * dst, const char * src);
extern char * StrnCpy(char * dst, const char * src, size_t n);
extern size_t get_nl(char *sfrom) ;
extern void set_nl(char *sto, size_t from);
extern void htons_buf( UINT16* pnbuf, const void* phbuf, UINT cnt );
extern void ntohs_buf( UINT16* phbuf, const void* pnbuf, UINT cnt );
extern void htonl_buf( UINT32* pnbuf, const void* phbuf, UINT cnt );
extern void ntohl_buf( UINT32* phbuf, const void* pnbuf, UINT cnt );
extern int StrToInt(const char * s);
extern double Pow(double x, int y);
extern char * itoa(int n, char* str) ;
extern char * ftoa(double num, char * str, int ndec);
//////////////////////////////////////////////////
extern bool IsFileExist(const char * FileName);
extern void SizeToStr(char *sizeStr, off_t size);
extern void delay_ms(int ms);
extern void TimeToStr(char *timeStr, double time);
extern int diffTime(struct timeval *result, struct timeval *x, struct timeval *y);
extern int numdigit(long a);
extern int is_number(char *str);
extern int  createOutFile(const char * PathName );
extern time_t strToTime(char* str);
extern time_t transTime(string timestr);

extern int DiffTime(struct timeval *result, struct timeval *x, struct timeval *y);
extern void ShowMsg(const char * args, ...);
extern int Panic(const char * args, ...);
extern int  CreateOutFile(const char * PathName);
extern bool IsTargetFileExist(const char * FileName);
extern void base64_encode(const char *s, char *store, int length);
extern char * StrDup(const char * str);
//void dbgout( const char* fmt, ... );
void strToLower(string& str, int nLen);
void strToUpper(string& str, int nLen);

#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')

#define ISBLANK(x) ((x) == ' ' || (x) == '\t')

#define RETURN_IF_FAILED( test ) \
	{ int ret; if((ret=(test)) != SUCC) return ret; }
#define EXIT_IF_FAILED( test ) \
	{ int ret; if((ret=(test)) != SUCC) return ret; }

}

#endif
