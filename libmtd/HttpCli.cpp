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


the following from RFC
The chunked encoding modifies the body of a message in order to
transfer it as a series of chunks, each with its own size indicator,
followed by an OPTIONAL trailer containing entity-header fields. This
allows dynamically produced content to be transferred along with the
information necessary for the recipient to verify that it has
received the full message.

		Chunked-Body   = *chunk
						 last-chunk
						 trailer
						 CRLF

		chunk          = chunk-size [ chunk-extension ] CRLF
					     chunk-data CRLF

		chunk-size     = 1*HEX
		last-chunk     = 1*("0") [ chunk-extension ] CRLF

		chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
		chunk-ext-name = token
		chunk-ext-val  = token | quoted-string
		chunk-data     = chunk-size(OCTET)
		trailer        = *(entity-header CRLF)

The chunk-size field is a string of hex digits indicating the size of
the chunk. The chunked encoding is ended by any chunk whose size is
zero, followed by the trailer, which is terminated by an empty line.

The trailer allows the sender to include additional HTTP header
fields at the end of the message. The Trailer header field can be
used to indicate which header fields are included in a trailer

***/

#define USE_URI_H
#define USE_UTILS_H
#define USE_SOCKET_H
#define USE_SOCK_PROXY_H
#define USE_HTTPMSG_H
#define USE_HTTPCLI_H

#include "matrix.h"

