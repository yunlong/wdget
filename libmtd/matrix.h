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

#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdexcept>
#include <utility>
#include <exception>
#include <new>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <iomanip>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <string>

using namespace std;

#include <ctype.h>
#include <signal.h>
#include <getopt.h>
#include <cstdarg>
//#include <cstdint>
#include <clocale>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cerrno>
#include <climits>
#include <cassert>
#include <cmath>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <locale.h>

#include <libintl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


#include "Types.h"
#include "wdget.h"

#ifdef USE_INIFILE_H
#include "IniFile.h"
#endif 

#ifdef USE_BITSET_H
#include "BitSet.h"
#endif 

#ifdef USE_BUFFER_H
#include "Buffer.h"
#endif

#ifdef USE_LIST_H
#include "List.h"
#endif 

#ifdef USE_MD5_H
#include "MD5.h"
#endif 

#ifdef USE_SHA_H
#include "Sha.h"
#endif 

#ifdef USE_SLAB_H
#include "Slab.h"
#endif

#ifdef USE_FILE_H
#include "File.h"
#endif 

#ifdef USE_PARSER_H
#include "Parser.h"
#endif 

#ifdef USE_LOGGER_H
#include "Logger.h"
#endif

#ifdef USE_URI_H
#include "Uri.h"
using namespace htmlcxx;
#endif

#ifdef USE_HTTPMSG_H
#include "HttpMsg.h"
#endif

#ifdef USE_SOCKET_H
#include "Socket.h"
#endif 

#ifdef USE_POLLER_H
#include "Poller.h"
#endif

#ifdef USE_THREAD_H
#include "Thread.h"
#endif

#ifdef USE_STREAM_H
#include "Stream.h"
#endif


#ifdef USE_UTILS_H
#include "Utils.h"
#endif

#ifdef USE_HASHTABLE_H
#include "HashTable.h"
#endif

#ifdef USE_HTTPCLI_H
#include "HttpCli.h"
#endif

#ifdef USE_FTPCLI_H
#include "FtpCli.h"
#endif

#ifdef USE_FTPPARSER_H
#include "FtpParser.h"
#endif

#ifdef USE_MMSCLI_H
#include "mms.h"
#include "MmsCliEx.h"
#endif

#ifdef USE_RTSPCLI_H
#include "rtsp.h"
#include "rmff.h"
#include "real.h"
#include "rtsp_session.h"
#include "RtspCliEx.h"
#endif

#ifdef USE_CRAWLER_H
#include "Crawler.h"
#endif

#ifdef USE_RAWSERVER_H
#include "RawServer.h"
#endif

#ifdef USE_DBG_H
#include "dbg.h"
#endif

#ifdef USE_SOCK_PROXY_H
#include "SockProxy.h"
#endif

#ifdef USE_TASK_H
#include "Task.h"
#endif

#ifdef USE_TASKMGR_H
#include "TaskMgr.h"
#endif


#define MAX_MSG_SIZE 	1024

#endif
