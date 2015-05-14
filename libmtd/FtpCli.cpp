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

#define USE_UTILS_H
#define USE_URI_H
#define USE_SOCKET_H
#define USE_SOCK_PROXY_H
#define USE_FTPPARSER_H
#define USE_FTPCLI_H

#include "matrix.h"


/**************** class TFtpCli ******************************/
#define UNIMPLEMENTED_CMD(a)  ((a == 500) || (a == 502) || (a == 504))
#define MAX_MSG_SIZE 1024

namespace matrix {


TFtpCli :: TFtpCli( void ) : TcpSocket()
{
	uri = new Uri();
	DataSkp = new TcpSocket();
	ListenSkp = new ListenSocket();
	
	IsPasv = true;
	FtpBuffer = new char[FTP_BUF_SIZE];
	memset(FtpBuffer, 0, FTP_BUF_SIZE);
	UseProxy = false;
}

TFtpCli :: TFtpCli(Proxy_t proxy) : TcpSocket()
{
	uri = new Uri();
	DataSkp = new TcpSocket();
	ListenSkp = new ListenSocket();
	
	IsPasv = true;
	FtpBuffer = new char[FTP_BUF_SIZE];
	memset(FtpBuffer, 0, FTP_BUF_SIZE);

	UseProxy = true;
	m_proxy = proxy;
}

TFtpCli :: ~TFtpCli(void)
{	
	if(DataSkp != NULL) 
	{
		DataSkp->closeSocket();
		delete DataSkp;
	}		
	
	if(ListenSkp != NULL) 
	{
		ListenSkp->closeSocket();
		delete ListenSkp;
	}

	if(uri != NULL) 
	{
		delete uri;
		uri = NULL;
	}
	
	if(FtpBuffer != NULL)
	{
		delete []FtpBuffer;
		FtpBuffer = NULL;
	}

}
	
void TFtpCli :: closeCtrlConn(void)
{
	if(getHandle() > 0)
	{
		closeSocket();
		ShowMsg("Close FTP Control Connection Channel");
	}
}

void TFtpCli :: closeDataConn(void)
{
	if(DataSkp->getHandle() > 0)
	{
		DataSkp->closeSocket();
		ShowMsg("Close FTP Data Connection Channel");
	}
}

void TFtpCli :: closeListenConn(void)
{
	if(ListenSkp->getHandle() > 0)
	{
		ListenSkp->closeSocket();
		ShowMsg("Close FTP Listen Connection Channel for Port Mode");
	}
}


bool TFtpCli :: connectProxyHost(TSocket * skp, Proxy_t& proxy, int dport)
{
	bool retval;
	char *ipaddr = NULL;
	char * proxyipaddr = NULL;
	
	int ret = -EINVAL;
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
	case PROXY_FTP:
		proxyipaddr = skp->host2str(proxy.host.c_str());
		if(proxyipaddr == NULL)
			return false;

		ShowMsg("Connecting to proxy server %s ......", proxy.host.c_str());
    	retval = skp->Connect(proxyipaddr, proxy.port, 100); 		
		return retval;
		break;

	case PROXY_SOCK4:
		{
		TSockProxy m_socks4(skp);	
			
		dwAddr = skp->host2ip( uri->hostname().c_str() );	
		proxyipaddr = skp->host2str(proxy.host.c_str());
		
		ShowMsg("Connecting to proxy server %s:%d", proxyipaddr, proxy.port);
		retval = skp->Connect(proxyipaddr, proxy.port, 100);
		if(ret != true)
			return ret;	
		ShowMsg("Connecting to Proxy Server Successfully");

		ret = m_socks4.Send4Req(SOCKS_CMDS_CONNECT, dport, dwAddr, proxy.user.c_str());
		if(ret <= 0) return false;
			
		ret = m_socks4.RecvPacket(SOCKS_PACK_4REP);
		if(ret <= 0) return false;
		
		ret = m_socks4.RecvOK();
		if(ret <= 0) return false;
		
		break;
		}	
	case PROXY_SOCK5:
		{
		TSockProxy m_socks5(skp);
		
		dwAddr = skp->host2ip(uri->hostname().c_str());			
		proxyipaddr = skp->host2str(proxy.host.c_str());
			
		ShowMsg("Connecting to proxy server %s:%d", proxyipaddr, proxy.port);
		retval = skp->Connect(proxyipaddr, proxy.port, 100);
		if(retval != true)
			return retval;	
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
			ret = m_socks5.Send5PassReq(proxy.user.c_str(), proxy.pass.c_str());
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
			
		ret = m_socks5.Send5Req(SOCKS_CMDS_CONNECT, bPack, 
											dport, sDatas.c_str());
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

bool TFtpCli :: setupCtrlConn( const string& sUrl )
{
	bool ret = false;

	uri->init(sUrl);
	if(uri->port() == 0 ) 
		uri->port(FTP_PORT);

    if (UseProxy)
    {    
        bool ret = connectProxyHost(this, m_proxy, 21); 
        if(ret != true)
         	return false;
    }    
	else
	{
		char * ipaddr = host2str( uri->hostname().c_str() );
		if(ipaddr == NULL)
			return false;

		ret = Connect(ipaddr, FTP_PORT, 100);

		if(ret == true)
		{
			ShowMsg("Connecting to remote server %s:%d successfully", uri->hostname().c_str(), uri->port());
		}
		else
		{
			ShowMsg("Connecting to remote server %s:%d failured", uri->hostname().c_str(), uri->port() );
			return false;
	    }
	}

	ret = getReply(FtpBuffer);

	if(ret == false ) { return false; cout << "GNU" << endl;}

  	if (*FtpBuffer != '2')
		return false;
	
    return ret;
}

/*
 * this will call bind to return a bound socket then the  ftp server 
 * will be connected with a port request and asked to connect 
 */
bool TFtpCli :: setupPortDataConn(void)
{
	bool ret;
    char Cmd[MAX_MSG_SIZE];
    
	struct sockaddr_in TmpAddr;
	struct sockaddr_in SvrAddr;
    
	socklen_t Len;
	
	char *Ports, *IpAddr;
    
	int CtrlSock, ListenSock;
				
	ret = ListenSkp->Listen(0);

	ListenSock = ListenSkp->getHandle();
    Len = sizeof(SvrAddr);
    if (getsockname(ListenSock, (struct sockaddr *)&SvrAddr, &Len) < 0)
    {
		perror("getsockname");
		closeListenConn();
		return false;
    }

   	Len = sizeof(TmpAddr);
	CtrlSock = getHandle();
    if (getsockname(CtrlSock, (struct sockaddr *)&TmpAddr, &Len) < 0)
    {
		perror("getsockname");
		closeCtrlConn();
		return false;
    }

    IpAddr = (char *)&TmpAddr.sin_addr;
    Ports = (char *)&SvrAddr.sin_port;

#define  UC(b)  (((int)b)&0xff)
    sprintf(
		Cmd, "PORT %d,%d,%d,%d,%d,%d\r\n",
	    UC(IpAddr[0]), UC(IpAddr[1]), 
		UC(IpAddr[2]), UC(IpAddr[3]),
	    UC(Ports[0]), UC(Ports[1])
	);
		
    ret = ftp_port(Cmd);
	if(ret)
	{
		int DataSock;
		string ipaddr;
		UINT16 port;
	
		DataSock = ListenSkp->Accept(ipaddr, port);
		if (DataSock < 0)
			return false;

		DataSkp->attach(DataSock);
	}
	return ret;
}

bool TFtpCli :: setupPasvDataConn(void)
{
	bool ret;

	char dhost[256];
	unsigned short dport;

	unsigned char RemotePasvAddr[6];

	ret = ftp_pasv(RemotePasvAddr);
	if ( ret == false )
	{
    	ShowMsg("Server doesn't seem to support PASV");
		return ret;
	}
		
	sprintf(dhost, "%d.%d.%d.%d", RemotePasvAddr[0], RemotePasvAddr[1],
									RemotePasvAddr[2], RemotePasvAddr[3]);
		
	dport = (RemotePasvAddr[4] << 8) + RemotePasvAddr[5];
		
	ShowMsg("FTP PASV server =%s port=%d", dhost, dport);


	if (UseProxy)
	{
		ret = connectProxyHost(DataSkp, m_proxy, dport);
	   	if(ret != true)
	       return false;
	}
	else
	{
		ret = DataSkp->Connect(dhost, dport, 100);
	
		if (ret != true)
		{
			ShowMsg("Error while connecting, according to servers PASV info");
			return ret;
		}
	}

	return ret;
}

/*
 * return the numeric response of the ftp server by reading the first 3
 * characters in the buffer 
 */
int TFtpCli :: getReplyCode(const char * szBuffer)
{
    char Code[3];
    strncpy(Code, szBuffer, 3);
    return atoi(Code);
}

bool TFtpCli :: getLine(char * szBuffer)
{
    ssize_t iLen, iBuffLen = 0, ret = 0;
    char *szPtr = szBuffer, ch = 0;

	ret = Recv(&ch, 1, MSG_PEEK);
	
    while (iBuffLen < FTP_BUF_SIZE && ret > 0)
    {
		iLen = Recv(&ch, 1, 0);
		
		if (iLen != 1)
	    	return false;
		
		iBuffLen += iLen;
		*szPtr = ch;
		szPtr += iLen;
		if (ch == '\n')
	    	break;		
    }
	
	if (ret == -1)
		return false;
   
	*(szPtr + 1) = '\0';
    
	return true;
}

bool TFtpCli :: getReply(char * szBuffer)
{
    int done = 0;
    memset(szBuffer, 0, FTP_BUF_SIZE);
    while (!done)
    {
		if (getLine(szBuffer) != true)
	    	return false;
		
		ShowMsg(szBuffer);

		(void)strtok(szBuffer, "\r\n");
		
		if (szBuffer[3] != '-' && getReplyCode(szBuffer) > 0)
	    	done = 1;
    }

    return true;
}

bool TFtpCli :: ftp_ascii(void)
{
    bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);
    sprintf(FtpBuffer, "TYPE A\r\n");
    
	if(Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;

    ret = getReply(FtpBuffer);
  	
	if (*FtpBuffer != '2')
		return false;

    return ret;
}

bool TFtpCli :: ftp_size(const char * FileName, off_t * FileSize)
{
	bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);		
    sprintf(FtpBuffer, "SIZE %s\r\n", FileName);
	
	if(Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;
	
    ret = getReply(FtpBuffer);
    if (ret != true)
		return ret;
	
    if (*FtpBuffer == '2')	
    {
		sscanf(FtpBuffer + 3, "%lld", FileSize);
		return true;
    }
 	else if (*FtpBuffer == '5')
    {
		if (strstr(FtpBuffer, "o such file")
	    	|| strstr(FtpBuffer, "o Such File")
	    	|| strstr(FtpBuffer, "ot found")
	    	|| strstr(FtpBuffer, "ot Found"))
	    return false;
    }
}

bool TFtpCli :: ftp_binary(void)
{
    bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);	
    sprintf(FtpBuffer, "TYPE I\r\n");
    
	if(Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;

    ret = getReply(FtpBuffer);

    if (*FtpBuffer != '2')
		return false;

    return ret;
}

bool TFtpCli :: ftp_list(const char * FileName)
{
    bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);
	if(FileName == NULL)
		sprintf(FtpBuffer, "LIST -aL \r\n", FileName);
	else
    	sprintf(FtpBuffer, "LIST -aL %s\r\n", FileName);
		
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;

    ret = getReply(FtpBuffer);

    if (*FtpBuffer == '5')
		return false;

    if (*FtpBuffer != '1')
		return false;

    return ret;
}

bool TFtpCli :: ftp_retr(const char * FileName)
{
    bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);
	sprintf(FtpBuffer, "RETR %s\r\n", FileName);
	
	if(Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;
    
	ret = getReply(FtpBuffer);
	
    if (*FtpBuffer == '5')
		return false;

    if (*FtpBuffer != '1')
		return false;

	return ret;
}

/* PASV command */
bool TFtpCli :: ftp_pasv(unsigned char * addr)
{
    bool ret;
    unsigned char *p;
    int i;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);	
	sprintf(FtpBuffer, "PASV\r\n");
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;
	
    ret = getReply(FtpBuffer);

    if (*FtpBuffer != '2') 
		return false;
	
    p = (unsigned char *)FtpBuffer;
    
	for (p += 4; *p && !isdigit(*p); p++);
	
    if (!*p) 
		return false;

    for (i = 0; i < 6; i++)
    {
		addr[i] = 0;

		for (; isdigit(*p); p++)
	    	addr[i] = (*p - '0') + 10 * addr[i];
		
		if (*p == ',')
	    	p++;
		else if (i < 5)
	    	return false;
    }

    return true;
}

bool TFtpCli :: ftp_rest(off_t Bytes)
{
    bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);	
	sprintf(FtpBuffer, "REST %lld\r\n", Bytes);
	if( Send( FtpBuffer, strlen(FtpBuffer), 0, 100 ) == -1)
		return false;
	