namespace matrix {

//////////////////////////class THttpCli Implement /////////////////////
THttpCli :: THttpCli(void) : TcpSocket()
{
	uri = new Uri();
 	htRequest = new THttpMsg;
	htResponse = new THttpMsg;
}


THttpCli :: ~THttpCli(void)
{
	if(uri != NULL) delete uri;
	if(htRequest != NULL) delete htRequest;
	if(htResponse != NULL) delete htResponse;
//	closeSocket();
}

int THttpCli :: BufReadChar(char *c)
{
    int retval = Recv(c, 1, 0, 100);

    if (retval <= 0)
        return retval;
    return 1;
}

int THttpCli :: BufPeek(char *c)
{
    int retval = Recv(c, 1, MSG_PEEK, 180);

    if (retval <= 0)
        return retval;
    return 1;
}

int THttpCli :: ReadLine(char *line, int maxsize)
{
    int i, retval;
    char c;
    char *ptr;

    ptr = line;

    for(i = 1; i < maxsize; i++)
    {
       // retval = read(getHandle(), &c, 1);
		retval = BufReadChar(&c);
        if (retval < 0)
            return retval;  //HERR

        if (retval == 0)
        {
            if(i == 1)
                return 0; //HEOF
            else
                break;
        }

        *ptr = c;
        ptr++;
        if(c == '\n')
            break;
    }

    *ptr = '\0';

    if(i == maxsize)
        return i - 1;
    else
        return i;
}

/* function to Parse the http headers from the socket */
int THttpCli :: recvHttpResponse(void)
{
	int ret;
	char *ptr, *strKey, *strVal;
    char linebuf[MAX_LINE_LEN];

	htResponse->ClearAllHdr();

	/***	
	rline_t   rline;
	readLineExInit(getHandle(), linebuf, MAX_LINE_LEN, &rline);
	ret = readLineEx(&rline);
	***/
	
	ret = ReadLine(linebuf, MAX_LINE_LEN);
	if(ret < 0) return ret;
	if(ret == 0) return -1;

    statusLine = linebuf;
//	cout << statusLine << endl;

	ptr = linebuf;
	while(*ptr == ' ') ptr++;

	// skip the http version info
	while(*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n') ptr++;
	if(*ptr != ' ') return -1;
	while(*ptr == ' ') ptr++;

	statusCode = atoi(ptr);
//	ShowMsg("%s", linebuf);
	do
	{	
	//	ret = readLineEx(&rline);
	    ret = ReadLine(linebuf, MAX_LINE_LEN);
		
		if(ret < 0) return ret;
		if(ret == 0) return -1;
		
		ptr = linebuf;
		while(*ptr == ' ') ptr++;
		
		strKey = ptr;
		
		if(*ptr == '\r' || *ptr == '\n') break;
		
		while(*ptr != '\0' && *ptr != ':' && *ptr != '\r' && *ptr != '\n') ptr++;
		
		if(*ptr == ':')
		{
			*ptr = '\0';
			ptr++;
		}
		else
		{
			return -1;
		}

		while(*ptr == ' ') ptr++;
		
		strVal = ptr;
				
		while(*ptr != '\0' && *ptr != '\r' && *ptr != '\n') ptr++;
		
		*ptr = '\0';
		
		htResponse->SetHdr(strKey, strVal);

	} while(true);

	
//	htResponse->ShowAllHttpHdr();
	/*
	if (H_20X(statusCode))
		return 0;
	if (H_REDIRECTED(statusCode))
		return statusCode;
	if(statusCode == HTTP_MULTIPLE_CHOICES)
    	return 0;
	*/

	//Error statusCode
	return statusCode;

}

bool THttpCli :: connectProxyHost(Proxy_t& proxy)
{
	char *ipaddr = NULL;
	char * proxyipaddr = NULL;
	
	bool ret = false;
	int len = 0;
	string addr;
	string host;
	
	int ch = 0;
	char *ip = NULL;
	
	string sAuths;
	string sDatas;
	uint8_t bPack = 0;
	uint32_t dwAddr = 0;
	
	
	switch (proxy.type)
	{
	case PROXY_HTTP:
		{
		proxyipaddr = host2str(proxy.host.c_str());
		if(proxyipaddr == NULL)
			return false;

		ShowMsg("Connecting to proxy server %s:%d", proxyipaddr, proxy.port);
		ret = Connect(proxyipaddr, proxy.port, 100);
		if( ret != true )
		{
			ShowMsg("I HAHA %s %d%", proxy.name.c_str(), proxy.port);
			return ret;	
		}			
		ShowMsg("Connecting to Proxy Server Successfully");
		}
		break;
		
	case HTTP_CONNECT:
		{
		ipaddr = host2str( uri->hostname().c_str() );	
		proxyipaddr = host2str(proxy.host.c_str());
		
		ShowMsg("Connecting to proxy server %s:%d", proxyipaddr, proxy.port);
		ret = Connect(proxyipaddr, proxy.port, 100);
		if(ret != true )
		{
			ShowMsg("I HAHA %s %d%", proxy.name.c_str(), proxy.port);
			return ret;	
		}			
		ShowMsg("Connecting to Proxy Server Successfully");

		if(proxy.auth)
		{
			if(!proxy.user.empty())
			{
				buildProxyAuthHdr(proxy.user.c_str(), proxy.pass.c_str());
			}
		}
		
    	sendHttpRequest(VERB_GET, true);
		ret = recvHttpResponse();

		break;
		}

	case PROXY_SOCK4:
		{
		TSockProxy m_socks4(this);		
		
		dwAddr = host2ip( uri->hostname().c_str() );	
		proxyipaddr = host2str( proxy.host.c_str() );
		
		ShowMsg("Connecting to proxy server %s:%d", proxyipaddr, proxy.port);
		ret = Connect(proxyipaddr, proxy.port, 100);
		if(ret != true)
			return ret;	
		ShowMsg("Connecting to Proxy Server Successfully");

		ret = m_socks4.Send4Req(SOCKS_CMDS_CONNECT,	proxy.port, 
											dwAddr, proxy.user.c_str());
		if(ret <= 0) return false;
			
		ret = m_socks4.RecvPacket(SOCKS_PACK_4REP);
		if(ret <= 0) return false;
			
		ret = m_socks4.RecvOK();
		if(ret <= 0) return false;
		break;
		}
	
	case PROXY_SOCK5:
		{
		TSockProxy m_socks5(this);	
			
		dwAddr = host2ip( uri->hostname().c_str() );	
		proxyipaddr = host2str( proxy.host.c_str() );
		
		ShowMsg("Connecting to proxy server %s:%d", proxyipaddr, proxy.port);
		ret = Connect(proxy.host.c_str(), proxy.port, 100);
		if( ret != true )
			return ret;	
		
		ShowMsg("Connecting to Proxy Server Successfully");
			
		sDatas = "";
		if (proxy.auth)
		{
			ch = SOCKS_AUTH_NONE;
			sDatas += (char)ch;
			ch = SOCKS_AUTH_PASSWD;
			sDatas += ch;
		}
		else
		{
			ch= SOCKS_AUTH_NONE;
			sDatas += ch;
		}
			
		len = sDatas.length();
		ret = m_socks5.Send5AuthReq(len, sDatas.c_str());
		if(ret <= 0) return false;

		ret = m_socks5.RecvPacket(SOCKS_PACK_5AUTHREP);
		if(ret <= 0) return false;

		ret = m_socks5.RecvOK();
		if(ret <= 0) return false;

		switch (m_socks5.RecvBufs()->sock5auth.auths)
		{
		case SOCKS_AUTH_NONE:
			break;
		case SOCKS_AUTH_PASSWD:
			ret = m_socks5.Send5PassReq( proxy.user.c_str(), proxy.pass.c_str());
			if(ret <= 0) return false;

			ret = m_socks5.RecvPacket(SOCKS_PACK_5PASSREP);
			if(ret <= 0) return false;

			ret = m_socks5.RecvOK();
			if(ret <= 0) return false;

			break;
		case SOCKS_AUTH_GSSAPI:
		case SOCKS_AUTH_CHAP:
		case SOCKS_AUTH_UNKNOWN:
		default:			
		break;
		}
		
		sDatas = "";
		if (dwAddr != INADDR_NONE)
		{
			bPack = SOCKS_ADDR_IPV4ADDR;
			sDatas += (char)((dwAddr  ) &0x000000ff);
			sDatas += (char)((dwAddr >> 8 ) &0x000000ff);
			sDatas += (char)((dwAddr >> 16) &0x000000ff);
			sDatas += (char)((dwAddr >> 24) &0x000000ff);
		}
		else
		{
			bPack = (int)SOCKS_ADDR_HOSTNAME;
			ch = strlen( uri->hostname().c_str() );
			sDatas += ch;
			sDatas += uri->hostname().c_str();
		}
			
		ret = m_socks5.Send5Req(SOCKS_CMDS_CONNECT,
							bPack, proxy.port, sDatas.c_str());
							
		if(ret <= 0) return false;

		ret = m_socks5.RecvPacket(SOCKS_PACK_5REP);
		if(ret <= 0) return false;

		ret = m_socks5.RecvOK();
		if(ret <= 0) return false;
		break;
		}
	}
	
	return true;
}

bool THttpCli :: connectRemoteHost(const string& sUrl)
{
	bool ret = false;

    uri->init(sUrl);
	if(uri->port() == 0 ) uri->port(80);

    char* ipaddr =  host2str( uri->hostname().c_str() );
	int dport = uri->port();

	ShowMsg("Connecting to server %s ......", uri->hostname().c_str() );

	string scheme =  uri->scheme();
	strToLower( scheme, strlen(scheme.c_str()) ) ;
		
	if(scheme == "http" )
	{
		ret = Connect(ipaddr, dport, 100);
		if(ret == true) { ShowMsg("Connecting to remote server %s:%d successfully", ipaddr, dport); }
		enableSSL(false);	
	}
	
	if(scheme == "https")
	{
		ret = Connect(ipaddr, 443, 100);
		if(ret == true) { ShowMsg("Connecting to remote server %s:%d successfully", ipaddr, dport); }
	
		enableSSL(true);
   		ret = SSL_Connect();
		if( ret != true )
		{
			ShowMsg("SSL Connecting failure");
			return ret;
		}
		ShowMsg("SSL Connecting Successfully");
		if(ret == true) { ShowMsg("Connecting to remote server %s:%d successfully", ipaddr, dport); }
	}

	return ret;
	
}

off_t THttpCli :: getContentLen(void)
{
	if(htResponse != NULL)
	{
		string strval = htResponse->GetHdr("Content-Length");
		off_t Len;
		if(!strval.empty())
		{
			const char * ptr = strval.c_str();
			for (Len = 0; isdigit(*ptr); ptr++)
				Len = 10 * Len + (*ptr - '0');
			return Len;
		}
	}	
	return -1;
}

int THttpCli :: getAcceptRanges(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Accept-Ranges");
		if(!strval.empty())
        {
        	const char * ptr = strval.c_str();
            if (strstr(ptr, "none"))
                return 0;
        }
    }
    return 1;
}

int THttpCli :: getStatusCode(void)
{
	return statusCode;
}

const char * THttpCli :: getConnectionState(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Connection");
		if(!strval.empty())
        	return strval.c_str();
    }
    return NULL;
}

