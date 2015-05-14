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

#ifndef __CRAWLER_H__
#define __CRAWLER_H__

#define MIN_WORKER_NUM	2

namespace matrix {

//large enough to hold sina's 437 links 
const unsigned int URL_LEN      = 256;
const int ANCHOR_TEXT_LEN       = 256;
const int MAX_URL_REFERENCES    = 2000;
const int URL_REFERENCE_LEN     = (URL_LEN + ANCHOR_TEXT_LEN) * MAX_URL_REFERENCES * 1 / 2 ;
const int MAX_TAG_NUMBERS		= 10000;

// <img src...>,<script src...> <A href...>, <area src...>
// Keep only <img ...>, <area ...>,<script ...> and <a href ...> tags.
struct RefLink4SE	
{
	char *link;
	char *anchor_text;
	string strCharset;
};

typedef struct{
    int offset;
    int len;
}TagInfo;

typedef struct {
	string url;
	int depth;
} uri_t;

const string DATA_FILE_NAME("webdata.dat");
const string INDEX_FILE_NAME("webdata.idx");
const string UNREACH_HOST_FILE("unreachhost.list");
const string VISITED_FILE("visited.url");
const string UNVISITED_FILE("unvisited.url");
const string URL_MD5_FILE("visitedurl.md5");
const string PAGE_MD5_FILE("visitedpage.md5");
const string IP_BLOCK_FILE("ipblock.list");


// return value of function fetchPage 
// connect failed : return -1;
// RecvFile	failed: return -2;
// Redirect		:	return -3;

class TCrawler;

class TSpiderWorker : public TThread
{
private:
	THttpCli* htcli;
	Uri * U;
	TCrawler* crawler;
public:
	TSpiderWorker(TCrawler* owner);
	~TSpiderWorker(void);
    void Run( void );
    bool Init( void );
    int  Exit( void );
    
    // parse hyperlinks from the page content
    int parseHyperlinks(int depth);
    bool IsFilterLink(string plink);
	
};


class TCrawler
{
friend class TSpiderThread;

public:
    pthread_mutex_t uq_mutex;
    pthread_cond_t uq_cond;
	std::list<uri_t> uriQue;

public:
	TMutex*	ht_mutex;
	HashTable * htable;

	TMutex* ur_mutex;
	std::vector<string> unreachedHosts;
	size_t ur_pos;

	TMutex* uv_mutex;
	std::vector<string> unvisitedUrls;
	size_t uv_pos;
	
	TMutex* vu_mutex;
	std::vector<string> visitedUrls;
	size_t vu_pos;
	
	TMutex*	dns_mutex;
	std::map<string, string> dnsCache;

public:
	unsigned int fseq;
	TMutex* seq_mutex;

public:
	TSpiderWorker *WorkerQueue[MIN_WORKER_NUM];
	
public:
	TCrawler();
	~TCrawler();
public:
    bool m_running;
    void start(void);
    void stop(void);

	///////////////////////////////////////
	uri_t fetchUrl(void);
	int checkUrl(Uri& U);

	int getfileseq();
	void dispUriQue(void);
	///////////////////////////////////////
	void saveUnreachHost(string host);
	void saveUnvisitedUrl(string url);
	void saveVisitedUrl(string url);
    void saveVisitedUrlMD5(string md5);
    void saveVisitedPageMD5(string md5);
 

};

}

#endif

