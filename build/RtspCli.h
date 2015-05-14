#ifndef _RTSPCLI_H
#define _RTSPCLI_H

#include "Types.h"

/*
RTSP Message {
	Message Types
	Message Headers
	Message Body
	Message Length
}


Request = { 
	Request-Line
	{
  		general-header
  		request-header
  		entity-header
	}
	CRLF
	[ message-body ]
}
		   
Request-Line = Method SP Request-URI SP RTSP-Version CRLF

Method = { 
	DESCRIBE;
	GET PARAMETER ; 
	OPTIONS ;
	PAUSE ;
	PLAY ; 
	PING ; 
	REDIRECT ;
	SETUP ; 
	SET PARAMETER ; 
	TEARDOWN ;
	
	extension-method = token
	{
		Request-URI = * / absolute URI
		RTSP-Version = RTSP / 1*DIGIT . 1*DIGIT
	}
}

general-header = {
	Cache-Control ;
	Connection ;
	CSeq ; 
	Date ; 
	Timestamp ; 
	Via ; 
}


request-header = { 
	Accept;
	Accept-Encoding ;
	Accept-Language ;
	Authorization ;
	Bandwidth ; 
	Blocksize ; 
	From ; 
	If-Modified-Since ;
	Proxy-Require ; 
	Range ; 
	Referer ; 
	Require ; 
	Scale ; 
	Session ; 
	Speed ; 
	Supported ; 
	Transport ; 
	User-Agent ; 
}
			

Response = {
	Status-Line ; Section 7.1
	{ 
	   general-header ;
	   response-header ; 
	   entity-header 
	}
	CRLF
	[ message-body ] ; 
}

Status-Line = RTSP-Version SP Status-Code SP Reason-Phrase CRLF

Status-Code 
{
	1xx: Informational - Request received, continuing process
	2xx: Success - The action was successfully received, understood, and accepted
	3rr: Redirection - Further action must be taken in order to complete the request
	4xx: Client Error - The request contains bad syntax or cannot be fulfilled
	5xx: Server Error - The server failed to fulfill an apparently valid request
}

Status-Code = {	
	100 ; Continue
	200 ; OK
	201 ; Created
	250 ; Low on Storage Space
	300 ; Multiple Choices
	301 ; Moved Permanently
	302 ; Moved Temporarily
	303 ; See Other
	304 ; Not Modified
	305 ; Use Proxy
	350 ; Going Away
	351 ; Load Balancing
	400 ; Bad Request
	401 ; Unauthorized
	402 ; Payment Required
	403 ; Forbidden
	404 ; Not Found
	405 ; Method Not Allowed
	406 ; Not Acceptable
	407 ; Proxy Authentication Required
	408 ; Request Time-out
	410 ; Gone
	411 ; Length Required
	412 ; Precondition Failed
	413 ; Request Entity Too Large
	414 ; Request-URI Too Large
	415 ; Unsupported Media Type
	451 ; Parameter Not Understood
	452 ; reserved
	453 ; Not Enough Bandwidth
	454 ; Session Not Found
	455 ; Method Not Valid in This State
	456 ; Header Field Not Valid for Resource
	457 ; Invalid Range
	458 ; Parameter Is Read-Only
	459 ; Aggregate operation not allowed
	460 ; Only aggregate operation allowed
	461 ; Unsupported transport
	462 ; Destination unreachable
	500 ; Internal Server Error
	501 ; Not Implemented
	502 ; Bad Gateway
	503 ; Service Unavailable
	504 ; Gateway Time-out
	505 ; RTSP Version not supported
	551 ; Option not supported
	extension-code
	{
		extension-code = 3DIGIT
		Reason-Phrase = *< TEXT, excluding CR, LF >
	}
}

response-header = {
	Accept-Ranges ;
	Location ; 
	Proxy-Authenticate ; 
	Public ; 
	Range ; 
	Retry-After ; 
	RTP-Info ;
	Scale ; 
	Session ; 
	Server ; 
	Speed ; 
	Transport ; 
	Unsupported ; 
	Vary ; 
	WWW-Authenticate ;
}

entity-header = { 
	Allow ; 
	Content-Base ; 
	Content-Encoding ; 
	Content-Language ;
	Content-Length ;
	Content-Location ; 
	Content-Type ; 
	Expires ; 
	Last-Modified ; 
	extension-header
	extension-header = message-header

}

 */