int THttpCli :: SkipLws(const char *hdr)
{
    int i;
    for (i = 0; *hdr == ' ' || *hdr == '\t' || *hdr == '\r' || *hdr == '\n'; ++hdr)
    {
        ++i;
    }
    return i;
}

off_t THttpCli :: getContentRange(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Content-Range");
        off_t Len;
		if( !strval.empty() )
        {
        	const char * ptr = strval.c_str();
            if(!strncasecmp(ptr, "Bytes", 5))
            {
                ptr += 5;
                ptr += SkipLws(ptr);
                if (!*ptr)
                    return -1;
            }
            for (Len = 0; isdigit(*ptr); ptr++)
                Len = 10 * Len + (*ptr - '0');
            return Len;
        }
    }
    return -1;
}

const char * THttpCli :: getContentType(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Content-Type");
		if( !strval.empty() )
        	return strval.c_str();
    }
    return NULL;

}

const char * THttpCli :: getContentEncoding(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Content-Encoding");
		if( !strval.empty() )
        	return strval.c_str();
    }
    return NULL;
}

const char * THttpCli :: getLastModified(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Last-Modified");
		if( !strval.empty() )
        	return strval.c_str();
    }
    return NULL;
}

const char * THttpCli :: getCharset(void)
{
	if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Content-Type");
        if( !strval.empty() )
        {
            unsigned int pos = strval.find("charset=");
            if( pos != string::npos)
                return strval.c_str() + pos + sizeof("charset=") - 1 ;
        }
    }
    return NULL;
}

