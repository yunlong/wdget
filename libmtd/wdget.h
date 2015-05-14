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

#ifndef __WIN_DGET_H__
#define __WIN_DGET_H__

#include <string>
#include <vector>

using namespace std;

//#define ENABLE_DEBUG_MODE
#define _FILE_OFFSET_BITS 64

#include <openssl/ssl.h>
#include <openssl/err.h>

#define	TRACE(x)	do { printf("%s, %d, %s\n", __FILE__, __LINE__, x);	\
					} while(0);
					
#define SOFTWARE_NAME 		"wdget for linux by yunlong.lee"
#define SOFTWARE_VERSION 	"0.5v with QT"


typedef  void * (*ThreadFunc)(void *);

typedef enum 
{
/////////////////////////////
	PROXY_DIRECT,
	PROXY_HTTP,
	PROXY_FTP,
	PROXY_SOCK4,
	PROXY_SOCK5,
	HTTP_CONNECT
/////////////////////////////	
} ProxyType;

typedef struct
{
	string		name;
	ProxyType	type;
	string		host;
	string		user;
	string		pass;
	bool		auth;
	int			port;
} Proxy_t;

typedef enum
{
	TSK_AUTO_EXEC		= 0,
	TSK_MANNUAL_EXEC	= 1,
	TSK_PLAN_EXEC		= 2,
} TaskExecKind;

typedef enum
{
	FILE_TYPE_SOFT			= 0,
	FILE_TYPE_GAME			= 1,
	FILE_TYPE_MUSIC			= 2,
	FILE_TYPE_MOVIE			= 3,
} FileType;

typedef struct
{
	string origUrl;
	string cookie;
	string reffer;
	string path;
	
	FileType kind;	
	TaskExecKind	type;
	
	string file;
	string comm;

	string user;
	string pwd;	
	int	nblks;

	string proxyname;

	vector<string>	vSurl; // for Mirrors Url
} NewTaskParam ;

typedef struct
{
	int		max_task_num;
	int		min_blk_size;

	int		retry_times;
	int		retry_interval;

	int		timeout_wait;
	int		recv_buf_size;
	int		sync_cache_size;
} GlobalConfigParam;

typedef  void * (*ThreadFunc)(void *);
typedef unsigned long long  	ULONG64;
typedef unsigned long  			ULONG32;


#define FTP_PORT 	21
#define HTTP_PORT 	80
#define P2P_PORT	6667
#define MMS_PORT	1755
#define RTSP_PORT   554

#define TRUE	1
#define FALSE   0

/* The default no of connections and maximum redirections allowed  */
#define DEFAULT_CONN_NUM  	4
#define MAX_REDIRECTIONS  	5
#define FTP_USE_PASV   		TRUE

/* The timeout period for connections in seconds */
#define CONN_TIMEOUT 		120
/* The max number of trys and the delay between each */
#define MAX_TRY_ATTEMPTS    30
#define RETRY_INTERVAL		5
    
/*Default is to not log any debug info */
#define USE_FTP_SEARCH		FALSE;
/* The maximum number of servers to ping at once */
#define MAX_SIMU_PINGS		5
/* The max number of seconds to wait for a server response to ping */
#define MAX_PING_WAIT		4
/* The maximum number of servers/mirrors to request */
#define REQ_FTP_MIRROR_MAXNUM		20

#define MAX_BPS				-1   

/*Output the file to the directory ( "./" by default) */
#define DEFAULT_OUT_DIR    "download"
#define DEFAULT_FTP_SEARCH_URL 

#define  MAX_SIZE       	((ULONG64)(-1))
#define MAX_BLK_SIZE		((ULONG32)(-1))
#define MIN_BLK_SIZE 		32

#define MIN_FILE_SIZE		(64 * 1024)
#define MAX_FILE_SIZE	   ((ULONG64)(-1))

#define MAX_THREADS 	6
#define MAX_NAME_LEN	255
#define MAX_PATH_LEN	1024
#define MAX_TASK_NUM	10

#define LINE_BUFFER 	256
#define FTP_BUF_SIZE 	4096
//#define HTTP_BUF_SIZE 	4096
#define MMS_BUF_SIZE   	4096
#define RTSP_BUF_SIZE  	4096
#define CACHE_SIZE		(512*1024)
#define MAX_MSG_SIZE 	1024
#define DEFAULT_USER  			"anonymous"
#define DEFAULT_PASSWD 			"yunlong.lee@163.com"

/*the extension for the target file created*/
#define DEFAULT_FILE_EXT	".dg"

/*the extension for the log file created */
#define DEFAULT_LOG_EXT 	".log"

#define INSTALLDIR 				"/usr/local/wdget/"
#define DL_DIR_PATH 			"download" 

#define WDGET_CONFIG_PATH		"/usr/local/wdget/config/"


#define NSR_PROCESS_ERROR(expr)  \
	if (expr)	\
		goto errout
	
#define NSR_PROCESS_ERROUT(expr, retval)     \
	if (expr)	\
	{		\
		ret = retval;	\
		goto errout;	\
	}

#define NSR_PROCESS_RETURN(expr, retval)  \
	if (expr)	\
		return retval

#define NSR_PROCESS_SERVICE(expr)         \
	if ((expr) != 0)	\
		goto errout;

#define NSR_PROCESS_SOCKSLEN(ret, size)     \
	if (ret < 0)	\
		goto errout;	\
	else if (ret < (int)(size))	\
	{		\
		ret = -EPROTO;	\
		goto errout;	\
	}


#endif