	ret = getReply(FtpBuffer);
	
    if (*FtpBuffer != '3')
		return false;
    
	return ret;
}

/* CWD command */
bool TFtpCli :: ftp_cwd(const char * Dir)
{
    bool ret;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);	
	sprintf(FtpBuffer, "CWD %s\r\n", Dir);
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100 ) == -1)
		return false;
    
    ret = getReply(FtpBuffer);

	/* check and see wether the CWD succeeded  */
	if (*FtpBuffer == '5')
		return false;

    if (*FtpBuffer != '2')
		return false;

    return ret;
}

/* Returns the Current working directory in dir */
bool TFtpCli :: ftp_pwd(char * Dir)
{
    bool ret;
	char *r, *l;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);	
	sprintf(FtpBuffer, "PWD\r\n");
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;
    
    ret = getReply(FtpBuffer);

    /* check and see wether the PWD succeeded */
    if ( *FtpBuffer == '5' )
		return false;

   	if ( *FtpBuffer != '2' )
		return false;

    if ((r = strrchr(FtpBuffer, '"')) != NULL)
    {
		l = strchr(FtpBuffer, '"');
		if ((l != NULL) && (l != r))
		{
	    	*r = '\0';
	    	++l;
	    	strcpy(Dir, l);
	    	*r = '"';
		}
    } 
	else
    {
		if ((r = strchr(FtpBuffer, ' ')) != NULL)
		{
	    	*r = '\0';
	    	strcpy(Dir, FtpBuffer);
	    	*r = ' ';
		}
    }

    return ret;
}