enum RtspMethod 
{
    opNONE,
    opANNOUNCE,
    opDESCRIBE,
    opGETPARAM,
    opOPTIONS,
    opPAUSE,
    opPLAY,
    opRECORD,
    opREDIRECT,
    opSETUP,
    opSETPARAM,
    opTEARDOWN,
    opLAST
};

enum RtspMsgType 
{
    mtNONE,
    mtREQUEST,
    mtRESPONSE,
    mtLAST
};


/* RtspHdr List */

class TRtspHdr
{
private :
    char * Key;
    char * Val;
public:
    TRtspHdr(void);
    char * GetKey( void ) const;
    char * GetVal( void ) const;
    void   SetVal( const char * strVal );
};

enum RtspErr
{
    RTSPE_NONE,
    RTSPE_CLOSED,
    RTSPE_NOTRAN,
    RTSPE_MAX
};

class TRtspMsg
{
private :    
	int    	RtspVer;  
	int		MsgType;
	char * 	MsgHdr;
	char * 	MsgBody;
public	:
    TRtspMsg( void );
    virtual ~TRtspMsg( void );
	
    virtual RtspMsgType GetType( void ) const;

    size_t GetHdrLen( void ) const;
    size_t GetBufLen( void ) const;

    BYTE GetAt( UINT nPos ) const;
    void SetAt( UINT nPos, BYTE by );

    void GetRtspVer( UINT* puMajor, UINT* puMinor ) const;
    void SetRtspVer( UINT uMajor, UINT uMinor );

    size_t GetHdrCount( void ) const;
    char *  GetHdr( const char * strKey ) const;
    TRtspHdr * GetHdr( UINT nIndex ) const;
    void      SetHdr( const char * strKey, const char * strVal );
    void      SetHdr( const TRtspHdr& hdrNew );

    PBYTE     GetBuf( void ) const;
    void      SetBuf( CPBYTE buf, size_t nLen );
};



class TRtspRequestMsg : public TRtspMsg
{
protected:
  	RtspMethod	Method
    char *         Url;
public:
    TRtspRequestMsg( void );
    
	virtual ~TRtspRequestMsg( void );

    virtual RtspMsgType GetType( void ) const;

    RtspVerb  GetVerb( void ) const;
    CPCHAR    GetVerbStr( void ) const;
    void      SetVerb( RtspVerb verb );
    void      SetVerb( CPCHAR szVerb );

    CPCHAR    GetUrl( void ) const;
    void      SetUrl( const char *& strUrl );
};

class TRtspResponseMsg : public TRtspMsg
{
private	:
    UINT            StatusCode;
    char *         	StatusMsg;
public	:
    TRtspResponseMsg( void );
	virtual ~TRtspResponseMsg( void );
    virtual RtspMsgType GetType( void ) const;
    UINT            GetStatusCode( void ) const;
    const char *&  GetStatusMsg( void ) const;
    void            SetStatus( UINT nCode, CPCHAR szMsg = NULL );
};



enum ReadState 
{
	rsFail,         // Unrecoverable error occurred
	rsSync,         // Trying to resync
	rsReady,        // Waiting for a command
	rsPkt,          // Reading interleaved packet
	rsCmd,          // Reading command (request or response line)
	rsHdr,          // Reading headers
	rsBody,         // Reading body (entity)
	rsDispatch,     // Fully formed message
	rsREADSTATE_LAST
};


class TRtspCli
{
private:
    
	TSocketCli *   Skcp;
	
	int  SendSeq;
	int  RecvSeq;
	
public:
    TRtspCli(void);
    virtual ~TRtspCli(void);


	void Play(void);
	void Rewind(void);
	void Stop(void);
	void Continue(void);
    
	void   SendRequest(TRtspMsg * pmsg);
    
	void SendDescribeRequest(TRtspRequestMsg * pmsg);
    void SendAnnounceRequest(TRtspRequestMsg * pmsg);
    void SendGetParamRequest(TRtspRequestMsg * pmsg);
    void SendSetParamRequest(TRtspRequestMsg * pmsg);
    void SendOptionsRequest(TRtspRequestMsg * pmsg);
    void SendPauseRequest(TRtspRequestMsg * pmsg);
    void SendPlayRequest(TRtspRequestMsg * pmsg);
    void SendRecordRequest(TRtspRequestMsg * pmsg);
    void SendRedirectRequest(TRtspRequestMsg * pmsg);
    void SendSetupRequest(TRtspRequestMsg * pmsg);
    void SendTeardownRequest(TRtspRequestMsg * pmsg);
    void SendExtensionRequest(TRtspRequestMsg * pmsg);

};

#endif 
