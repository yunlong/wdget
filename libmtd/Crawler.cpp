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

#define USE_INIFILE_H
#define USE_SOCKET_H
#define USE_URL_H
#define USE_HTTPMSG_H
#define USE_HTTPCLI_H
#define USE_UTILS_H

#define USE_THREAD_H
#define USE_LIST_H
#define USE_URL_H
#define USE_HASHTABLE_H
#define USE_CRAWLER_H

#include "matrix.h"

#include "Uri.h"
#include "ParserDom.h"
using namespace htmlcxx;

namespace matrix {

TSpiderWorker :: TSpiderWorker( TCrawler* owner )
{
	crawler = owner;
	htcli = new THttpCli();
}

TSpiderWorker :: ~TSpiderWorker( void )
{
	if(htcli != NULL) delete htcli;
}

bool TSpiderWorker :: Init(void)
{
    return true;
}

int TSpiderWorker :: Exit(void)
{
	pthread_cancel(m_thread);
    return 0;
}

void TSpiderWorker :: Run(void)
{
	//downloaded file named like larbin
	int ret = 0;
	while(crawler->m_running)
	{
		
		//	cout << pthread_self() <<  ":I HATE YOU" << endl;
		uri_t uri = crawler->fetchUrl();
		
		cout << uri.url << endl;

		Uri U( uri.url.c_str() );
		////////////////////////////////////////////////////
		ret = crawler->checkUrl(U);
		//unreachedHosts
		if(ret == -1 ) { continue; }
		//unvisitedUrls
		if(ret == -2 ) { continue; }
		//visitedUrls
		if(ret == -3 ) { continue; }
		////////////////////////////////////////////////////
		
		unsigned long m = crawler->getfileseq();
		char filename[25] = { 0 };
		char * cp = filename + sizeof(filename) - 1;
		while(m > 0)
		{
			*--cp = m % 10 + '0';
			m /= 10;
		}
		int len = strlen(cp);
		while(len++ < 9)
			*--cp = '0';
		*--cp = 'f';
		//cout << cp << endl;
		
		ret = htcli->fetchHtmlPage(uri.url, cp);

		// Connect failed
		if(ret == -1)
		{
	    	crawler->ur_mutex->Lock();
		 	crawler->saveUnreachHost(U.hostname());
	    	crawler->ur_mutex->Unlock();
			continue;
		}
		// RecvFile failed
		if(ret == -2)
		{
	    	crawler->uv_mutex->Lock();
			crawler->saveUnvisitedUrl(uri.url);
	    	crawler->uv_mutex->Unlock();
			continue;
		}

		// Redirect
		if(ret == -3)
		{
		}
		//Page Not Found 404
		if(ret == -4) 
		{
		}
		//////////////////////////////////////////////
	
		//recv html successfully
		if(ret == 0)
		{
			crawler->vu_mutex->Lock();
			crawler->saveVisitedUrl(uri.url);
			crawler->vu_mutex->Unlock();
			parseHyperlinks(uri.depth);
		}
	}
}

int TSpiderWorker::parseHyperlinks(int depth)
{   
	tree<HTML::Node> dom; 	
	HTML::ParserDom parser;

	string html = htcli->getHtmlContent();
	parser.parse(html);
	dom = parser.getTree();
	tree<HTML::Node>::iterator it = dom.begin();
	tree<HTML::Node>::iterator end = dom.end();

	pthread_mutex_lock(&crawler->uq_mutex);
	for (; it != end; ++it)
	{
		if (it->tagName() == "a")
		{
			it->parseAttributes();
			/* filter start with javascript, mailto, https */
			if(it->attribute("href").second.substr(0, 4) == "http")
			{
				uri_t uri;
				uri.url = it->attribute("href").second;
				//cout << uri.url << endl;
			
				Uri U( uri.url.c_str() );
				int ret = crawler->checkUrl(U);
				//unreachedHosts
				if(ret == -1 ) { continue; }
				//unvisitedUrls
				if(ret == -2 ) { continue; }
				//visitedUrls
				if(ret == -3 ) { continue; }

				uri.depth = depth + 1;
				crawler->uriQue.push_back(uri);
			}
		}
	}

//	crawler->dispUriQue();
	pthread_mutex_unlock(&crawler->uq_mutex);
	pthread_cond_broadcast(&crawler->uq_cond);

	return 0;

	//Dump all text of the document
	/****
	it = dom.begin();
	end = dom.end();

	int len = 0;
	std::vector<TagInfo> vt;
	for (; it != end; ++it)
	{
		if ( it->tagName() == "script" || it->tagName() == "style" )
		{
			TagInfo ti;
			ti.offset = it->offset();
			ti.len = it->length();
			vt.push_back(ti);
		}
	}

	string htmlText;
	int start = 0;
	for(int i = 0; i < vt.size(); i++)
	{
	//  cout << vt[i].offset << ":" << vt[i].len << endl;
		htmlText.append(html, start, vt[i].offset - start);
		start = vt[i].offset + vt[i].len;
	}

	if(start < html.size())
		htmlText.append(html, start, html.size() - start);

	parser.parse(htmlText);
	dom = parser.getTree();

	it = dom.begin();
	end = dom.end();
    for (; it != end; ++it)
   	{
    	if ((!it->isTag()) && (!it->isComment()))
        {
        	cout << it->text();
        }
    }
	***/

}

/*
 * Filter spam links
 * If it is, return ture; otherwise false
 */
bool TSpiderWorker::IsFilterLink(string plink)
{
	if( plink.empty() ) return true;
	if( plink.size() > URL_LEN ) return true;

	string link = plink, tmp;
	string::size_type idx = 0;

	strToLower( link, link.length() );
	// find two times following symbols, return false
	tmp = link;
	idx = tmp.find("?");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("?");
		if( idx != string::npos ) 
			return true;
	}