const char * THttpCli :: getTransferEncoding(void)
{
    if(htResponse != NULL)
    {
        string strval = htResponse->GetHdr("Transfer-Encoding");
		if( !strval.empty() )
        	return strval.c_str();
    }
    return NULL;
}

const char * THttpCli :: getLocation(void)
{
	if(htResponse != NULL)
	{
		string strval = htResponse->GetHdr("Location");
		const char * ptr = strval.c_str();
		if(ptr != NULL) 
			return ptr;
	}
	return NULL;
}

int THttpCli :: sendHttpRequest( THttpVerb verb, bool useproxy )
{
	int pos = 0, nHdr;
	char linebuf[MAX_LINE_LEN];
	char szBuffer[HTTP_BUF_SIZE];
	THttpHdr * pHdr;
	string reqpath;

	if(useproxy)
	{
		reqpath = uri->unparse();	
	}
	else
	{
		reqpath = uri->path();
		if( uri->existsQuery() )
			reqpath += "?" + uri->query();
        if(uri->existsFragment())
			reqpath += "#" + uri->fragment();

	}

	if(htRequest != NULL && htRequest->GetHdrCount() > 0)
	{
		if(verb == VERB_GET)
		{
			if(useproxy)
			{
				snprintf(linebuf, MAX_LINE_LEN, "GET %s HTTP/1.0\r\n", reqpath.c_str());
				snprintf(szBuffer, 1024, "GET %s HTTP/1.0\r\n", reqpath.c_str());
				pos += 15 + strlen( reqpath.c_str() );
			}
			else
			{		
				//cout << u->query().c_str() << endl;
				snprintf(linebuf, MAX_LINE_LEN, "GET %s HTTP/1.0\r\n", reqpath.c_str() );
				snprintf(szBuffer, 1024, "GET %s HTTP/1.0\r\n",reqpath.c_str() );
				pos += 15 + strlen( reqpath.c_str() );
			}
		}
		
		if(verb == VERB_CONNECT)
		{
			if(useproxy)
			{	
				char sites[MAX_LINE_LEN];
				snprintf(sites, sizeof(sites) - 2, "%s:%u", reqpath.c_str(), uri->port());
				snprintf(linebuf, MAX_LINE_LEN, "CONNECT %s HTTP/1.0\r\n", sites);
				snprintf(szBuffer, 1024, "CONNECT %s HTTP/1.0\r\n", sites);
				pos += 19 + strlen(sites);
			}
			else
			{
				snprintf(linebuf, MAX_LINE_LEN, "CONNECT %s HTTP/1.0\r\n", reqpath.c_str() );
				snprintf(szBuffer, 1024, "CONNECT %s HTTP/1.0\r\n", reqpath.c_str() );
				pos += 19 + strlen(reqpath.c_str() );
			}
		}

		
		if(verb == VERB_HEAD)
    	{
			if(useproxy)
			{
				snprintf(linebuf, MAX_LINE_LEN, "GET %s HTTP/1.0\r\n", reqpath.c_str() );
				snprintf(szBuffer, 1024, "GET %s HTTP/1.0\r\n", reqpath.c_str() );
				pos += 15 + strlen(reqpath.c_str() );
			}
			else
			{
				snprintf(linebuf, MAX_LINE_LEN, "GET %s HTTP/1.0\r\n", reqpath.c_str() );
				snprintf(szBuffer, 1024, "GET %s HTTP/1.0\r\n", reqpath.c_str() );
				pos += 15 + strlen(reqpath.c_str() );
			}
		}
	
		statusLine = linebuf;
		nHdr = htRequest->GetHdrCount();
    	for(int i = 0; i < nHdr; i++)
		{
			pHdr = htRequest->GetHdr( i );
	        snprintf(szBuffer + pos, MAX_LINE_LEN, "%s: %s\r\n", 
						pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
			pos += htRequest->GetHdrLen(i);
    	}
		sprintf(szBuffer + pos, "\r\n");
	}
	
//	ShowMsg("%s\n", szBuffer);
	int retval = Send(szBuffer, strlen(szBuffer));
//	int retval = SendData(szBuffer, strlen(szBuffer), 0, 100);
	return retval;
}

bool THttpCli :: redirectUrl(std::string& redirUrl)
{
    int status = setupHttpSession(redirUrl);
		    
    if (H_REDIRECTED(status))
    {    
        int cnt = 0; 
retry:
        redirUrl = getLocation();  
        closeSocket();
        int status = setupHttpSession( redirUrl.c_str() );
		if ( H_REDIRECTED(status) && cnt < 5 )
		{    
			cnt++;
			goto retry;
		}    
	}    

	if (H_20X(status))
	{    
	}    

	if(status == HTTP_MULTIPLE_CHOICES)
	{    
	}    


	// Error StatusCode
	switch (status)
	{    
	case HTTP_UNAUTHORIZED      :    
	case HTTP_NOT_FOUND         :
	case HTTP_INTERNAL          :
	case HTTP_NOT_IMPLEMENTED   :
	case HTTP_BAD_GATEWAY       :
	case HTTP_UNAVAILABLE       :
	case HTTP_GATEWAY_TIMEOUT   :
		return false;
		//return -4;
	}    

	return true;
}

int THttpCli :: setupHttpSession(const string& sUrl)
{
	int ret = 0;
    char LineBuf[MAX_LINE_LEN];


    bool status = connectRemoteHost( sUrl );
    
    if(status == true) 
	{
    	ShowMsg("Connecting to remote server %s:%d successfully", uri->hostname().c_str(), uri->port());
	}
    else
	{
    	ShowMsg("Connecting to remote server %s:%d failured", uri->hostname().c_str(), uri->port() );
    	return -1;
	}

    if (htRequest != NULL)
    {
        memset(LineBuf, 0, MAX_LINE_LEN);
        if(uri->port() != 80)
            snprintf(LineBuf, MAX_LINE_LEN, "%s:%d", uri->hostname().c_str(), uri->port() );
        else
            snprintf(LineBuf, 1024, "%s", uri->hostname().c_str() );

        htRequest->SetHdr("Host", LineBuf);
		htRequest->SetHdr("Referer", uri->unparse().c_str() );
   		htRequest->SetHdr("Accept", "*/*"); 
        htRequest->SetHdr("User-Agent", "CrowdSpider");
		htRequest->SetHdr("Connection", "Close");
		
	//	htRequest->SetHdr("Connection", "Keep-Alive");
	
		if( !uri->user().empty() && !uri->password().empty() )
		{
			string authstr;
			authstr.append(uri->user());
			authstr.append(":");
			authstr.append(uri->password());
//			cout << authstr << endl;
			//sprintf( authstr, "%s:%s", u->user().c_str(), u->password().c_str() ); 
			htRequest->SetHdr("Authorization", "Basic " + Base64Encode( authstr.c_str() ) );
		}
    }

    sendHttpRequest(VERB_GET, false);

	int retcode  = recvHttpResponse();
	return retcode;

}

int THttpCli :: fetchHtmlPage(string& sUrl)
{
	int ret = 0;

	bool redirok = redirectUrl(sUrl);
	if(redirok == false) {	return -4; }

	const char * strval = getTransferEncoding();

	if(strval != NULL)
	{
		if( strcasecmp(strval, "chunked") == 0 )
		{
			ret = fetchHtmlChunk();
		}
		
		if(ret == -1) 
		{
			return -2;
		}
	}
	else
	{
		ret = fetchHtmlFile();
		if(ret == -1)
		{
			return -2;
		}
	}
	return ret;
}

off_t THttpCli :: fetchHtmlChunk(void)
{
	char linebuf[MAX_LINE_LEN];
	char szBuffer[HTTP_BUF_SIZE];
	bool chunked = true;
	off_t filesize = 0;
	
	htmlContent.clear();
	ReadLine(linebuf, MAX_LINE_LEN);
	filesize = strtol(linebuf, (char**)NULL, 16);
	do 
	{
		
		while (filesize > 0)
		{
			long n = 0;
			if(filesize > sizeof(szBuffer))
				n = safe_read(getHandle(), szBuffer, sizeof(szBuffer) );
			else
				n = safe_read(getHandle(), szBuffer, filesize );

			if( n > 0 )
			{
				htmlContent.append(szBuffer, n);
				filesize -= n;
			}
		}

		if (chunked) 
		{
			ReadLine(linebuf, MAX_LINE_LEN); /* This is a newline */
			ReadLine(linebuf, MAX_LINE_LEN);
			filesize = strtol(linebuf, (char **) NULL, 16);
			if (filesize==0) chunked = 0; /* all done! */
		}

	} while (chunked);

	return 0;
}

int THttpCli :: fetchHtmlFile(void)
{
	off_t totalBytes = 0, BytesRead = 0, Length = 0;
	char szBuffer[HTTP_BUF_SIZE];

	htmlContent.clear();

	setIPTos();
	Length = getContentLen();

	cout << "file size :" << Length << endl;

	if(Length == -1)
	{
    	do
    	{	
		//	BytesRead = RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
         	BytesRead = Recv(szBuffer, HTTP_BUF_SIZE);
        	if(BytesRead > 0)
        	{
			//	ShowMsg("%s", szBuffer);
            	htmlContent.append(szBuffer, BytesRead);
				totalBytes += BytesRead;
        	}
    	}while(BytesRead > 0);

	    if(BytesRead == -1)
    	{
        	if(errno == ETIMEDOUT)
        	{
            	ShowMsg("%s", "TThread timed out");
            	return -1;
        	}
        	return -1;
    	}
	}
	else
	{
		while(Length > 0)
		{
			if(Length < HTTP_BUF_SIZE && Length > 0)
			{
				while(Length > 0)
				{
				//	BytesRead = Recv(szBuffer, HTTP_BUF_SIZE);
					BytesRead = RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
					if((BytesRead == 0 && Length > 0))
					{
						ShowMsg("%s", "Server closed the conenction prematurely!");
						return -1;
					}

					if(BytesRead == -1)
					{
						ShowMsg("%s", "Error receving data!!!");
						if (errno == ETIMEDOUT)
						{
							ShowMsg("%s", "connection timed out");
                       		return -1;
                    	}
                    	return -1;
           		 	}

					htmlContent.append(szBuffer, BytesRead);
					Length -= BytesRead;
					totalBytes += BytesRead;
				}
				break;
			}

			//BytesRead = Recv(szBuffer, HTTP_BUF_SIZE);
			BytesRead = RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
			if ((BytesRead == 0 && Length > 0))
			{
				ShowMsg("%s", "Server closed the conenction prematurely!");
				return -1;
			}

			if(BytesRead == -1)
			{
				ShowMsg("%s", "Error receving data");
				if (errno == ETIMEDOUT)
				{
					ShowMsg("%s", "connection timed out");
					return -1;
				}
				return -1;
			}

			htmlContent.append(szBuffer, BytesRead);
			Length -= BytesRead;
			totalBytes += BytesRead;
		}
	}

	ShowMsg("%lldK bytes received", totalBytes / 1024);
	return totalBytes;
}

int THttpCli :: fetchHtmlPage(string& sUrl, const string& filename)
{
	int ret = 0;

	bool redirok = redirectUrl(sUrl);
	if(redirok == false) {	return -4; }

	const char * strval = getTransferEncoding();

	string savefile;
	savefile.append(SAVE_DIR);
	savefile.append(filename);
	if(strval != NULL)
	{
		if( strcasecmp(strval, "chunked") == 0 )
		{
			ret = fetchHtmlChunk(savefile.c_str());
		}
		
		if(ret == -1) 
		{
			return -2;
		}
	}
	else
	{
		ret = fetchHtmlFile(savefile.c_str());
		if(ret == -1)
		{
			return -2;
		}
	}
	
	return ret;

}

off_t THttpCli :: fetchHtmlChunk(const string& LocalFile)
{
	char linebuf[MAX_LINE_LEN];
	char szBuffer[HTTP_BUF_SIZE];
	bool chunked = true;
	off_t filesize = 0;
	
	FILE * fp = NULL;
	if(!(fp = fopen(LocalFile.c_str(), "wb")))
	{
		ShowMsg("Error opening file %s for writing: %s",LocalFile.c_str(), strerror(errno));
		return -1;
	}

	htmlContent.clear();
	ReadLine(linebuf, MAX_LINE_LEN);
	filesize = strtol(linebuf, (char**)NULL, 16);
	
	do 
	{
		
		while (filesize > 0)
		{
			long n = 0;
			if(filesize > sizeof(szBuffer))
				n = safe_read(getHandle(), szBuffer, sizeof(szBuffer) );
			else
				n = safe_read(getHandle(), szBuffer, filesize );

			if( n > 0 )
			{
				htmlContent.append(szBuffer, n);
				n = safe_write(fileno(fp), szBuffer, n); 
				filesize -= n;
			}
		}

		if (chunked) 
		{
			ReadLine(linebuf, MAX_LINE_LEN); /* This is a newline */
			ReadLine(linebuf, MAX_LINE_LEN);
			filesize = strtol(linebuf, (char **) NULL, 16);
			if (filesize==0) chunked = 0; /* all done! */
		}

	} while (chunked);

	return 0;

}

int THttpCli :: fetchHtmlFile(const string& LocalFile)
{
	off_t totalBytes = 0, BytesRead = 0, Length = 0;
	char szBuffer[HTTP_BUF_SIZE];
	FILE * fp = NULL;
	if(!(fp = fopen(LocalFile.c_str(), "wb")))
	{
		ShowMsg("Error opening file %s for writing: %s",LocalFile.c_str(), strerror(errno));
		return -1;
	}

	fseek(fp, 0L, SEEK_SET);
	setvbuf(fp, NULL, _IONBF, 0);
//	htmlContent.clear();

	setIPTos();
	Length = getContentLen();

	cout << "file size :" << Length << endl;

	if(Length == -1)
	{
    	do
    	{	
		//	BytesRead = RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
         	BytesRead = Recv(szBuffer, HTTP_BUF_SIZE);
        	if(BytesRead > 0)
        	{
			//	ShowMsg("%s", szBuffer);
            	htmlContent.append(szBuffer, BytesRead);
        		
            	if(fwrite(szBuffer, sizeof(char), BytesRead, fp) < BytesRead)
            	{
                	ShowMsg("Error writing to  file %s:%s",LocalFile.c_str(), strerror(errno));
                	fclose(fp);
				}
				totalBytes += BytesRead;
        	}
    	}while(BytesRead > 0);

	    if(BytesRead == -1)
    	{
        	if(errno == ETIMEDOUT)
        	{
            	ShowMsg("%s", "TThread timed out");
            	return -1;
        	}
        	return -1;
    	}
	}
	else
	{
		while(Length > 0)
		{
			if(Length < HTTP_BUF_SIZE && Length > 0)
			{
				while(Length > 0)
				{
				//	BytesRead = Recv(szBuffer, HTTP_BUF_SIZE);
					BytesRead = RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
					if((BytesRead == 0 && Length > 0))
					{
						ShowMsg("%s", "Server closed the conenction prematurely!");
						fclose(fp);
						return -1;
					}

					if(BytesRead == -1)
					{
						ShowMsg("%s", "Error receving data!!!");
						fclose(fp);
						if (errno == ETIMEDOUT)
						{
							ShowMsg("%s", "connection timed out");
                       		return -1;
                    	}
                    	return -1;
           		 	}

					htmlContent.append(szBuffer, BytesRead);
					if(BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
					{
						ShowMsg("%s", "write data to file failed !!!");
						fclose(fp);
						return -1;
					}
					Length -= BytesRead;
					totalBytes += BytesRead;
				}
				break;
			}

			//BytesRead = Recv(szBuffer, HTTP_BUF_SIZE);
			BytesRead = RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
			if ((BytesRead == 0 && Length > 0))
			{
				ShowMsg("%s", "Server closed the conenction prematurely!");
				fclose(fp);
				return -1;
			}

			if(BytesRead == -1)
			{
				ShowMsg("%s", "Error receving data");
				fclose(fp);
				if (errno == ETIMEDOUT)
				{
					ShowMsg("%s", "connection timed out");
					return -1;
				}
				return -1;
			}

			htmlContent.append(szBuffer, BytesRead);
			if (BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
			{
				ShowMsg("%s", "write data to file failed !!!");
				fclose(fp);
				return -1;
			}
			Length -= BytesRead;
			totalBytes += BytesRead;
		}
	}


	fclose(fp);
	ShowMsg("%s : %lldK bytes received", LocalFile.c_str(), totalBytes / 1024);
	
	return totalBytes;
}

off_t THttpCli :: RecvData(char *buffer, int size, int flags, int timeout)
{
    int BytesRead = Recv(buffer, size, flags, timeout);
    return BytesRead;
}

off_t THttpCli :: SendData(char *buffer, int size, int flags, int timeout)
{
    int BytesWritten = Send(buffer, size, flags, timeout);
    return BytesWritten;
}


void THttpCli :: buildUserAgentHdr(void)
{
  	if (htRequest != NULL)
  	{
		htRequest->SetHdr("User-Agent", SOFTWARE_NAME);
	}
}

void THttpCli :: buildHostHdr(const char * host, int port)
{
	char szBuffer[LINE_BUFFER];
  	if (htRequest != NULL)
  	{
		memset(szBuffer, 0, LINE_BUFFER);

       	if(port != 80)
		{
       		snprintf(szBuffer, 1024, "%s:%d", host, port);
       	}
		else
		{
			snprintf(szBuffer, 1024, "%s", host);
       	}
	
		htRequest->SetHdr("Host", szBuffer);
	}
}

void THttpCli :: buildAcceptHdr(void)
{
  	if (htRequest != NULL)
  	{
		htRequest->SetHdr("Accept", "*/*");
	}
}

void THttpCli :: buildRangeHdr(off_t StartPos, off_t EndPos)
{
	char szBuffer[LINE_BUFFER];
  	if (htRequest != NULL)
  	{
		memset(szBuffer, 0, LINE_BUFFER);
		if(EndPos != -1)
     		sprintf(szBuffer, "bytes=%lld-%lld", StartPos, EndPos);
		else
		    sprintf(szBuffer, "bytes=%lld-", StartPos);

		htRequest->SetHdr("Range", szBuffer);
	}
}

void THttpCli :: buildRefererHdr(const char * Ref)
{
	if (htRequest != NULL)
  	{
 		htRequest->SetHdr("Referer", Ref);
  	}
}

void THttpCli :: buildBasicAuthHdr(const char * UserName, const char * Password )
{
	if(htRequest != NULL)
	{	
		char *p1, *p2, *p3;
	
		if( UserName == NULL)
			UserName = 	DEFAULT_USER;
		if(Password == NULL)
			Password = 	DEFAULT_PASSWD;	
		
  		int len = strlen(UserName) + strlen(Password) + 1;
  		int b64len = 4 * ((len + 2) / 3);
  	
  		p1 = new char[len + 1];
  		sprintf(p1, "%s:%s", UserName, Password);
  		p2 = new char[b64len + 1];
  	
  		/* Encode username:passwd to base64. */
  		base64_encode(p1, p2, len);
  		p3 = new char[b64len + 7];
  		sprintf(p3, "Basic %s", p2);
		htRequest->SetHdr("Authorization", p3);

  		delete [] p1;
  		delete [] p2;
		delete [] p3;
	}
}

void THttpCli :: buildProxyAuthHdr( const char * UserName, const char * Password )
{
	if(htRequest != NULL)
	{	
		char *p1, *p2, *p3;
	
		if( UserName == NULL)
			UserName = 	DEFAULT_USER;
		if(Password == NULL)
			Password = 	DEFAULT_PASSWD;	
		
  		int len = strlen(UserName) + strlen(Password) + 1;
  		int b64len = 4 * ((len + 2) / 3);
  	
  		p1 = new char[len + 1];
  		sprintf(p1, "%s:%s", UserName, Password);
  		p2 = new char[b64len + 1];
  	
  		/* Encode username:passwd to base64. */
  		base64_encode(p1, p2, len);
  		p3 = new char[b64len + 7];
  		sprintf(p3, "Basic %s", p2);
		htRequest->SetHdr("Proxy-Authorization", p3);

  		delete [] p1;
  		delete [] p2;
		delete [] p3;
	}

}

void THttpCli :: buildConnectionHdr( void )
{
	if(htRequest != NULL)
	{	
		htRequest->SetHdr("Connection", "Close");
	}
}

void THttpCli :: buildContentRangeHdr( void )
{
	if(htRequest != NULL)
	{	
		htRequest->SetHdr("Content-Range", "bytes=0-");
	}
}

void THttpCli :: buildPragmaHdr( void )
{
	if(htRequest != NULL)
	{	
		htRequest->SetHdr("Pragma", "no-cache");
	}
}

}