bool TFtpCli :: ftp_port(const char * Cmd)
{
    bool ret;

	memset(FtpBuffer, 0, FTP_BUF_SIZE); 
	strcpy(FtpBuffer, Cmd);
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;

	ret = getReply(FtpBuffer);
	
	/***
    if (err != FTPOK)
		return err;
	***/

    if (*FtpBuffer != '2')
		return false;

    return ret;

}

bool TFtpCli :: ftp_login(void)
{
    bool ret;
	const char * UserName, *PassWord;

	memset(FtpBuffer, 0, FTP_BUF_SIZE);

	if(UseProxy)
	{
		if(!m_proxy.user.empty())
		{
			UserName = strdup(m_proxy.user.c_str());
			PassWord = strdup(m_proxy.pass.c_str());
		}
		else
		{
			UserName = "anonymous";
			PassWord = uri->password().c_str();
			ShowMsg("Logging in as user %s password %s", UserName, PassWord);
		}
		sprintf(FtpBuffer, "USER %s@%s:%d\r\n", UserName, uri->hostname().c_str() , uri->port() );
	}
	else
	{

		if(uri->user().empty())
			UserName = DEFAULT_USER;
		else
			UserName = uri->user().c_str();
				
	   	if(uri->password().empty() )
			PassWord = DEFAULT_PASSWD;
		else
			PassWord = uri->password().c_str();	

		ShowMsg("Logging in as user %s password %s", UserName, PassWord);
		sprintf(FtpBuffer, "USER %s\r\n", UserName);
	}
	
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;

    ret = getReply(FtpBuffer);

    if (*FtpBuffer == '2')
		return true;
    
	if (*FtpBuffer != '3')
		return false;
	
	memset(FtpBuffer, 0, FTP_BUF_SIZE);			
	sprintf(FtpBuffer, "PASS %s\r\n", PassWord);
	if( Send(FtpBuffer, strlen(FtpBuffer), 0, 100) == -1)
		return false;
    
    ret = getReply(FtpBuffer);

    if (*FtpBuffer != '2')
		return false;

    return ret;
}