	tmp = link;
	idx = tmp.find("-");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("+");
		if( idx != string::npos ) 
			return true;
	}

	tmp = link;
	idx = tmp.find("&");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("&");
		if( idx != string::npos ) 
			return true;
	}

	tmp = link;
	idx = tmp.find("//");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("//");
		if( idx != string::npos ) 
			return true;
	}

	tmp = link;
	idx = tmp.find("http");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("http");
		if( idx != string::npos ) 
			return true;
	}

	tmp = link;
	idx = tmp.find("misc");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("misc");
		if( idx != string::npos ) 
			return true;
	}

	tmp = link;
	idx = tmp.find("ipb");
	if( idx != string::npos )
	{
		tmp = tmp.substr(idx+1);
		idx = tmp.find("ipb");
		if( idx != string::npos ) 
			return true;
	}

	// robots.txt of http://www.expasy.org/
	const char *filter_str[]={
		"cgi-bin",	"htbin",	"linder",	"srs5",		"uin-cgi",  
		"uhtbin",	"snapshot",	"=+",		"=-",		"script",
		"gate",		"search",	"clickfile",	"data/scop",	"names",
		"staff/",	"enter",	"user",		"mail",	"pst?",
		"find?",	"ccc?",		"fwd?",		"tcon?",	"&amp",
		"counter?",	"forum",	"cgisirsi",	"{",		"}",
		"proxy",	"login",	"00.pl?",	"sciserv.pl",	"sign.asp",
		"<",		">",		"review.asp?",	"result.asp?",	"keyword",
		"\"",		"'",		"php?s=",	"error",	"showdate",
		"niceprot.pl?",	"volue.asp?id",	".css",		".asp?month",	"prot.pl?",
		"msg.asp",	"register.asp", "database",	"reg.asp",	"qry?u",
		"p?msg",	"tj_all.asp?page", ".plot.",	"comment.php",	"nicezyme.pl?",
		"entr",		"compute-map?", "view-pdb?",	"list.cgi?",	"lists.cgi?",
		"details.pl?",	"aligner?",	"raw.pl?",	"interface.pl?","memcp.php?",
		"member.php?",	"post.php?",	"thread.php",	"bbs/",		"/bbs"
	};

	int filter_str_num = 75;

	for(int i=0; i<filter_str_num; i++)
	{
		if( link.find(filter_str[i]) != string::npos)
		return true;
	}	

	return false;
}
            
////////////////////////////////////////////////////
TCrawler :: TCrawler(void)
{
	m_running = false;
    pthread_mutex_init(&uq_mutex, NULL);
    pthread_cond_init(&uq_cond, NULL);
    
    ht_mutex = new TMutex();
	htable = new HashTable(true);

    dns_mutex = new TMutex();
    
    ur_mutex = new TMutex();
    uv_mutex = new TMutex();
    vu_mutex = new TMutex();
    
    ur_pos = 0;
    uv_pos = 0;
    vu_pos = 0;

	seq_mutex = new TMutex();
	fseq = 0;

}

TCrawler :: ~TCrawler(void)
{
    pthread_cond_destroy( &uq_cond );
    pthread_mutex_destroy( &uq_mutex );
    
    delete ht_mutex;
    delete dns_mutex;
    
    delete ur_mutex;
    delete uv_mutex;
    delete vu_mutex;

	delete seq_mutex;

}

void TCrawler :: start(void)
{
	IniFile config("../build/crowdspider.conf");
	std::string startUrl = config.read("crowdspider", "StartUrl");

	m_running = true;
	uri_t uri;
	uri.url = startUrl;
	uri.depth = 0;
	uriQue.push_back(uri);
    for( int i = 0; i < MIN_WORKER_NUM; i++ )
    {
        WorkerQueue[i] = new TSpiderWorker(this);
        WorkerQueue[i]->Create();
    }
}

