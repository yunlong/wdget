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

#ifndef __HTTP_CLI_H__
#define __HTTP_CLI_H__

/* Some status code validation macros: */
#define H_20X(x)        (((x) >= 200) && ((x) < 300))
#define H_PARTIAL(x)    ((x) == HTTP_PARTIAL_CONTENTS)
#define H_REDIRECTED(x) (((x) == HTTP_MOVED_PERMANENTLY) || ((x) == HTTP_MOVED_TEMPORARILY))

/* HTTP/1.1 status codes from RFC1945*/
/* Successful 2xx. */
#define HTTP_OK                		200
#define HTTP_CREATED           		201
#define HTTP_ACCEPTED          		202
#define HTTP_NO_CONTENT        		204
#define HTTP_PARTIAL_CONTENTS  		206

/* Redirection 3xx. */
#define HTTP_MULTIPLE_CHOICES  		300
#define HTTP_MOVED_PERMANENTLY 		301
#define HTTP_MOVED_TEMPORARILY 		302
#define HTTP_NOT_MODIFIED      		304

/* Client error 4xx. */
#define HTTP_BAD_REQUEST       		400
#define HTTP_UNAUTHORIZED     		401
#define HTTP_FORBIDDEN         		403
#define HTTP_NOT_FOUND         		404

/* Server errors 5xx. */
#define HTTP_INTERNAL          500
#define HTTP_NOT_IMPLEMENTED   501
#define HTTP_BAD_GATEWAY       502
#define HTTP_UNAVAILABLE       503
#define HTTP_GATEWAY_TIMEOUT   504

#define HTTP_BUF_SIZE 			8192
#define MAX_REDIRECTIONS 		5
#define SAVE_DIR				"savedir/"


#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define timersub(a, b, result)                            \
  do {                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                 \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                  \
    if ((result)->tv_usec < 0) {                          \
      --(result)->tv_sec;                             \
      (result)->tv_usec += 1000000;                       \
    }                                         \
  } while (0)

namespace matrix {

typedef enum  {
    VERB_HEAD,
    VERB_GET,
	VERB_POST,
	VERB_CONNECT,
    VERB_NONE
}THttpVerb;

class THttpCli : public TcpSocket
{
private :	
	Uri * 	   uri;

	THttpMsg * htRequest;
	THttpMsg * htResponse;
	string 	   htmlContent;
	int 	   statusCode;
	string	   statusLine;
public  :
	THttpCli(void);
	~THttpCli(void);

	void initUri(const string& sUrl) 
	{
		if(uri != NULL)
		{
			uri->init(sUrl);
		    if(uri->port() == 0 ) uri->port(80);
		}
	}

public:
	THttpMsg* getHttpResponse(void) { return htResponse; }
	THttpMsg* getHttpRequest(void)  { return htRequest;  }

	string getStatusLine(void) { return statusLine; }

	bool connectRemoteHost(const string& sUrl );
	bool connectProxyHost( Proxy_t& proxy );

	//parse header information from the header content
	int SkipLws(const char *hdr);
	const char * getLocation(void);
    int getStatusCode(void);
    int getAcceptRanges(void);
    const char * getConnectionState(void);
    const char * getCharset(void);

    off_t getContentRange(void);
	off_t getContentLen(void);
    const char * getContentEncoding(void);
    const char * getContentType(void);
    const char * getTransferEncoding(void);
	const char * getAuthStr(void);
    const char * getLastModified(void);
    
	//////////////////////////////////////////
	void buildUserAgentHdr( void );
	void buildConnectionHdr( void );
	void buildContentRangeHdr( void );
	void buildHostHdr(const char * host, int port);
	void buildAcceptHdr( void );
	void buildRangeHdr(off_t StartPos, off_t EndPos);
	void buildRefererHdr(const char * Ref);
	void buildBasicAuthHdr( const char * UserName, const char * Password );
	void buildProxyAuthHdr( const char * UserName, const char * Password );
	void buildPragmaHdr( void );
	////////////////////////////////////////////////////
	int setupHttpSession(const string& sUrl);

	bool redirectUrl(string& redirUrl);
	int fetchHtmlPage(string& sUrl, const string& filename);
	int fetchHtmlFile(const string& LocalFile);
	off_t fetchHtmlChunk(const string& LocalFile);
	////////////////////////////////////////////////////
	int fetchHtmlPage(string& sUrl);
	int fetchHtmlFile(void);
	off_t fetchHtmlChunk(void);
    string& getHtmlContent() { return htmlContent; }
	////////////////////////////////////////////////////////////
	int sendHttpRequest( THttpVerb verb, bool useproxy);
	int recvHttpResponse(void);


    int BufReadChar(char * c);
    int BufPeek(char * c);
	int ReadLine(char *line, int maxsize);

    off_t RecvData(char *buffer, int size, int flags, int timeout );
    off_t SendData(char *buffer, int size, int flags, int timeout );
		
};

}

#endif