int TFtpCli :: readLineFromDataConn(char *line, int maxsize)
{
	int i, retval;
	char c;
	char *ptr;
    
	ptr = line;
	
	for(i = 1; i < maxsize; i++)
    {
		retval = recvFromDataConn(&c, 1, 0, 100);
	
		if (retval < 0)
			return retval;	//HERR

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

bool TFtpCli :: getDirList(const char* sUrl, char * DirName)
{	
	bool ret;
//	int ret;
	char LineBuf[LINE_BUFFER];

	ret = setupCtrlConn(sUrl);
    if (ret != true)
    {
		ShowMsg("Error connecting to %s", uri->hostname().c_str() );
		return ret;
    }
    ShowMsg("Connect Ok");

    ret = ftp_login();
    if (ret != true)
    {
		ShowMsg("Login Failed");
		closeCtrlConn();
		return ret;
    } 
	ShowMsg("Login OK");

////////////////////////////////////////////
	if(IsPasvMode())
	{
		if(IsPasvMode())
		{
			ret = setupPasvDataConn();
			if(ret != true)
			{
				ShowMsg("Error while connecting, according to servers PASV info");
				closeCtrlConn();
				return ret;
			}
		}
	}
	else
	{
		ret = setupPortDataConn();
		if(ret != true)
		{
  			ShowMsg("Error while server connecting, according to Client Port info");
			closeCtrlConn();
			return ret;
		}
	}
	    
	ret = ftp_ascii();
    if (ret != true)
	{
		closeCtrlConn();
 		return ret;
    }
	
	ret = ftp_list(DirName); 
    if (ret != true)
	{
		closeCtrlConn();
    	return ret;
	}
//////////////////////////////////////////////////
	while(1)
	{
		memset(LineBuf, 0, LINE_BUFFER);
		int len = readLineFromDataConn(LineBuf, LINE_BUFFER);
		if(len == 0) break;
		if(len < 0)
			return false;

		ShowMsg(LineBuf);	
	}
	return true;
}

////////////////////////////////////////////////////////////////
void TFtpCli :: setDataConnIPTOS()
{
	DataSkp->setIPTos();
}

off_t TFtpCli :: recvFromDataConn(char *buffer, int size, int flags, int timeout)
{
	int	BytesRead = DataSkp->Recv(buffer, size, flags, timeout);
	return BytesRead;
}

off_t TFtpCli :: sendToDataConn(char *buffer, int size, int flags, int timeout)
{
	int BytesWritten = DataSkp->Send(buffer, size, flags, timeout);
	return BytesWritten;
}

off_t TFtpCli :: recvFromCtrlConn(char *buffer, int size, int flags, int timeout)
{
	int	BytesRead = Recv(buffer, size, flags, timeout);
	return BytesRead;
}

off_t TFtpCli :: sendToCtrlConn(char *buffer, int size, int flags, int timeout)
{
	int BytesWritten = Send(buffer, size, flags, timeout);
	return BytesWritten;
}

bool TFtpCli :: fetchRemoteFile( const char * sUrl )
{
	int ret;
	char Log[MAX_MSG_SIZE];
	unsigned char PasvAddr[6];	
    
	bool status = setupCtrlConn(sUrl);
	if ( status == false )
	{
		closeCtrlConn();
    	return status;
	} 

    ShowMsg("Connect Ok");

	status = ftp_login();
	if (status == false)
	{
    	closeCtrlConn();
    	return status;
	}
	
	ShowMsg("Login Ok");
	status = ftp_binary();

	if (status == false)
	{
		ShowMsg("Binary Failed");
		closeCtrlConn();
		return status;
	}
	ShowMsg("Binary OK");

	int pos = uri->path().find_last_of('/');
	string	dir, file;
	if(pos != string::npos)
	{	
		dir = uri->path().substr(1, pos);
		file = uri->path().substr(pos + 1);

		cout << dir << '\n' << file << '\n' << uri->path() << endl;

		if(!dir.empty())
		{
			status = ftp_cwd( dir.c_str() );
			if (status != true)
			{
	   			ShowMsg("CWD failed to change to directory %s", dir.c_str() );
				closeCtrlConn();
	    		return status;
			}
		}
		else
		{
			ShowMsg("CWD is not needed");
		}
	}

	ShowMsg("CWD OK");


	if( IsPasvMode() )
	{
		status = setupPasvDataConn();
		if(status != true )
		{
			ShowMsg("Error while connecting, according to servers PASV info");
			closeCtrlConn();
			return false;
		}
	}
	else
	{
		status = setupPortDataConn();
		if(status != true)
		{
  			ShowMsg("Error while server connecting, according to Client Port info");
			closeCtrlConn();
			return false;
		}
	}
	
	status = ftp_rest(0);
   	if ( status != true)
   	{
		ShowMsg("REST failed");
		closeCtrlConn();
		return false;
	}
	ShowMsg("REST OK");

    status = ftp_retr(file.c_str());
    if (status != true)
    {
		ShowMsg("RETR failed");
		closeCtrlConn();
		return false;
    }
    ShowMsg("RETR OK");

    closeCtrlConn();
	closeDataConn();
	
	return true;

}

}