void TCrawler :: stop(void)
{
	m_running = false;
}

uri_t TCrawler :: fetchUrl(void) 
{
	uri_t entry;
    pthread_mutex_lock(&uq_mutex);
    while( uriQue.empty() )
    {
    	pthread_cond_wait(&uq_cond, &uq_mutex);
    }
    entry = uriQue.front();
    uriQue.pop_front();
    pthread_mutex_unlock(&uq_mutex);
    return entry;
}

int TCrawler :: checkUrl(Uri& U)
{
	std::vector<string>::iterator res;
	/////////////////////////////////////////////////////////////////////////////////////////
	ur_mutex->Lock();
	if(unreachedHosts.size() > ur_pos)
	{
		fstream ur_file( UNREACH_HOST_FILE.c_str(), ios::in | ios::out | ios::app|ios::binary );
		while(ur_pos < unreachedHosts.size() )
			ur_file << unreachedHosts[ur_pos++] << endl;
		ur_file.close();
	}
	res = find( unreachedHosts.begin(), unreachedHosts.end(), U.hostname());            
	if( res != unreachedHosts.end() )
	{
		ur_mutex->Unlock();
		return -1;
	}
	ur_mutex->Unlock();
	/////////////////////////////////////////////////////////////////////////////////////////	
	uv_mutex->Lock();
	if(unvisitedUrls.size() > uv_pos)
	{
		fstream uv_file( UNVISITED_FILE.c_str(), ios::in | ios::out | ios::app|ios::binary );
		while( uv_pos < unvisitedUrls.size() )
			uv_file << unvisitedUrls[uv_pos++] << endl;
		uv_file.close();
	}
	res = find( unvisitedUrls.begin(), unvisitedUrls.end(), U.unparse());
	if( res != unvisitedUrls.end() )
	{
		uv_mutex->Unlock();
		return -2;
	}
	uv_mutex->Unlock();
	///////////////////////////////////////////////////////////////////////////////////////////
	vu_mutex->Lock();
	if(visitedUrls.size() > vu_pos)
	{
		fstream vu_file( VISITED_FILE.c_str(), ios::in | ios::out | ios::app|ios::binary );
		while( vu_pos < visitedUrls.size() )
			vu_file << visitedUrls[vu_pos++] << endl;
		vu_file.close();
	}
	
	res = find( visitedUrls.begin(), visitedUrls.end(), U.unparse());            
	if( res != visitedUrls.end() )
	{
		vu_mutex->Unlock();
		return -3;
	}
	vu_mutex->Unlock();
	///////////////////////////////////////////////////////////////////////////////////////////

/***
	ht_mutex->Lock();
	bool exist = htable->testSet(&U);
	if(exist) 
	{
		ht_mutex->Unlock();
		return -4;
	}
	ht_mutex->Unlock();
	//dns_mutex->Lock();
	//std::map<string, string> dnsCache;
	//dns_mutex->Unlock();
***/

	return 0;
}

void TCrawler :: dispUriQue(void)
{
	std::list<uri_t>::iterator iter = uriQue.begin();
	for( ; iter != uriQue.end(); iter++)
	{
		cout << iter->url << ":" << iter->depth << endl;
	}
}

int TCrawler :: getfileseq(void)
{
	int num = 0;
	seq_mutex->Lock();
	num = fseq++;
	seq_mutex->Unlock();
	return num;
}

void TCrawler::saveUnreachHost(string host)
{
	fstream ur_file( UNREACH_HOST_FILE.c_str(), ios::in | ios::out | ios::app );
	ur_file << host << endl;
	ur_file.close();
}

void TCrawler::saveUnvisitedUrl(string url)
{
	fstream uv_file( UNVISITED_FILE.c_str(), ios::in | ios::out | ios::app );
	uv_file << url << endl;
	uv_file.close();
}

void TCrawler::saveVisitedUrl(string url)
{
	fstream vu_file( VISITED_FILE.c_str(), ios::in | ios::out | ios::app );
	vu_file << url << endl;
	vu_file.close();
}

void TCrawler::saveVisitedUrlMD5(string md5)
{
	fstream vu_md5_file( VISITED_FILE.c_str(), ios::in | ios::out | ios::app );
	vu_md5_file << md5 << endl;
	vu_md5_file.close();
}

void TCrawler::saveVisitedPageMD5(string md5)
{
	fstream vp_md5_file( VISITED_FILE.c_str(), ios::in | ios::out | ios::app );
	vp_md5_file << md5 << endl;
	vp_md5_file.close();
}

}

