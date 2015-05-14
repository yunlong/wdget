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

#ifndef __FTPCLI_H__
#define __FTPCLI_H__

namespace matrix {

class TFtpCli : public TcpSocket
{
private :
	Uri	* uri;	
	bool IsPasv;  //true : Passive mode ;  false: Port mode 
	///////////////////////////////////////
 	TSocket * DataSkp;    //for Passive Mode and Data Channel
	TSocket * ListenSkp;  //for Port Mode

    char * FtpBuffer;

	bool UseProxy;
	Proxy_t m_proxy;

public:
	TFtpCli(void); 
	~TFtpCli(void); 


	TFtpCli(Proxy_t proxy);

	void initUri(const string& sUrl) 
	{   
		if(uri != NULL)
		{
			uri->init(sUrl);
			if(uri->port() == 0 ) uri->port(80);
		}
	}   

public:
	
	bool IsPasvMode() { return IsPasv; }
	void setPasvMode( bool val ) { IsPasv = val; }

	char * getFtpBuf(void) { return FtpBuffer; }
	
	bool setupCtrlConn(const string& sUrl);
	bool setupPasvDataConn(void);
	bool setupPortDataConn(void);

	bool connectRemoteHost(const char * ipaddr, int dport);
	bool connectProxyHost( TSocket * skp, Proxy_t& proxy, int dport);

	void closeCtrlConn(void);
	void closeDataConn(void);
	void closeListenConn(void);  // for Port Mode
	
/////////////////////////////////////////////////////////////////
	void setDataConnIPTOS(void);
	off_t recvFromDataConn(char *buffer, int size, int flags, int timeout);
	off_t sendToDataConn(char *buffer, int size, int flags, int timeout);
	off_t recvFromCtrlConn(char *buffer, int size, int flags, int timeout);
	off_t sendToCtrlConn(char *buffer, int size, int flags, int timeout);

///////////////////////////////////////////////////////////////////		
	
	bool ftp_login(void);
	bool ftp_pwd(char * dir);
	bool ftp_cwd(const char * dir);
	bool ftp_rest(off_t Bytes);
	bool ftp_list(const char * FileName = NULL);
	bool ftp_binary(void);
	bool ftp_ascii(void);
	bool ftp_type(void);
	bool ftp_size(const char * FileName, off_t * FileSize);
	bool ftp_port(const char * Cmd);
	bool ftp_retr(const char * FileName);
	bool ftp_pasv(unsigned char * addr);
//////////////////////////////////////////////////////////////////////////	
	int getReplyCode(const char * szBuffer);
	bool getLine(char * szBuffer);
	bool getReply(char * szBuffer);
	
///////////////////// the following ////////////////////////////////////
	int readLineFromDataConn(char *line, int maxsize);
	bool getDirList(const char* sUrl, char * DirName);
	////////////////////////////////////////////////////////
	bool fetchRemoteFile(const char* sUrl);

};

}

#endif
