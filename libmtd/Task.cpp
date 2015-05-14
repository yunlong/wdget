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


#define USE_SOCKET_H
#define USE_THREAD_H
#define USE_HTTPMSG_H
#define USE_HTTPCLI_H
#define USE_FTPCLI_H
#define USE_MMSCLI_H
#define USE_RTSPCLI_H

#define USE_TASKMGR_H
#define USE_TASK_H
#define USE_URI_H
#define USE_UTILS_H
#define USE_CONFIG_H
#define USE_FTPPARSER_H

#define USE_INIFILE_H


#include "matrix.h"

namespace matrix {

const string ThreadStatusList[] = {
		"IDLE",
   		"CONNECTING",
   		"LOGININ",
   		"DOWNLOADING",
   		"COMPLETED",
   		"LOGINDENIED",
   		"CONNREFUSED",
   		"REMOTEFATAL",
   		"LOCALFATAL",
   		"TIMEOUT",
   		"MAXATTEMPTS"
};

/************** class TaskThread Object Impl ***************/
pthread_key_t tg_keyself;
void* task_thread_start( void* pvoid )
{
    TaskThread* pth;
    pthread_setspecific( tg_keyself, pvoid );

    pth = (TaskThread*)pvoid;
    if( pth->init() )
    {
        pth->run();
    }
    pth->m_retval = pth->exit();
    return NULL;
}

TaskThread :: TaskThread(void)
{
	m_thread = -1;
	ThreadId = -1;
	
	Status = THS_UNKNOWN;	
	tsk = NULL;
	
	BlkSize = -1;
	RemoteStartPos = -1;
	RemoteEndPos = -1;
	RemoteBytesReceived = 0;	
	LocalStartPos = 0;
	
	Retry = true;
	TryAttempts = 0;

	memset(&BeginTime, 0, sizeof(struct timeval));
	memset(&EndTime, 0, sizeof(struct timeval));
	memset(&RetryStart, 0, sizeof(struct timeval));
}

TaskThread :: ~TaskThread( void )
{

}

int TaskThread :: create( void )
{
	int retval;
    retval = pthread_create( &m_thread, NULL, task_thread_start, this );
//	pthread_detach(m_thread);
	return retval;
}

bool TaskThread :: init( void )
{
    return false;
}

int TaskThread::exit( void )
{
	//pthread_exit(&m_retval);
    return 0;
}

void TaskThread :: run( void )
{
	//Empty
}

void TaskThread :: suspend( void )
{
	pthread_cancel(m_thread);	
}

void TaskThread :: joinSelf(void)
{
	pthread_join(m_thread, NULL);
}

void TaskThread :: resume( void )
{
	//Empty
}

TaskThread* TaskThread::This( void )
{
    return (TaskThread*)pthread_getspecific( tg_keyself );
}

TTask * TaskThread :: getOwner(void)
{
	return tsk;
}
	
void TaskThread :: setOwner(TTask * val)
{
	tsk = val;
}


//////////////	Http Thread Class Impl	/////////////
THttpThread :: THttpThread( void )
{
	pthread_key_create( &tg_keyself, NULL );
	pthread_setspecific( tg_keyself, this );
	hcp = NULL;
}

bool THttpThread :: retrKnownSizeFile(void)
{
    off_t Total = 0, BytesRead = 0, Length;	
	char Log[MAX_MSG_SIZE];
    char szBuffer[HTTP_BUF_SIZE];	
	FILE * fp = NULL;
	
    Length = RemoteEndPos - RemoteStartPos;
	
	if(Length == 0) Length = 1;

    if (! (fp = fopen( LocalFile.c_str(), FileMode.c_str() )) )
    {
		ShowMsg("Error opening file %s for writing: %s", LocalFile.c_str(), strerror(errno));
		sprintf(Log, "Error opening file %s for writing: %s", LocalFile.c_str(), strerror(errno));
		tsk->saveThreadLog(ThreadId, Log);
		Status = THS_LOCALFATAL;
		return false;
    }
    
	fseeko(fp, LocalStartPos, SEEK_SET);

	/* Make sure all writes go directly to the file */
    setvbuf(fp, NULL, _IONBF, 0);
	
	if ((BeginTime.tv_sec == 0) && (BeginTime.tv_usec == 0))
		gettimeofday(&BeginTime, NULL);
   	
	sprintf(Log, "Thread%d Receive Data from Remote host", ThreadId);
	tsk->saveThreadLog(ThreadId, Log);
	
    Status = THS_DOWNLOADING;
    
	hcp->setIPTos();

    while(Length > 0)
    {
		if(Length < HTTP_BUF_SIZE && Length > 0)
		{
	    	while(Length > 0)
	    	{
				BytesRead = hcp->RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100); 
				
				if((BytesRead == 0 && Length > 0))
				{	
		    		ShowMsg("Server closed the conenction prematurely!");
					sprintf(Log, "Server closed the conenction prematurely!");
					tsk->saveThreadLog(ThreadId, Log);
		    		fclose(fp);
		    		Status = THS_REMOTEFATAL;
		    		return false;
				}

				if(BytesRead == -1)
				{
		    		ShowMsg("Error receving data");
					sprintf(Log, "Error receving data!!!");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);
		    		if (errno == ETIMEDOUT)
		    		{
						ShowMsg("connection timed out");
						sprintf(Log, "connection timed out");
						tsk->saveThreadLog(ThreadId, Log);

						Status = THS_TIMEDOUT;
						return false;
		    		}
		    		Status = THS_REMOTEFATAL;
		    		return false;
				}
				
				flockfile(fp);
				if(BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
				{
		    		ShowMsg("write failed");
					sprintf(Log, "write data to file failed !!!");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);
		    		Status = THS_LOCALFATAL;
					
		    		return false;
				}
				funlockfile(fp);
						
				Length -= BytesRead;
				Total += BytesRead;
												
		 		RemoteStartPos += BytesRead;
 		 		LocalStartPos += BytesRead;				
				RemoteBytesReceived += BytesRead;
	    	}	
			
	    	break;
		}
		
		BytesRead = hcp->RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);	
		
		if ((BytesRead == 0 && Length > 0))
		{
			ShowMsg("Server closed the conenction prematurely!");
			sprintf(Log, "Server closed the conenction prematurely!");
			tsk->saveThreadLog(ThreadId, Log);

			fclose(fp);
			Status = THS_REMOTEFATAL;
			return false;
		}

		if(BytesRead == -1)
		{
	    	ShowMsg("Error receving data");
			sprintf(Log, "Error receving data");
			tsk->saveThreadLog(ThreadId, Log);

	    	fclose(fp);
	    	if (errno == ETIMEDOUT)
	    	{
				ShowMsg("connection timed out");
				sprintf(Log, "connection timed out");
				tsk->saveThreadLog(ThreadId, Log);

				Status = THS_TIMEDOUT;
				return false;
	    	}
	    	Status = THS_REMOTEFATAL;
	    	return false;
		}
		
		flockfile(fp);
		if (BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
		{
	    	ShowMsg("write failed");
			sprintf(Log, "write data to file failed !!!");
			tsk->saveThreadLog(ThreadId, Log);

	    	fclose(fp);
	    	Status = THS_LOCALFATAL;
	    	return false;
		}
		funlockfile(fp);
		
		
		Length -= BytesRead;
		Total += BytesRead;
		
		RemoteStartPos += BytesRead;
 		LocalStartPos += BytesRead;				
		RemoteBytesReceived += BytesRead;
		
    }
	
   	fclose(fp);
    Status = THS_ALLDONE;
	
    ShowMsg("%s : %lldK bytes received", LocalFile.c_str(), Total / 1024);
	sprintf(Log, "%s : %lldK bytes received", LocalFile.c_str(), Total / 1024);
	tsk->saveThreadLog(ThreadId, Log);
    return true;
}

bool THttpThread :: retrUnknownSizeFile(void)
{
	char Log[MAX_MSG_SIZE];

    off_t Total = 0, BytesRead = 0, Length;
	char szBuffer[HTTP_BUF_SIZE];
	FILE * fp = NULL;
		
    if( !(fp = fopen(LocalFile.c_str(), FileMode.c_str()) ) )
    {
		ShowMsg("Error opening file %s for writing: %s",
										LocalFile.c_str(), strerror(errno));
		sprintf(Log, "Error opening file %s for writing: %s",
										LocalFile.c_str(), strerror(errno));
		tsk->saveThreadLog(ThreadId, Log);

		Status = THS_LOCALFATAL;
		return false;
    }
    	
	fseeko(fp, LocalStartPos, SEEK_SET);	
	/*Make sure all writes go directly to the file */
    setvbuf(fp, NULL, _IONBF, 0);
    	
	if ((BeginTime.tv_sec == 0) && (BeginTime.tv_usec == 0))
		gettimeofday(&BeginTime, NULL);

   	sprintf(Log, "Thread%d Receive Data from Remote host", ThreadId);
	tsk->saveThreadLog(ThreadId, Log);

	Status = THS_DOWNLOADING;
	
    hcp->setIPTos();
  	do
    {
		BytesRead = hcp->RecvData(szBuffer, HTTP_BUF_SIZE, 0, 100);
		if(BytesRead > 0)
		{
			flockfile(fp);			
	    	if(fwrite(szBuffer, sizeof(char), BytesRead, fp) < BytesRead)
	    	{
				ShowMsg("Error writing to  file %s : %s",
									LocalFile.c_str(), strerror(errno));
				sprintf(Log, "Error writing to  file %s : %s",
										LocalFile.c_str(), strerror(errno));
				tsk->saveThreadLog(ThreadId, Log);

				fclose(fp);
				Status = THS_LOCALFATAL;
				return false;
	    	}
	    	funlockfile(fp);	
		}

		RemoteStartPos += BytesRead;
 		LocalStartPos += BytesRead;				
		RemoteBytesReceived += BytesRead;

    }while(BytesRead > 0);
    
	if(BytesRead == -1)
    {
		if(errno == ETIMEDOUT)
		{
		   	ShowMsg("TThread timed out");
			sprintf(Log, "TThread timed out");
			tsk->saveThreadLog(ThreadId, Log);

	    	Status = THS_TIMEDOUT;
	    	return false;
		}
		Status = THS_REMOTEFATAL;
		return false;
    }
   
    fclose(fp);
	Status = THS_ALLDONE;
    ShowMsg("%s : %lldK bytes received", LocalFile.c_str(), Total / 1024);
	
	sprintf(Log, "%s : %lldK bytes received", LocalFile.c_str(), Total / 1024);
	tsk->saveThreadLog(ThreadId, Log);

	return true;
	
}
		 
bool THttpThread :: retrFileChunk(void)
{
    bool ret;
	char Log[MAX_MSG_SIZE];
    FILE *fp = NULL;
		    
    if (Status == THS_ALLDONE)
    {
		gettimeofday(&BeginTime, NULL);
		return true;
    }
    
    Status = THS_CONNECTING;
	
	if(tsk->UseProxy)
	{
		hcp->initUri(tsk->uri->unparse());
		ret = hcp->connectProxyHost( tsk->m_proxy );
    	if(ret != true)
    	{
			ShowMsg("Error connecting to %s", tsk->m_proxy.host.c_str() );
			sprintf(Log, "Error connecting to %s", tsk->m_proxy.host.c_str() );
			tsk->saveThreadLog(ThreadId, Log);
			Status = THS_REMOTEFATAL;
			return false;
    	}
	}
	else
	{

		if( tsk->uri->port() == 0 ) tsk->uri->port(80);
		ret = hcp->connectRemoteHost( tsk->uri->unparse() );
    	if(ret != true)
    	{
			ShowMsg("Error connecting to %s", tsk->uri->hostname().c_str() );
			sprintf(Log, "Error connecting to %s", tsk->uri->hostname().c_str() );
			tsk->saveThreadLog(ThreadId, Log);
			Status = THS_REMOTEFATAL;
			return false;
    	}

	}

	hcp->buildHostHdr( tsk->uri->hostname().c_str(), tsk->uri->port() );
	hcp->buildRefererHdr( tsk->uri->unparse().c_str() );
   	hcp->buildAcceptHdr();
    hcp->buildUserAgentHdr();
	hcp->buildConnectionHdr();
	if( !tsk->uri->user().empty() && !tsk->uri->password().empty() )
	{
		hcp->buildBasicAuthHdr( tsk->uri->user().c_str(), tsk->uri->password().c_str() );
    }

	if(tsk->FileSize != -1)
		hcp->buildRangeHdr(RemoteStartPos, RemoteEndPos);
	else
		hcp->buildRangeHdr(RemoteStartPos, -1);

	if(tsk->UseProxy)
	{
		if(!tsk->m_proxy.user.empty())
		{
			hcp->buildProxyAuthHdr(tsk->m_proxy.user.c_str(), tsk->m_proxy.pass.c_str());
		}
		hcp->buildPragmaHdr();
	}

	if(tsk->UseProxy)
   		hcp->sendHttpRequest(VERB_GET, true);
	else
   		hcp->sendHttpRequest(VERB_GET, false);

////////////////////////////////////////////////////////	
	ShowMsg("%s", hcp->getStatusLine().c_str() );

	int nHdr;
	THttpHdr * pHdr;
	tsk->saveThreadLog(ThreadId, hcp->getStatusLine() );
	nHdr = hcp->getHttpRequest()->GetHdrCount();
	for(int i = 0; i < nHdr; i++)
	{
		pHdr = hcp->getHttpRequest()->GetHdr( i );
		ShowMsg("%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		sprintf(Log, "%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		tsk->saveThreadLog(ThreadId, Log);
   	}
	
///////////////////////////////////////////////////////////
	int statuscode  = hcp->recvHttpResponse();
	ShowMsg("\n%s", hcp->getStatusLine().c_str() );
///////////////////////////////////////////////////////////	

	tsk->saveThreadLog(ThreadId, hcp->getStatusLine());
	nHdr = hcp->getHttpResponse()->GetHdrCount();
	for(int i = 0; i < nHdr; i++)
	{
		pHdr = hcp->getHttpResponse()->GetHdr( i );
		ShowMsg("%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		sprintf(Log, "%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		tsk->saveThreadLog(ThreadId, Log);
   	}
////////////////////////////////////////////////////////////////
		
	if(tsk->FileSize == -1)
		ret = retrUnknownSizeFile();
	else
		ret = retrKnownSizeFile();
	
	hcp->closeSocket();
   	return ret;
}

bool THttpThread :: init( void )
{
	return true;
}

int THttpThread :: exit( void )
{
	return 0;
}

void THttpThread :: run( void ) 
{
    bool ret = false;
	char buf[MAX_MSG_SIZE];
	bool FirstTry = true;
	
	hcp = new THttpCli();
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
    do
    {		
		if (FirstTry != true)
		{
	    	sleep(RETRY_INTERVAL);	
	    	if (TryAttempts == 0)
			{	
				ShowMsg("Trying infinitely...attempt %d", TryAttempts);
				sprintf(buf, "Trying infinitely...attempt %d", TryAttempts);
				tsk->saveThreadLog(ThreadId, buf);
			}
			else
			{
				ShowMsg("Trying....attempt %d of %d", TryAttempts,
														 MAX_TRY_ATTEMPTS);
				sprintf(buf, "Trying....attempt %d of %d", TryAttempts,
														 MAX_TRY_ATTEMPTS);
				tsk->saveThreadLog(ThreadId, buf);
			}
	    	
			delay_ms(400);
	    	Status = THS_IDLE;
		}
		
		if(BlkSize == RemoteBytesReceived)
		{	
	   		Status = THS_ALLDONE;
			ShowMsg("%s have finished its task", ThreadName.c_str() );
			sprintf(buf, "%s have finished its task", ThreadName.c_str() );
			tsk->saveThreadLog(ThreadId, buf);

			if(hcp != NULL)
			{
				delete hcp; hcp = NULL;
			}

			pthread_exit(0);
	    	return;
		}
		
		if (FirstTry == true)
	    	FirstTry = false;
			
		ret = retrFileChunk();
		
		TryAttempts++;

		if (ret == true)
		{
			ShowMsg("%s have finished its task", ThreadName.c_str() );
			sprintf(buf, "%s have finished its task", ThreadName.c_str() );
			tsk->saveThreadLog(ThreadId, buf);

			if(hcp != NULL)
			{
				delete hcp; hcp = NULL;
			}

			pthread_exit(0);
	    	return;
		}

    }while (TryAttempts < MAX_TRY_ATTEMPTS);

	if(hcp != NULL)
	{
		delete hcp; hcp = NULL;
	}

	Status = THS_MAXTRYS;
	pthread_exit(0);
}

//////////////	Ftp Thread Class Impl	/////////////
TFtpThread :: TFtpThread( void )
{
	pthread_key_create( &tg_keyself, NULL );
	pthread_setspecific( tg_keyself, this );
	fcp = NULL;
}

bool TFtpThread :: retrKnownSizeFile(void)
{
	char Log[MAX_MSG_SIZE];
	char szBuffer[FTP_BUF_SIZE];
	off_t Total = 0, BytesRead = 0, Length;
 	FILE * fp = NULL;
	
	Length = RemoteEndPos - RemoteStartPos; 
		
    if (!(fp = fopen(LocalFile.c_str(), FileMode.c_str()) ))
    {
		Status = THS_LOCALFATAL;
		return false;
    }
    	
	fseeko(fp, LocalStartPos, SEEK_SET);
	setvbuf(fp, NULL, _IONBF, 0);
	
	Status = THS_DOWNLOADING;	
	if ((BeginTime.tv_sec == 0) && (BeginTime.tv_usec == 0))
		gettimeofday(&BeginTime, NULL);
		
	sprintf(Log, "Thread%d Receive Data from Remote host", ThreadId);
	tsk->saveThreadLog(ThreadId, Log);
	
	fcp->setDataConnIPTOS();

	while(Length > 0)
    {
		if(Length < FTP_BUF_SIZE && Length > 0)
		{
	    	while(Length > 0)
	    	{
				BytesRead = fcp->recvFromDataConn(szBuffer, Length, 0, 100);
				
				if ((BytesRead == 0 && Length > 0))
				{
		    		ShowMsg("Server closed the connection prematurely!");
					sprintf(Log, "Server closed the connection prematurely!");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);
		    		Status = THS_REMOTEFATAL;
		    		return false;
				}
				
				if (BytesRead == -1)
				{
		    		ShowMsg("error receving data");
					sprintf(Log, "error receving data");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);
				
		    		if(errno == ETIMEDOUT)
		    		{
						ShowMsg("TThread timed out");
						sprintf(Log, "TThread timed out");
						tsk->saveThreadLog(ThreadId, Log);
						Status = THS_TIMEDOUT;
						return false;
		    		}				
		    		Status = THS_REMOTEFATAL;
		    		return false;
				}

				flockfile(fp);
				if (BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))	
				{
		    		ShowMsg("write failed");
					sprintf(Log, "write data to file failed !!!");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);		    			
		    		Status = THS_LOCALFATAL;
		    		return false;
				}
				funlockfile(fp);
					
				Length -= BytesRead;
				Total += BytesRead;
				
		 		RemoteStartPos += BytesRead;
 		 		LocalStartPos += BytesRead;				
				RemoteBytesReceived += BytesRead;
				
			}	
	    	break;
		}

		BytesRead = fcp->recvFromDataConn(szBuffer, FTP_BUF_SIZE, 0, 100);
		if ((BytesRead == 0 && Length > 0))
		{
		    ShowMsg("Server closed the connection prematurely!");
			sprintf(Log, "Server closed the connection prematurely!");
			tsk->saveThreadLog(ThreadId, Log);

		    fclose(fp);
		    Status = THS_REMOTEFATAL;
		    return false;
		}

		if(BytesRead == -1)
		{
	    	ShowMsg("error receving data");
			sprintf(Log, "error receving data");
			tsk->saveThreadLog(ThreadId, Log);

	    	fclose(fp);
	    	if (errno == ETIMEDOUT)
	    	{
				ShowMsg("TThread timed out");
				sprintf(Log, "TThread timed out");
				tsk->saveThreadLog(ThreadId, Log);

				Status = THS_TIMEDOUT;
				return false;
	    	}
	    	Status = THS_REMOTEFATAL;
	    	return false;
		}
		
		flockfile(fp);
		if(BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
		{
	    	ShowMsg("write failed");
			sprintf(Log, "write data to file failed");
			tsk->saveThreadLog(ThreadId, Log);

	    	fclose(fp);
	    	Status = THS_LOCALFATAL;
	    	return false;
		}
		funlockfile(fp);
		
		Length -= BytesRead;
		Total += BytesRead;
		
		RemoteStartPos += BytesRead;
 		LocalStartPos += BytesRead;				
		RemoteBytesReceived += BytesRead;
				
    }

    fclose(fp);

	Status = THS_ALLDONE;

   	ShowMsg("%s : %lldK bytes received", LocalFile.c_str(), Total / 1024);
	sprintf(Log, "%s : %lldK bytes received", LocalFile.c_str(), Total / 1024);
	tsk->saveThreadLog(ThreadId, Log);

    return true;
}

bool TFtpThread ::retrUnknownSizeFile(void)
{
	char Log[MAX_MSG_SIZE];
	char szBuffer[FTP_BUF_SIZE];
	
	off_t BytesRead;
	FILE * fp = NULL;

    if (!(fp = fopen(LocalFile.c_str(), FileMode.c_str()) ))
    {
		Status = THS_LOCALFATAL;
		return false;
    }

	fseeko(fp, LocalStartPos, SEEK_SET);	
    setvbuf(fp, NULL, _IONBF, 0);
    
	Status = THS_DOWNLOADING;
	if ((BeginTime.tv_sec == 0) && (BeginTime.tv_usec == 0))
		gettimeofday(&BeginTime, NULL);
	
	sprintf(Log, "Thread%d Receive Data from Remote host", ThreadId);
	tsk->saveThreadLog(ThreadId, Log);
	
	fcp->setDataConnIPTOS();
	do
    {
		BytesRead = fcp->recvFromDataConn(szBuffer, FTP_BUF_SIZE, 0, 100);
		if (BytesRead > 0)
		{
			flockfile(fp);
		    if (fwrite(szBuffer, sizeof(char), BytesRead, fp) < BytesRead)
	    	{
				fclose(fp);
				sprintf(Log, "write data to file failed");
				tsk->saveThreadLog(ThreadId, Log);
				ShowMsg("write failed");
				Status = THS_LOCALFATAL;
				return false;
	    	}
			funlockfile(fp);
			
	    	RemoteStartPos += BytesRead;
 			LocalStartPos += BytesRead;				
	   		RemoteBytesReceived += BytesRead;
		}
    }while (BytesRead > 0);

    fclose(fp);

	if (BytesRead == -1)
    {
		if (errno == ETIMEDOUT)
		{
	    	ShowMsg("TThread timed out");
			sprintf(Log, "TThread timed out");
			tsk->saveThreadLog(ThreadId, Log);
	    	Status = THS_TIMEDOUT;
	    	return false;
		}
		Status = THS_REMOTEFATAL;
		return false;
    } 
	
	Status = THS_ALLDONE;
	
    return true;
}

bool TFtpThread :: retrFileChunk(void)
{
	bool ret;
	char Log[MAX_MSG_SIZE];
	unsigned char PasvAddr[6];	
    
	if (Status == THS_ALLDONE)
    {
		gettimeofday(&BeginTime, NULL);
		return true;
    }

//	tsk->lockThread(tsk);
    Status = THS_CONNECTING;
//  tsk->broadcastCond(tsk);
//  tsk->unlockThread(tsk);

	ret = fcp->setupCtrlConn( tsk->uri->unparse() );
	
	sprintf(Log, "connecting to %s", tsk->uri->hostname().c_str());
	tsk->saveThreadLog(ThreadId, Log);
	tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
	
    if (ret != true)
    {
	   	ShowMsg("Error connecting to %s", tsk->uri->hostname().c_str());
		sprintf(Log, "Error connecting to %s", tsk->uri->hostname().c_str());
		tsk->saveThreadLog(ThreadId, Log);
	   	Status = THS_REMOTEFATAL;
		fcp->closeCtrlConn();
	   	return false;
    }

    ShowMsg("Connect Ok");
	sprintf(Log, "Connect Ok");
	tsk->saveThreadLog(ThreadId, Log);

    Status = THS_LOGGININ;
	ret = fcp->ftp_login();
    if (ret != true)
    {
	   	ShowMsg("Login Failed");
		sprintf(Log, "Login Failed");
		tsk->saveThreadLog(ThreadId, Log);
    	fcp->closeCtrlConn();
    	Status = THS_LOGINFAIL;
    	//Status = THS_REMOTEFATAL;
    	return false;
  	} 

	ShowMsg("Login Ok");
	sprintf(Log, "Login Ok");
	tsk->saveThreadLog(ThreadId, Log);

	ret = fcp->ftp_binary();
	if (ret != true)
	{
		ShowMsg("Binary Failed");
		sprintf(Log, "Binary Failed");
		tsk->saveThreadLog(ThreadId, Log);
		fcp->closeCtrlConn();
		Status = THS_REMOTEFATAL;
		return false;
	}
	tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
	ShowMsg("Binary OK");
	sprintf(Log, "Binary OK");
	tsk->saveThreadLog(ThreadId, Log);
	
	int pos = tsk->uri->path().find_last_of('/');
	string  dir, file;
	if(pos != string::npos)
	{    
		dir = tsk->uri->path().substr(1, pos);
		file = tsk->uri->path().substr(pos + 1);

		if ( file.empty() )
		{
			ShowMsg("No file specified.");
			return false ;
		}

		if(!dir.empty())
		{
			ret = fcp->ftp_cwd( dir.c_str() );
			if (ret != true)
			{
				ShowMsg("CWD failed to change to directory %s", dir.c_str() );
				sprintf(Log, "CWD failed to change to directory %s", dir.c_str());
				tsk->saveThreadLog(ThreadId, Log);
				fcp->closeCtrlConn();
	    		Status = THS_REMOTEFATAL;
		    	return false;
			}
		}
		else
		{
			tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
			ShowMsg("CWD is not needed");
			sprintf(Log, "CWD is not needed");
			tsk->saveThreadLog(ThreadId, Log);
		}
	}    

	tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
	ShowMsg("CWD OK");
	sprintf(Log, "CWD OK");
	tsk->saveThreadLog(ThreadId, Log);
	if( fcp->IsPasvMode() )
	{    
		ret = fcp->setupPasvDataConn();
		if( ret != true )
		{
			ShowMsg("Server doesn't seem to support PASV");
			sprintf(Log, "Server doesn't seem to support PASV");
			tsk->saveThreadLog(ThreadId, Log);
			fcp->setPasvMode(false);
			fcp->closeCtrlConn();
			Status = THS_REMOTEFATAL;
			return false;
		}
		tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
	}    
	else 
	{    
		ret = fcp->setupPortDataConn();
		if(ret != true)
		{
  			ShowMsg("Error while server connecting, according to Client Port info");
			sprintf(Log, "Error while server connecting, according to Client Port info");
			fcp->closeCtrlConn();
    		Status = THS_REMOTEFATAL;
			return false;
		}
		tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
	} 

	if (RemoteStartPos >= 0 && tsk->IsResumeSupport == true)
	{
		ret = fcp->ftp_rest(RemoteStartPos);
    	if (ret != true)
    	{
			ShowMsg("REST failed");
			sprintf(Log, "REST failed");
			tsk->saveThreadLog(ThreadId, Log);

			fcp->closeCtrlConn();
			Status = THS_REMOTEFATAL;
			return false;
		}
		else
		{
			tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
			ShowMsg("REST OK");
			sprintf(Log, "REST OK");
			tsk->saveThreadLog(ThreadId, Log);
		}
    }


    ret = fcp->ftp_retr( file.c_str() );

    if (ret != true)
    {
		ShowMsg("RETR failed");
		sprintf(Log, "RETR failed");
		tsk->saveThreadLog(ThreadId, Log);
		fcp->closeCtrlConn();
		Status = THS_REMOTEFATAL;
		return false;
    }
	tsk->saveThreadLog(ThreadId, fcp->getFtpBuf());
    ShowMsg("RETR OK");
	sprintf(Log, "RETR OK");
	tsk->saveThreadLog(ThreadId, Log);

	
	if(tsk->FileSize == -1)
		ret = retrUnknownSizeFile();
	else
		ret = retrKnownSizeFile();

	fcp->closeDataConn();
    fcp->closeCtrlConn();
	
	return ret;

}

bool TFtpThread :: init( void )
{
	return true;
}

int TFtpThread :: exit( void )
{
	return 0;
}

void TFtpThread :: run(void)
{
	char buf[MAX_MSG_SIZE];
	
 	bool FirstTry = true;
	bool ret;
	
	if(tsk->UseProxy)
	{
		fcp = new TFtpCli(tsk->m_proxy);
	}
	else
	{
		fcp = new TFtpCli();
	}

	/* set the thread attributes */
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	do
   	{		
		if (FirstTry != true)
		{
	    	sleep(RETRY_INTERVAL);	
			
			if (TryAttempts == 0)
			{	
				ShowMsg("Trying infinitely...attempt %d", TryAttempts);
				sprintf(buf, "Trying infinitely...attempt %d", TryAttempts);
				tsk->saveThreadLog(ThreadId, buf);
			}
			else
			{
				ShowMsg("Trying....attempt %d of %d", TryAttempts, MAX_TRY_ATTEMPTS);
				sprintf(buf, "Trying....attempt %d of %d", TryAttempts, MAX_TRY_ATTEMPTS);
				tsk->saveThreadLog(ThreadId, buf);
			}
	    	
			delay_ms(100);
	    	Status = THS_IDLE;
		}
		
		if(BlkSize == RemoteBytesReceived)
		{	
	   		Status = THS_ALLDONE;
			ShowMsg("%s have finished its task", ThreadName.c_str() );
			sprintf(buf, "%s have finished its task", ThreadName.c_str() );
			tsk->saveThreadLog(ThreadId, buf);
			if(fcp != NULL)
			{
				delete fcp; fcp = NULL;
			}
			pthread_exit(0);
	    	return;
		}
						
		if (FirstTry == true)
	    	FirstTry = false;
		
		ret = retrFileChunk();

		TryAttempts++;

		
		if (ret == true)
		{	
			ShowMsg("%s has finished its task", ThreadName.c_str() );
			sprintf(buf, "%s have finished its task", ThreadName.c_str() );
			tsk->saveThreadLog(ThreadId, buf);
			if(fcp != NULL)
			{
				delete fcp; fcp = NULL;
			}
			pthread_exit(0);
			return;
		}
	
		
	}while(TryAttempts < MAX_TRY_ATTEMPTS) ;
	
	if(fcp != NULL)
	{
		delete fcp; fcp = NULL;
	}
	Status = THS_MAXTRYS;
	pthread_exit(0);
}

//////////////	Mms Thread Class Impl	/////////////
TMmsThread :: TMmsThread( void )
{
	pthread_key_create( &tg_keyself, NULL );
	pthread_setspecific( tg_keyself, this );
	mcp = NULL;
}

bool TMmsThread :: retrKnownSizeFile(void)
{
	char Log[MAX_MSG_SIZE];
	int ret = -EINVAL;
	int Tos = IPTOS_THROUGHPUT;
    char szBuffer[MMS_BUF_SIZE];	
    char* sdata = NULL;
	FILE * fp = NULL;
	
	off_t Total = 0, BytesRead = 0;
	off_t Start = RemoteStartPos;
	off_t Length = RemoteEndPos - RemoteStartPos;
	
	off_t maxsize = 0;
    off_t timestamp = 0; 
  

    if (!(fp = fopen(LocalFile.c_str(), FileMode.c_str()) ))
    {
		ShowMsg("Error opening file %s for writing: %s",
									LocalFile.c_str(), strerror(errno));
		sprintf(Log, "Error opening file %s for writing: %s",
									LocalFile.c_str(), strerror(errno));
		tsk->saveThreadLog(ThreadId, Log);

		Status = THS_LOCALFATAL;
		return false;
    }
	
	fseeko(fp, LocalStartPos, SEEK_SET);

    /*prioritize packets  */
//	int Sock = mcp->GetSock();
//  setsockopt(Sock, IPPROTO_IP, IP_TOS, (char *)&Tos, sizeof(Tos));

    /*Make sure all writes go directly to the file */
    setvbuf(fp, NULL, _IONBF, 0);
    Status = THS_DOWNLOADING;
   
    ret = mcp->PreProcess(Start, Length, (void *)tsk->uri->unparse().c_str() );
	if (ret < 0)
	{
		ShowMsg("write failed");
		sprintf(Log, "write data to file failed");
		tsk->saveThreadLog(ThreadId, Log);
		fclose(fp);
		return false;
	}
	
	timestamp = MAX_SIZE;
	
	if ((BeginTime.tv_sec == 0) && (BeginTime.tv_usec == 0))
		gettimeofday(&BeginTime, NULL);

	while(Length > 0)
    {
		if(Length < MMS_BUF_SIZE && Length > 0)
		{
		    while(Length > 0)
	    	{
				BytesRead = mcp->Recv(szBuffer, Length, &timestamp);
				
				if(BytesRead == 0) 
				{
					Length = 0;	
					break;
				}

				if(BytesRead == -1)
				{
			    	ShowMsg("Error receving data");
					sprintf(Log, "Error receving data");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);
		    		if (errno == ETIMEDOUT)
		    		{
						ShowMsg("connection timed out");
						sprintf(Log, "connection timed out");
						tsk->saveThreadLog(ThreadId, Log);

						Status = THS_TIMEDOUT;
						return false;
		    		}
		    		Status = THS_REMOTEFATAL;
		    		return false;
				}

				flockfile(fp);
				if(BytesRead !=  fwrite(szBuffer, sizeof(char), BytesRead, fp))
				{
		    		ShowMsg("write failed");
					sprintf(Log, "write data to file failed");
					tsk->saveThreadLog(ThreadId, Log);

		    		fclose(fp);
		    		Status = THS_LOCALFATAL;
		    		return false;
				}
				funlockfile(fp);
				
				Length -= BytesRead;
				Total += BytesRead;												
		 		RemoteStartPos += BytesRead;
				RemoteBytesReceived += BytesRead;
 		 		LocalStartPos += BytesRead;				
	    	}
	    	
	    	break;
		}
		
		BytesRead = mcp->Recv(szBuffer, sizeof(szBuffer), &timestamp);
		
		if(BytesRead == 0) 
		{
			Length = 0;	
			break;
		}

		if(BytesRead == -1)
		{
	    	ShowMsg("Error receving data");
			sprintf(Log, "Error receving data");
			tsk->saveThreadLog(ThreadId, Log);

	    	fclose(fp);
	    	if (errno == ETIMEDOUT)
	    	{
				ShowMsg("connection timed out");
				sprintf(Log, "connection timed out");
				tsk->saveThreadLog(ThreadId, Log);

				Status = THS_TIMEDOUT;
				return false;
	    	}
	    	Status = THS_REMOTEFATAL;
	    	return false;
		}

		flockfile(fp);
		if (BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
		{
	    	ShowMsg("write failed");
			sprintf(Log, "write data to file failed");
			tsk->saveThreadLog(ThreadId, Log);

	    	fclose(fp);
	    	Status = THS_LOCALFATAL;
	    	return false;
		}
		funlockfile(fp);
		
		Length -= BytesRead;		
		Total += BytesRead;
		
		RemoteStartPos += BytesRead;
 		LocalStartPos += BytesRead;				
		RemoteBytesReceived += BytesRead;
				
	}

	if(ThreadId == 0)
	{
	    ret = mcp->PostProcess(&Start, &Length, (void **)&sdata);
		if (ret > 0)
		{
			ret = fseeko(fp, Start, SEEK_SET);
			if (ret < 0)
			{
				if (errno > 0)
					ret = -errno;
				fclose(fp);
				return false;
			}
		
			ret = fwrite(sdata, 1, Length, fp);
			if (ret < 0)
			{
				if (errno > 0)
				ret = -errno;
				fclose(fp);
				return false;
			}		
		}
	}
	
    fclose(fp);
    Status = THS_ALLDONE;
	
    ShowMsg("%s : %lld received", LocalFile.c_str(), Total);
	sprintf(Log, "%s : %lld received", LocalFile.c_str(), Total);
	tsk->saveThreadLog(ThreadId, Log);

	
    return true;
    
}

bool TMmsThread :: retrFileChunk(void)
{
	char Log[MAX_MSG_SIZE];
	
	bool bret = false;	
	FILE * fp = NULL;
	  
    if (Status == THS_ALLDONE)
    {
		gettimeofday(&BeginTime, NULL);
		return true;
    }
  
    Status = THS_CONNECTING;
    bret = mcp->Connect();
    
	if (!bret)
    {
		ShowMsg("Error connecting to %s", tsk->uri->hostname().c_str() );
		sprintf(Log, "Error connecting to %s", tsk->uri->hostname().c_str() );
		tsk->saveThreadLog(ThreadId, Log);

		Status = THS_REMOTEFATAL;
		return false;
    }    
    
    ShowMsg("Connecting to %s", tsk->uri->hostname().c_str() );
	sprintf(Log, "Connecting to %s", tsk->uri->hostname().c_str() );
	tsk->saveThreadLog(ThreadId, Log);

	bret = retrKnownSizeFile();
	
	mcp->Close();	
    
	return bret;
}

bool TMmsThread :: init( void )
{
	return true;
}

int TMmsThread :: exit( void )
{
	return 0;
}

void TMmsThread :: run( void )
{
	char buf[MAX_MSG_SIZE];

	bool FirstTry = true;
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	gettimeofday(&BeginTime, NULL);
	
	mcp = new TMmsCliEx(tsk->uri->unparse().c_str());   

    do
    {
	
		if (FirstTry != true)
		{
	    	sleep(RETRY_INTERVAL);	
			
			if (TryAttempts == 0)
			{	
				ShowMsg("Trying infinitely...attempt %d", TryAttempts);
				sprintf(buf, "Trying infinitely...attempt %d", TryAttempts);
				tsk->saveThreadLog(ThreadId, buf);
			}
			else
			{
				ShowMsg("Trying....attempt %d of %d", TryAttempts, MAX_TRY_ATTEMPTS);
			    sprintf(buf,"Trying....attempt %d of %d", TryAttempts,MAX_TRY_ATTEMPTS);
				tsk->saveThreadLog(ThreadId, buf);
			}
	    	
			delay_ms(400);
	    	Status = THS_IDLE;
		}	  		
		
		if(BlkSize == RemoteBytesReceived)
		{	
	   		Status = THS_ALLDONE;
			ShowMsg("%s have finished its task", ThreadName.c_str() );
			sprintf(buf, "%s have finished its task", ThreadName.c_str() );
			tsk->saveThreadLog(ThreadId, buf);

			if(mcp != NULL)
			{
				delete mcp; mcp = NULL;
			}

			pthread_exit(0);
	    	return;
		}
		
		if (FirstTry == true)
	    	FirstTry = false;

		bool ret = retrFileChunk();
		
		TryAttempts++;

		if (ret == true)
		{
	   		Status = THS_ALLDONE;
			ShowMsg("%s have finished its task", ThreadName.c_str() );
			sprintf(buf, "%s have finished its task", ThreadName.c_str() );
			tsk->saveThreadLog(ThreadId, buf);

			if(mcp != NULL)
			{
				delete mcp; mcp = NULL;
			}

			pthread_exit(0);
	    	return;
		}

	}while (TryAttempts < MAX_TRY_ATTEMPTS);
	
	if(mcp != NULL)
	{
		delete mcp; mcp = NULL;
	}
	
	Status = THS_MAXTRYS;
	pthread_exit(0);
}

//////////////	RTSP Thread Object Impl	/////////////
TRtspThread :: TRtspThread( void )
{
	pthread_key_create( &tg_keyself, NULL );
	pthread_setspecific( tg_keyself, this );
}

bool TRtspThread :: retrKnownSizeFile( void )
{
	char Log[MAX_MSG_SIZE];
   	char szBuffer[RTSP_BUF_SIZE] = { 0 };
	char* sdata = NULL;
	
	int ret = -EINVAL;	
	int Tos = IPTOS_THROUGHPUT;
	
	FILE * fp = NULL;	
	
	off_t Total = 0, BytesRead = 0;
	off_t Start = RemoteStartPos;
	off_t Length = RemoteEndPos - RemoteStartPos;
	
	
	off_t maxsize = 0;
    off_t timestamp = 0;
	
	bool flagx = false;
 	
	char ofile[1024] = { 0 };
	FILE * ofp;
	sprintf(ofile, "%s%d", LocalFile.c_str(), ThreadId);
	
	if (!(fp = fopen(ofile, "wb")))
//  if (!(fp = fopen(LocalFile, FileMode)))
    {
		ShowMsg("Error opening file %s for writing: %s",
										LocalFile.c_str(), strerror(errno));
		sprintf(Log, "Error opening file %s for writing: %s",
										LocalFile.c_str(), strerror(errno));
		tsk->saveThreadLog(ThreadId, Log);
		Status = THS_LOCALFATAL;
		return false;
    }

	//fseeko(fp,  LocalStartPos, SEEK_SET);
	
	/*prioritize packets  */
//	int Sock = rcp->GetSock();
//  setsockopt(Sock, IPPROTO_IP, IP_TOS, (char *)&Tos, sizeof(Tos));

    /*Make sure all writes go directly to the file */ 
    setvbuf(fp, NULL, _IONBF, 0);
    Status = THS_DOWNLOADING;

    gettimeofday(&BeginTime, NULL);
	
	ret = rcp->PreProcess(Start, Length, NULL);
	if (ret < 0)
	{
		ShowMsg("write failed");
		sprintf(Log, "write failed");
		tsk->saveThreadLog(ThreadId, Log);
		fclose(fp);
		return false;
	}

	timestamp = MAX_SIZE;
	
	Length += rcp->GetHdrLen();	
	
/**
	while(Length > 0)
    {
		if(Length < RTSP_BUF_SIZE && Length > 0)
		{
	    	while(Length > 0)
	    	{
				BytesRead = rcp->Recv(szBuffer, Length, Name, &timestamp);
				if(BytesRead == 0) 
				{
					Length = 0;	
					break;
				}
				
				if(BytesRead == -1)
				{
		    		ShowMsg("Error receving data");
		    		fclose(fp);
		    		if (errno == ETIMEDOUT)
		    		{
						ShowMsg("connection timed out");
						Status = THS_TIMEDOUT;
						return RTSPERR;
		    		}
		    		Status = THS_REMOTEFATAL;
		    		return READERR;
				}
				
				flockfile(fp);
				if(BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
				{
		    		ShowMsg("write failed");
		    		fclose(fp);
		    		Status = THS_LOCALFATAL;
		    		return FWRITEERR;
				}
				funlockfile(fp);
										
				Length -= BytesRead;
				Total += BytesRead;
				RemoteBytesReceived += BytesRead;
		 		RemoteStartPos += BytesRead;
 		 		LocalStartPos += BytesRead;				
	    	}
			
	    	break;
		}
		
		BytesRead = rcp->Recv(szBuffer, sizeof(szBuffer), Name, &timestamp);
		if(BytesRead == 0) 
		{
			Length = 0;	
			break;
		}
		
		if(BytesRead == -1)
		{
	    	ShowMsg("Error receving data");
	    	fclose(fp);
	    	if (errno == ETIMEDOUT)
	    	{
				ShowMsg("connection timed out");
				Status = THS_TIMEDOUT;
				return MMSERR;
	    	}
	    	Status = THS_REMOTEFATAL;
	    	return READERR;
		}

		flockfile(fp);
		
		if(flagx == false && ThreadId != 0)
		{
			char * ptr = szBuffer;
			BytesRead = fwrite(ptr + rcp->GetHdrLen(), sizeof(char),
											BytesRead - rcp->GetHdrLen(), fp);
			flagx = true;
		}
		else
		{		
			if (BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
			{
	    		ShowMsg("write failed");
	    		fclose(fp);
	    		Status = THS_LOCALFATAL;
	    		return FWRITEERR;
			}
		
		}
		funlockfile(fp);
		
		Length -= BytesRead;
		Total += BytesRead;
		
		RemoteBytesReceived += BytesRead;
		RemoteStartPos += BytesRead;
 		LocalStartPos += BytesRead;				
 	}
*/


	for (;;)
	{
		BytesRead = rcp->Recv(szBuffer, sizeof(szBuffer), NULL);
		if (BytesRead <= 0)
			break;		

		flockfile(fp);		
		if(flagx == false && ThreadId != 0)
		{
			char * ptr = szBuffer;
			BytesRead = fwrite(ptr + rcp->GetHdrLen(), sizeof(char), 
												BytesRead - rcp->GetHdrLen(), fp);
			flagx = true;
		}
		else
		{
			if (BytesRead != fwrite(szBuffer, sizeof(char), BytesRead, fp))
			{
	    		ShowMsg("write failed");
	    		fclose(fp);
	    		Status = THS_LOCALFATAL;
	    		return false;
			}
		}

		funlockfile(fp);
		
		Length -= BytesRead;
		Total += BytesRead;
		
		RemoteBytesReceived += BytesRead;
		RemoteStartPos += BytesRead;
 		LocalStartPos += BytesRead;				
	}


/**
	if(ThreadId == 0)
	{
    	ret = rcp->PostProcess(&Start, &Length, (void **)&sdata);
		if (ret > 0)
		{
			ret = fseeko(fp, Start, SEEK_SET);
			if (ret < 0)
			{
				if (errno > 0)
					ret = -errno;
				fclose(fp);
				return RTSPERR;
			}
		
			ret = fwrite(sdata, 1, Length, fp);
			if (ret < 0)
			{
				if (errno > 0)
					ret = -errno;
				fclose(fp);
				return RTSPERR;
			}
			Total += ret;
		}
	}
*/

 	fclose(fp);
    Status = THS_ALLDONE;
    ShowMsg("%s : %lld received", LocalFile.c_str(), Total);
	sprintf(Log, "%s : %lld received", LocalFile.c_str(), Total);
	tsk->saveThreadLog(ThreadId, Log);
	
    return true;
}

bool TRtspThread :: retrFileChunk(void)
{
	char Log[MAX_MSG_SIZE];
	
	bool bret = false;	
	FILE * fp = NULL;    
	
    if (Status == THS_ALLDONE)
    {
		gettimeofday(&BeginTime, NULL);
		return true;
    }
	     
    Status = THS_CONNECTING;
    bret = rcp->Connect();
    if (!bret)
    {
		ShowMsg("Error connecting to %s", tsk->uri->hostname().c_str() );
		sprintf(Log, "Error connecting to %s", tsk->uri->hostname().c_str() );
		tsk->saveThreadLog(ThreadId, Log);
		Status = THS_REMOTEFATAL;
		return false;
    }
	
	ShowMsg("Connecting to %s", tsk->uri->hostname().c_str() );
	sprintf(Log, "Connecting to %s", tsk->uri->hostname().c_str() );
	tsk->saveThreadLog(ThreadId, Log);

	bret = retrKnownSizeFile();
	if(bret != true)
		return false;
		
	rcp->Close();
    return true;
}

bool TRtspThread :: init( void )
{
	return true;
}

int TRtspThread :: exit( void )
{
	return 0;
}

void TRtspThread :: run( void )
{
	bool FirstTry = true;
	char buf[MAX_MSG_SIZE];

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	gettimeofday(&BeginTime, NULL);
	rcp = new TRtspCliEx(tsk->uri->unparse().c_str() );      	
    do
    {			
		if (FirstTry != true)
		{
	    	sleep(RETRY_INTERVAL);	
			
			if (TryAttempts == 0)
			{	
				ShowMsg("Trying infinitely...attempt %d", TryAttempts);
				sprintf(buf, "Trying infinitely...attempt %d", TryAttempts);
				tsk->saveThreadLog(ThreadId, buf);
			}
			else
			{
				ShowMsg("Trying....attempt %d of %d", TryAttempts, MAX_TRY_ATTEMPTS);
				sprintf(buf, "Trying....attempt %d of %d", TryAttempts,
														 MAX_TRY_ATTEMPTS);
				tsk->saveThreadLog(ThreadId, buf);
			}
	    	
			delay_ms(400);
	    	Status = THS_IDLE;
		}	  		
	
		if(BlkSize == RemoteBytesReceived)
		{	
	   		Status = THS_ALLDONE;
			ShowMsg("%s have finished its task", ThreadName.c_str() );

			if(rcp != NULL)
			{
				delete rcp; rcp = NULL;
			}

			pthread_exit(0);
	    	return;
		}
		
	   	bool ret = retrFileChunk();
		
		TryAttempts++;

		if (ret == true)
		{
			ShowMsg("%s have finished its task", ThreadName.c_str() );
			
			if(rcp != NULL)
			{
				delete rcp; rcp = NULL;
			}

			pthread_exit(0);
	    	return;
		}

	}while (TryAttempts < MAX_TRY_ATTEMPTS);

	if(rcp != NULL)
	{
		delete rcp; rcp = NULL;
	}

    Status = THS_MAXTRYS;
	pthread_exit(0);
}

/********************* Task Impl*****************************/
TTask :: TTask(const string& sUrl,  int n) : ThreadCnt(n)
{
	int i, Len;

	if(n <= 0) 
		n = 2;

	if(n > MAX_THREADS) 
		n = MAX_THREADS;
	
	for(i = 0; i < MAX_THREADS; i++)
    		ThreadQue[i] = NULL;
			
	for(int i = 0; i < MAX_THREADS; i++)
	{				
		CHS[i].offset =	0;
		CHS[i].finished = 0;
		CHS[i].increase = 0;
	}
	
	for(i = 0; i < MAX_THREADS; i++)
	{
		ThreadLogQue[i].clear();
		ThreadLogQue[i].reserve(100);
	}
	
	uri = new Uri();
	uri->init(sUrl);


	string scheme =  uri->scheme();
	strToLower( scheme, strlen(scheme.c_str()) ) ;
	for(i = 0; i < n; i++)
	{
		if(scheme == "http" ) 	{ ThreadQue[i] = new THttpThread(); }
		if(scheme == "https" )	{ ThreadQue[i] = new THttpThread(); }
		if(scheme == "ftp" )	{ ThreadQue[i] = new TFtpThread();  }
		if(scheme == "mms" )	{ ThreadQue[i] = new TMmsThread();  }
		if(scheme == "rtsp" )	{ ThreadQue[i] = new TRtspThread(); }
		ThreadQue[i]->setOwner(this); 
   	}
		
	Mode = TSK_RUN_NORMAL;
 	pthread_mutex_init(&StatusMutex, NULL);
    pthread_cond_init(&ConnectingCond, NULL);
	
	TotalAvgSpeed = 0;
	MaxSimConns = 0;

	FileSize = -1;
	IsResumeSupport = false;


	UseProxy = false;

	/*** test proxy code 
	IniFile cfg(INSTALLDIR"config/wdget.ini");
	m_proxy.id 	 = 1;
	string proxytype = cfg.read("proxy", "type");

	if( !proxytype.empty() )
	{
		if(proxytype == "http")
			m_proxy.type = PROXY_HTTP;
		if(proxytype == "ftp")
			m_proxy.type = PROXY_FTP;
		if(proxytype == "socks4")
			m_proxy.type = PROXY_SOCK4;
		if(proxytype == "socks5")
			m_proxy.type = PROXY_SOCK5;

		m_proxy.host = cfg.read("proxy", "host");
		m_proxy.port = atoi( cfg.read("proxy", "port").c_str() );
		m_proxy.user = cfg.read("proxy", "user");
		m_proxy.pass = cfg.read("proxy", "pass");
		m_proxy.auth = true;

		UseProxy = true;
	}
	***/


}

TTask :: TTask(const string& sUrl) 
{
	int i;
	
	for(i = 0; i < MAX_THREADS; i++)
    	ThreadQue[i] = NULL;
	
	for(i = 0; i < MAX_THREADS; i++)
	{				
		CHS[i].offset =	0;
		CHS[i].finished = 0;
		CHS[i].increase = 0;
	}

	for(i = 0; i < MAX_THREADS; i++)
	{
		ThreadLogQue[i].clear();
		ThreadLogQue[i].reserve(100);
	}

	uri = new Uri();
	uri->init(sUrl);

	
	Mode = TSK_RUN_NORMAL;

	ThreadCnt = -1;

 	pthread_mutex_init(&StatusMutex, NULL);
    pthread_cond_init(&ConnectingCond, NULL);
	
    TotalAvgSpeed = 0;
	MaxSimConns = 0;
	
	FileSize = -1;
	IsResumeSupport = false;
	
	UseProxy = false;

	/*** test proxy code 
	IniFile cfg(INSTALLDIR"config/wdget.ini");
	m_proxy.id 	 = 1;
	string proxytype = cfg.read("proxy", "type");

	if( !proxytype.empty() )
	{
		if(proxytype == "http")
			m_proxy.type = PROXY_HTTP;
		if(proxytype == "ftp")
			m_proxy.type = PROXY_FTP;
		if(proxytype == "socks4")
			m_proxy.type = PROXY_SOCK4;
		if(proxytype == "socks5")
			m_proxy.type = PROXY_SOCK5;

		m_proxy.host = cfg.read("proxy", "host");
		m_proxy.port = atoi( cfg.read("proxy", "port").c_str() );
		m_proxy.user = cfg.read("proxy", "user");
		m_proxy.pass = cfg.read("proxy", "pass");
		m_proxy.auth = true;

		UseProxy = true;
	}
	***/
	
}

void TTask :: createThreadQue(int nConn)
{
	int i;

	if(nConn <= 0) 
		nConn = 2;

	if(nConn > MAX_THREADS) 
		nConn = MAX_THREADS;
		
	setTaskThreadCnt(nConn);
				
	string scheme =  uri->scheme();
	strToLower( scheme, strlen(scheme.c_str()) ) ;
	for(i = 0; i < nConn; i++)
	{
		if(scheme == "http" ) 	{ ThreadQue[i] = new THttpThread(); }
		if(scheme == "https" )	{ ThreadQue[i] = new THttpThread(); }
		if(scheme == "ftp" )	{ ThreadQue[i] = new TFtpThread();  }
		if(scheme == "mms" )	{ ThreadQue[i] = new TMmsThread();  }
		if(scheme == "rtsp" )	{ ThreadQue[i] = new TRtspThread(); }
		ThreadQue[i]->setOwner(this); 
   	}
}

TTask :: ~TTask(void)
{
	
	for(int i = 0; i < MAX_THREADS; i++)
	{
		if(ThreadQue[i] != NULL)
		{
			delete ThreadQue[i];
			ThreadQue[i] = NULL;
		}
	}

	if(uri != NULL)
	{
		delete uri;
		uri = NULL;	
	}
		
	pthread_cond_destroy( &ConnectingCond ); 
	pthread_mutex_destroy( &StatusMutex );
}

int TTask :: getTaskId(void) 
{ 
	return TaskId; 
}
	
void TTask :: setTaskId(int val) 
{ 
	TaskId = val;
}

const string TTask :: getTaskName(void) 
{ 
	return TaskName; 
}
	
void TTask :: setTaskName(const string& s) 
{ 
	TaskName = s;
}
		
int TTask :: getTaskPrio(void) 
{ 
	return Prio; 
}
	
void TTask :: setTaskPrio(int value) 
{ 
	Prio = value; 
}	
			
TaskStatus TTask :: getTaskStatus(void) 
{ 
	return ts; 
}
	
void TTask :: setTaskStatus(TaskStatus s) 
{ 
	ts = s; 
}
	
void TTask :: setOutDir(const string& s)
{
	OutDir = s;
}

void TTask :: setOutFilePath( const string&  path ) 
{ 
	OutFilePath = path;
}
	

void TTask :: setLogFile( const string&  s ) 
{
	LogFile = s;
}

const string TTask :: getOutdir(void)
{
	return OutDir;	
}

const string TTask :: getOutFilePath( void ) 
{	
	return OutFilePath; 
}

const string TTask :: getLogFile( void )
{
	return LogFile;	
}


/**********************************************/
// This ==> Current Thread 
int TTask :: createThread(TaskThread * This)
{
	int retval = This->create();
	return retval;
}

void TTask :: joinThread(TaskThread * This)
{
	//pthread_join(This->ThreadId, NULL);
	This->joinSelf();
}

void TTask :: cancelThread(TaskThread * This)
{
	//pthread_cancel(This->ThreadId);
	This->suspend();
}

void TTask :: exitTaskThread(TaskThread * This)
{
	pthread_exit(&This->Retval);
}

void TTask :: supendThread(TaskThread * This)
{
	//CancelThread(This);
	This->suspend();
}

void TTask :: resumeThread(TaskThread * This)
{
	//cancelThread(This);
	//This->suspend();
}

void TTask :: lockThread(TTask * This) 
{
	pthread_mutex_lock(&This->StatusMutex); 
}

void TTask :: unlockThread(TTask * This) 
{
    pthread_mutex_unlock(&This->StatusMutex);   
}

void TTask :: threadWaitCond(TTask * This) 
{
    pthread_cond_wait(&This->ConnectingCond, &This->StatusMutex);
}

void TTask :: broadcastCond(TTask * This) 
{
    pthread_cond_broadcast(&This->ConnectingCond);
}

/*******************************************************/
double TTask :: getCurThreadAvgSpeed(TaskThread * This)
{
	struct timeval CurTime;
	long TimeOfCost;
	double AvgSpeed;
	
	gettimeofday(&CurTime, NULL);
	TimeOfCost = CurTime.tv_sec - This->BeginTime.tv_sec;
	
	if(TimeOfCost > 0)
		AvgSpeed = This->RemoteBytesReceived  / TimeOfCost;
    else    
		AvgSpeed = This->RemoteBytesReceived;
	
	return AvgSpeed / 1024;
}

double TTask :: getCurThreadBytesReceived(TaskThread * This)
{
	return (double)This->RemoteBytesReceived / 1024;
}

double TTask :: getCurThreadBlkSize(TaskThread * This)
{
	return (double)This->BlkSize / 1024;
}

const string TTask :: getCurThreadStatus(TaskThread * This)
{
	return ThreadStatusList[This->Status];
}

double TTask :: getCurThreadPercent(TaskThread * This)
{
	 return (double)This->RemoteBytesReceived / This->BlkSize * 100;
}

double TTask :: getCurTaskPercent(void)
{
	off_t AllRemoteBytesReceived = getAllRemoteBytesReceived();
	return (double)AllRemoteBytesReceived / FileSize * 100;
}

off_t TTask :: getAllRemoteBytesReceived(void)
{
	int i;
	off_t AllRemoteBytesReceived = 0;
		
	for(i = 0; i < ThreadCnt; i++)
	{
		if(ThreadQue[i] != NULL)
      		AllRemoteBytesReceived += ThreadQue[i]->RemoteBytesReceived;
    }
	
	return AllRemoteBytesReceived;
}

void TTask :: setTotalAvgSpeed( double val)
{
	TotalAvgSpeed = val;
}

double TTask :: getTotalAvgSpeed(void)
{
	off_t AllRemoteBytesReceived = 0;
	off_t AllLocalBytesReceived = 0;

	double TotalAvgSpeed = 0;
	struct timeval CurTime;
	long TimeOfCost;

    AllRemoteBytesReceived = getAllRemoteBytesReceived();
	
	gettimeofday(&CurTime, NULL);

    TimeOfCost = CurTime.tv_sec - dlStartTime.tv_sec;

  	if(TimeOfCost > 0)
    	TotalAvgSpeed = AllRemoteBytesReceived  / TimeOfCost;
	else
	   	TotalAvgSpeed = AllRemoteBytesReceived ;

	return TotalAvgSpeed / 1024;
}

void TTask :: setdlStartTime(void)
{
	gettimeofday(&dlStartTime, NULL);
}

void TTask :: setdlEndTime(void)
{
	gettimeofday(&dlEndTime, NULL);
}

long TTask :: getTotalCostTime(void)
{
	long TimeOfCost = dlEndTime.tv_sec - dlStartTime.tv_sec;
	return TimeOfCost;
}

////////////////////////////////////////////////////////
bool TTask :: IsAllCompleted(void)
{
	int i;
   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status != THS_ALLDONE)
    			return false;
		}
   	}
   	return true;
}

bool TTask :: IsAllLoginFailed(void)
{
   	int i;
   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status != THS_LOGINFAIL)
    			return false;
		}
   	}
   	return true;
}

bool TTask :: IsAllConnRejected(void)
{
   	int i;
   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status != THS_CONREJECT)
    			return false;
		}
   	}
   	return true;
}

bool TTask :: IsAllRemoteFailed(void)
{
   	int i;
   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status != THS_REMOTEFATAL)
    			return false;
		}
   	}
   	return true;
}

int TTask :: queryAllDoneCnt(void)
{
	int i;
   	int Cnt = 0;

   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status == THS_ALLDONE)
    			Cnt++;
		}
   	}
   	return Cnt;
}


int TTask :: queryConnectingCnt(void)
{
   	int i;
   	int Cnt = 0;

   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status == THS_CONNECTING)
    			Cnt++;
		}
   	}
   	return Cnt;
}

int TTask :: queryLoggingCnt(void)
{
   	int i;
   	int Cnt = 0;

   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status == THS_LOGGININ)
    			Cnt++;
		}
   	}
   	return Cnt;
}

int TTask :: queryDownloadingCnt(void)
{
   	int i;
   	int Cnt = 0;

   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status == THS_DOWNLOADING)
    			Cnt++;
		}
   	}
   	return Cnt;
}

int TTask :: queryRemoteErrorConnCnt(void)
{
    int i;
    int Cnt = 0;

    for (i = 0; i < ThreadCnt; i++)
    {
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status == THS_REMOTEFATAL || 
								ThreadQue[i]->Status == THS_TIMEDOUT)
	    	Cnt++;
		}
    }
    return Cnt;
}


int TTask :: queryLoginfailCnt(void)
{
   	int i;
   	int Cnt = 0;

   	for (i = 0; i < ThreadCnt; i++)
   	{
		if(ThreadQue[i] != NULL)
		{
			if (ThreadQue[i]->Status == THS_LOGINFAIL)
    			Cnt++;
		}
   	}
   	return Cnt;
}

//////////////////////////////////////////////

TTaskManager * TTask :: getTaskMgr(void) 
{ 
	return TskMgr; 
}

void TTask :: setTaskMgr(TTaskManager * val) 
{ 
	TskMgr = val;
}

int TTask :: getTaskThreadCnt(void) const
{
	return ThreadCnt;
}

void TTask :: setTaskThreadCnt(int Val)
{
	ThreadCnt = Val;
}

void TTask :: initTaskThreadQue(void)
{
   	int i;
   	off_t BytesPerConn;
   	off_t BytesLeft;
	char buffer[MAX_PATH_LEN];
	
	if(FileSize == -1)
	{
		BytesPerConn = -1;
		BytesLeft = -1;
		ThreadCnt = 1;
	}
	else
	{
		if(ThreadCnt == 0) ThreadCnt++;
		BytesPerConn = FileSize / ThreadCnt;
		BytesLeft = FileSize % ThreadCnt;
	}

   	for (i = 0; i < ThreadCnt; i++)
   	{
		ThreadQue[i]->ThreadId = i;
		
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s%d", "Thread", i);	
		ThreadQue[i]->ThreadName = buffer;	
		ThreadQue[i]->tsk = this;

		memset(buffer, 0, sizeof(buffer));
    	sprintf(buffer, "%s%s", OutFilePath.c_str(), DEFAULT_FILE_EXT);  	
		ThreadQue[i]->LocalFile = buffer;		
		ThreadQue[i]->FileMode = "r+b";

   		if(FileSize == -1)
		{
			ThreadQue[i]->BlkSize = -1;
			ThreadQue[i]->LocalStartPos = 0;
			ThreadQue[i]->RemoteStartPos = 0;
    		ThreadQue[i]->RemoteEndPos = -1;
 		}
		else
		{
			ThreadQue[i]->BlkSize = BytesPerConn;

			ThreadQue[i]->RemoteStartPos = i * BytesPerConn;
			ThreadQue[i]->LocalStartPos = ThreadQue[i]->RemoteStartPos;

			string scheme =  uri->scheme();
			strToLower( scheme, strlen(scheme.c_str()) ) ;
			if(scheme == "http" ) 	{ ThreadQue[i]->RemoteEndPos = (i + 1) * BytesPerConn; } 			 
			if(scheme == "https" )	{ ThreadQue[i]->RemoteEndPos = (i + 1) * BytesPerConn; }
			if(scheme == "ftp" )	{ ThreadQue[i]->RemoteEndPos = (i + 1) * BytesPerConn; }
			if(scheme == "mms" )	{ ThreadQue[i]->RemoteEndPos = (i + 1) * BytesPerConn; }
			if(scheme == "rtsp" )	{ ThreadQue[i]->RemoteEndPos = (i + 1) * BytesPerConn; }
		}
		
		ThreadQue[i]->RemoteBytesReceived = 0;
		ThreadQue[i]->Status = THS_IDLE;
		
		ThreadQue[i]->Retry = true;
		ThreadQue[i]->TryAttempts = 0;
   	}
	
	if(FileSize != -1 && IsResumeSupport == true && ThreadCnt > 1)
	{
		ThreadQue[--i]->BlkSize += BytesLeft;
   		ThreadQue[i]->RemoteEndPos += BytesLeft;
	}
	
  	for (i = 0; i < ThreadCnt; i++)
   	{
		ShowMsg("%s : [ BlockSize is %lld | StartPos is %lld | EndPos is %lld ]",
				ThreadQue[i]->ThreadName.c_str(), ThreadQue[i]->BlkSize,
				ThreadQue[i]->RemoteStartPos, ThreadQue[i]->RemoteEndPos
		);
   	}
}

int TTask :: execDownloads(void)
{
   	int i;

   	for (i = 0; i < ThreadCnt; i++)
   	{	
		if(createThread(ThreadQue[i]) != 0)
		{
			return -1; 
		}
		usleep(500);
   	} 

/*
    for (i = 0; i < ThreadCnt; i++)
    {   
        joinThread(ThreadQue[i]);
    }   
*/

}

////////////////////////////////////////////////////////
int TTask :: processIdleThread(TaskThread * This)
{
	ShowMsg("Thread%d is Idle...", This->ThreadId);
	return 0;
}

int TTask :: processConnectingThread(TaskThread * This)
{
	ShowMsg("Thread%d Connecting remote host...", This->ThreadId);
	return 0;
}

int TTask :: processLogginInThread(TaskThread * This)
{
	return 0;
}

int TTask :: processDownloadingThread(TaskThread * This)
{
	ShowMsg("Thread%d Connecting remote host successfully.", This->ThreadId);
	return 0;
}

int TTask :: processAllDoneThread(TaskThread * This)
{
	ShowMsg("Thread%d has finished his task!", This->ThreadId);
	return 0;
}

int TTask :: processLoginFailThread(TaskThread * This)
{
 	ShowMsg("Thread%d Login server Fail!", This->ThreadId);
	if(IsAllLoginFailed() == true)
	{
		joinThread(This);
		This->Status = THS_IDLE;
		if(createThread(This) != 0)
			return -1;
		return 0;
	}
	else
	{
		int DCnt = queryDownloadingCnt();
		int ConnCnt = queryConnectingCnt();
		int LoggingCnt = queryLoggingCnt();
					
		if (DCnt == 0 && (ConnCnt == 0) && (LoggingCnt == 0))
		{
			joinThread(This);
			This->Status = THS_IDLE;
			lockThread(this);
			if(createThread(This) != 0)
				return -1;
			threadWaitCond(this);
			unlockThread(this);
			return 0;
		}
		else
		{
	 		if (DCnt > MaxSimConns)
			{
				MaxSimConns = DCnt;
				return 0;
			}
					
			if ((DCnt < MaxSimConns) && (ConnCnt == 0) && (LoggingCnt == 0))
			{
				joinThread(This);
				This->Status = THS_IDLE;
				lockThread(this);
				if(createThread(This) != 0)
	  				return -1;
				threadWaitCond(this);
				unlockThread(this);
				return 0;
			}
		}
	}
}

int TTask :: processConRejecTaskThread(TaskThread * This)
{
	ShowMsg("Thread%d Connecting remote host failed.", This->ThreadId);
	if (IsAllConnRejected() == true)
	{
		joinThread(This);
		This->Status = THS_IDLE;
		if(createThread(This) != 0)
			return -1;
		return 0;
	}
	else
	{
		int DCnt = queryDownloadingCnt();
		int ConnCnt = queryConnectingCnt();
		int LoggingCnt = queryLoggingCnt();
					
		if (DCnt == 0 && ConnCnt == 0 && LoggingCnt == 0)
		{
			joinThread(This);
			This->Status = THS_IDLE;
			lockThread(this);
			if(createThread(This) != 0)
      			return -1;
			threadWaitCond(this);
			unlockThread(this);
			return 0;
		}
		else
		{
			if (DCnt > MaxSimConns)
			{
				MaxSimConns = DCnt;
				return 0;
			}

			if (DCnt < MaxSimConns && ConnCnt == 0 && LoggingCnt == 0)
			{
				joinThread(This);
				This->Status = THS_IDLE;
				lockThread(this);
				if(createThread(This) != 0)
		   			return -1;
				threadWaitCond(this);
				unlockThread(this);
				return 0;
			}
		}
	}
}

int TTask :: processRemoteFatalThread(TaskThread * This)
{
	ShowMsg("Thread%d Connecting remote host failed.", This->ThreadId);
	return 0;
}

int TTask :: processLocalFatalThread(TaskThread * This)
{
	ShowMsg("A local  error occured in Thread%d, aborting...", This->ThreadId);
	return 0;
}

int TTask :: processTimeOuTaskThread(TaskThread * This)
{
	ShowMsg("Thread%d Connecting remote host timeout.", This->ThreadId);
	return 0;
}

int TTask :: processMaxTrysThread(TaskThread * This)
{
	ShowMsg("Thread%d has been tried %d time(s) and has failed. "
					   "Aborting!", This->ThreadId, MAX_TRY_ATTEMPTS);

	return 0;
}

int  TTask :: processUnknownThread(TaskThread * This)
{
	ShowMsg("Thread%d Status unknown.", This->ThreadId);
	return 0;
}

int TTask :: scheduleThread(void)
{
	int i;
	TaskThread * This;

 	for (i = 0; i < ThreadCnt; i++)
   	{
		This = ThreadQue[i];
   		switch (This->Status)
		{
		case THS_IDLE			: 	processIdleThread(This); 			break;
		case THS_CONNECTING		:	processConnectingThread(This); 		break;
		case THS_LOGGININ		:	processLogginInThread(This); 		break;
		case THS_DOWNLOADING	:	processDownloadingThread(This); 	break;
		case THS_ALLDONE		:	processAllDoneThread(This); 		break;
		case THS_LOGINFAIL		: 	processLoginFailThread(This); 		break;
		case THS_CONREJECT 		: 	processConRejecTaskThread(This); 	break;
		case THS_REMOTEFATAL	:	processLoginFailThread(This); 		break;
		case THS_LOCALFATAL 	: 	processLocalFatalThread(This); 		break;
		case THS_TIMEDOUT		:	processLoginFailThread(This); 		break;
		case THS_MAXTRYS		: 	processMaxTrysThread(This); 		break;
		case THS_UNKNOWN		: 	processLoginFailThread(This); 		break;

		default:
  			break;
		}
   	}
}

////////////////////////////////////////////////////////////


void TTask :: terminateThreads(void)
{
   	
	for(int i = 0; i < ThreadCnt; i++)
	{
		cancelThread(ThreadQue[i]);
	}	

	/***
	for(int i = 0; i < ThreadCnt; i++)
	{
		delete ThreadQue[i];
	}	
	***/
	
/*	
    for(int i = 0; i < ThreadCnt; i++)
	{
		JoinThread(ThreadQue[i]);
	}
*/
}

off_t TTask::getEstTime(off_t BytesLeft, double CurSpeed)
{
    off_t SecondsLeft;

    if(BytesLeft < 0 || CurSpeed <=0)
		return 0 ;
    else
		SecondsLeft = BytesLeft / CurSpeed;
   	/*
    if(SecondsLeft >=3600)
	{
		printf("Time remaining %d hours %d minutes %d seconds",
	 			SecondsLeft / 3600, 
				SecondsLeft % 3600 / 60, 
				(SecondsLeft % 3600) % 60);
	}
    else if(SecondsLeft >=60)
	{
		printf("Time remaining %d minutes %d seconds",
				SecondsLeft % 3600 / 60, (SecondsLeft % 3600) % 60);
	}
    else 
		printf("Time remaining %d seconds", SecondsLeft);
	*/
	return SecondsLeft;
}

int TTask :: createLogFile(void)
{
 	int i = 0;
 	FILE *fp = NULL;
 	LogHeader loghdr;
 	ThreadRec tdrec;
 		
     	
    if (!(fp = fopen(LogFile.c_str(), "wb")))
		return -1;
		
	loghdr.FileSize = FileSize;
	loghdr.UrlAddrOff = sizeof(LogHeader);
	loghdr.UrlAddrSize = strlen( uri->unparse().c_str() );
	loghdr.ThreadRecOff = sizeof(LogHeader) + strlen( uri->unparse().c_str() );
	loghdr.ThreadRecSize = sizeof(ThreadRec);
	loghdr.ThreadRecNum = ThreadCnt;
	
	fwrite(&loghdr, sizeof(loghdr), 1, fp);
	fwrite(uri->unparse().c_str(), strlen(uri->unparse().c_str()), 1, fp);
	
	for(i = 0; i < ThreadCnt; i++)
	{
		 tdrec.BlkSize = ThreadQue[i]->BlkSize;
		 tdrec.RemoteStartPos = ThreadQue[i]->RemoteStartPos;
 		 tdrec.RemoteEndPos =ThreadQue[i]->RemoteEndPos;
		 tdrec.RemoteBytesReceived = ThreadQue[i]->RemoteBytesReceived;
		 tdrec.LocalStartPos =	 ThreadQue[i]->LocalStartPos;
		 fwrite(&tdrec, sizeof(tdrec), 1, fp); 	
	}
		
	fclose(fp);
}

int TTask :: loadLogFile(void)
{
  	int i = 0;
  	char buffer[MAX_PATH_LEN]; 	 	
 	int offset, nConn, Len;
 	
 	TaskThread * This;
 	
 	off_t BytesPerConn;
   	off_t BytesLeft;
   	
	LogHeader loghdr;
 	ThreadRec tdrec;	
	FILE *fp = NULL;
    

    if (!(fp = fopen(LogFile.c_str(), "rb")))
		return -1;

		
	if(fread(&loghdr, sizeof(LogHeader), 1, fp) != 1)
	{
		fclose(fp);
		return -1;
    }


    ShowMsg("LogFileHeader : [ FileSize is %lld | ThreadRecNum is %u "
			"| UrlAddrOff is %u  | UrlAddrSize is %u | ThreadRecOff is %u  ]", 
			loghdr.FileSize, loghdr.ThreadRecNum, loghdr.UrlAddrOff, 
			loghdr.UrlAddrSize, loghdr.ThreadRecOff
	);
						
        
    FileSize = loghdr.FileSize;
	IsResumeSupport = true;
    nConn= loghdr.ThreadRecNum;

    //createThreadQue(nConn);
        
	if(FileSize == -1)
	{
		BytesPerConn = -1;
		BytesLeft = -1;
	}
	else
	{
		if(ThreadCnt == 0) ThreadCnt++;		
		BytesPerConn = FileSize / ThreadCnt;
		BytesLeft = FileSize % ThreadCnt;
	}
        
 ////////////////////////////////////////////////////////////
 /*
    offset = loghdr.UrlAddrOff;
	Len = loghdr.UrlAddrSize;
    fseek(fp, offset, SEEK_SET);
    UrlAddr = new char[Len + 1];    
    if(fread(UrlAddr, Len, 1, fp) != 1 )
    {
       	fclose(fp);
       	return -1;
    }
   	uri = new Uri();
   	uri->init(UrlAddr);
*/
////////////////////////////////////////////////////////////   	
    	 	    	
	offset = loghdr.ThreadRecOff;

	fseek(fp, offset, SEEK_SET);
	for(i = 0; i < ThreadCnt; i++)
	{		
		This = ThreadQue[i];
		This->ThreadId = i;

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s%d", "Thread", i);	
		This->ThreadName = buffer;
		
		This->tsk = this;
		
		memset(buffer, 0, sizeof(buffer));
    	sprintf(buffer, "%s%s", OutFilePath.c_str(), DEFAULT_FILE_EXT);
		
		This->LocalFile = buffer;
		This->FileMode = "r+b";

		if(FileSize == -1)
		{
			This->BlkSize = -1;
			This->RemoteStartPos = 0;
    		This->RemoteEndPos = -1;
		}
		else
		{
			if(fread(&tdrec, sizeof(ThreadRec), 1, fp) != 1)
			{
				fclose(fp);
				return -1;	
			}
		
		 	This->BlkSize = tdrec.BlkSize;
		 	This->RemoteStartPos = tdrec.RemoteStartPos;
 		 	This->RemoteEndPos = tdrec.RemoteEndPos;
		 	This->RemoteBytesReceived = tdrec.RemoteBytesReceived;
			This->LocalStartPos =  tdrec.LocalStartPos;

			ShowMsg(
				"ThreadRec :  [ BlockSize is %lld | RemoteStartPos is %lld "
				"| RemoteEndPos is %lld  | RemoteBytesReceived is %lld "
				"| LocalStartPos is %lld  ]", 
				tdrec.BlkSize,
	 	 		tdrec.RemoteStartPos,
 	 	 		tdrec.RemoteEndPos,
	 	 		tdrec.RemoteBytesReceived,
		 		tdrec.LocalStartPos
			);
			
		}
		
		if(This->RemoteEndPos - This->RemoteStartPos == 0)
			This->Status = THS_ALLDONE;
		else
			This->Status = THS_IDLE;
			
		This->Retry = true;
		This->TryAttempts = 0;
		
	}		
	

  	for (i = 0; i < ThreadCnt; i++)
   	{
   		This = ThreadQue[i];
		ShowMsg("%s : [ BlockSize is %lld | StartPos is %lld | EndPos is %lld ]", 
				This->ThreadName.c_str(), This->BlkSize, This->RemoteStartPos, This->RemoteEndPos);
   	}
   	
//	cout << "HELLO" << endl;
	fclose(fp);
		
    return 0;	
}

int TTask :: updateLogFile(void)
{
	FILE *fp = NULL;
 	int offset, i = 0;
 	LogHeader loghdr;
 	ThreadRec tdrec;
 
    if (!(fp = fopen(LogFile.c_str(), "r+b")))
		return -1;
	if(fread(&loghdr, sizeof(LogHeader), 1, fp) != 1)
	{
		fclose(fp);
		return -1;
    }
    	
    offset = loghdr.ThreadRecOff;	
	fseek(fp, offset, SEEK_SET);
		
	for(i = 0; i < ThreadCnt; i++)
	{
		 tdrec.BlkSize = ThreadQue[i]->BlkSize;
		 tdrec.RemoteStartPos = ThreadQue[i]->RemoteStartPos;
 		 tdrec.RemoteEndPos =ThreadQue[i]->RemoteEndPos;
		 tdrec.RemoteBytesReceived = ThreadQue[i]->RemoteBytesReceived;
		 tdrec.LocalStartPos =	 ThreadQue[i]->LocalStartPos;
		 fwrite(&tdrec, sizeof(tdrec), 1, fp); 	
	}
		
	fclose(fp);
	
	return 0;	
}

int TTask :: deleteLogFile(void)
{
    int ret;
     
    ret = unlink(LogFile.c_str());
    if (ret == -1)
    {
		if (errno == ENOENT)
		{
			return 0;
		} 
	    return -1;
    }
    return 0;
}

bool TTask :: IsLogFileExist(void)
{
    struct stat stbuf;
	int ret;

	ret = stat(LogFile.c_str(), &stbuf);
   	if (ret == -1)
		return false;
	else
		return true;	
}

//////////////////////////////////////////////////////
void TTask :: saveThreadLog(int idx, string info)
{
	ThreadLogRec logrec;
	struct tm *p;
 	struct tm *ptm;
	time_t tm;
	char strTime[60];

	tm = time(NULL);
	ptm = localtime(&tm);
	strftime(strTime ,100 , "%Y-%m-%d %H:%M:%S", ptm);
	
	logrec.time = strTime;
	logrec.info = info;
	ThreadLogQue[idx].push_back(logrec);
}

void TTask :: loadThreadLog(int idx)
{
	//Empty
}

/////////////////////////////////////////////////////////////////

bool TTask :: fetchHttpRemoteFileInfo( void )
{
    char LineBuf[MAX_LINE_LEN];
	int port;

	bool ret;
	char Log[LINE_BUFFER];

    int BytesWritten;

	THttpCli * hcp = new THttpCli();

	int cnt = 0;

retry:

	if( uri->port() == 0 ) uri->port(80);

	if(UseProxy)
	{
		hcp->initUri(uri->unparse());
		bool status = hcp->connectProxyHost( m_proxy );
    	if(status != true)
    		return false;
	}
	else
	{

		bool status = hcp->connectRemoteHost( uri->unparse() );
    	if(status != true)
    		return false;
	}

	hcp->buildHostHdr( uri->hostname().c_str(), uri->port() );

	hcp->buildRefererHdr( uri->unparse().c_str() );
   	hcp->buildAcceptHdr();
    hcp->buildUserAgentHdr();
	hcp->buildConnectionHdr();
	if( !uri->user().empty() && !uri->password().empty() )
	{
		hcp->buildBasicAuthHdr( uri->user().c_str(), uri->password().c_str() );
    }
	
	if(UseProxy)
	{
		if(!m_proxy.user.empty())
		{
			hcp->buildProxyAuthHdr(m_proxy.user.c_str(), m_proxy.pass.c_str());
		}
		hcp->buildPragmaHdr();
	}

	if(UseProxy)
    	hcp->sendHttpRequest(VERB_GET, true);
	else
    	hcp->sendHttpRequest(VERB_GET, false);

////////////////////////////////////////////////////////	
	ShowMsg("%s",hcp->getStatusLine().c_str() );

	int nHdr;
	THttpHdr * pHdr;
	saveThreadLog(0, hcp->getStatusLine() );
	nHdr = hcp->getHttpRequest()->GetHdrCount();
	for(int i = 0; i < nHdr; i++)
	{
		pHdr = hcp->getHttpRequest()->GetHdr( i );
		ShowMsg("%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		sprintf(Log, "%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		saveThreadLog(0, Log);
   	}
	
////////////////////////////////////////////////
	int statuscode  = hcp->recvHttpResponse();
///////////////////////////////////////////////////////////	
	ShowMsg("\n%s", hcp->getStatusLine().c_str() );

	saveThreadLog(0, hcp->getStatusLine());
	nHdr = hcp->getHttpResponse()->GetHdrCount();
	for(int i = 0; i < nHdr; i++)
	{
		pHdr = hcp->getHttpResponse()->GetHdr( i );
		ShowMsg("%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		sprintf(Log, "%s: %s", pHdr->GetKey().c_str(), pHdr->GetVal().c_str());
		saveThreadLog(0, Log);
   	}
////////////////////////////////////////////////////////////////

	if ( H_REDIRECTED(statuscode) && cnt < 5 )
	{    
		hcp->closeSocket();

		string redirUrl = hcp->getLocation();  
		uri->init(redirUrl);

		string scheme =  uri->scheme();
		strToLower( scheme, strlen(scheme.c_str()) ) ;

		if(scheme == "ftp" )	 
		{
			ret = fetchFtpRemoteFileInfo();
			if (ret != true)
			{
				ShowMsg("Remote File not Exist");
				return false;
			}
		}


		cnt++;
		goto retry;
	}    

	if (H_20X(statuscode))
	{    
	}    

	if(statuscode == HTTP_MULTIPLE_CHOICES)
	{    
	}    

	// Error StatusCode
	switch (statuscode)
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


    const char * strval = hcp->getTransferEncoding();

	if(strval != NULL)
	{    
		if( strcasecmp(strval, "chunked") == 0 )
		{
			ShowMsg("chunked mode");
		}

	}    


	FileSize = hcp->getContentLen();

	if(FileSize == -1 ) 
		IsResumeSupport = false;
	else
		IsResumeSupport = true;

	ShowMsg("FileSize : %lld IsResumeSupport: %d ", FileSize, IsResumeSupport);
	return true;
}

/** Gets Size about the file from the ftp server 
 * if ftp server did not support the SIZE command, 
 * so lets try to get the file size through the LIST command 
 */
bool TTask :: fetchFtpRemoteFileInfo( void )
{
	bool ret;
	char *tmp;
	char szBuffer[FTP_BUF_SIZE];
	TFtpCli * fcp = NULL;

//	if(UseProxy)
//		fcp = new TFtpCli(m_proxy);
//	else

	fcp = new TFtpCli();
	
	ret = fcp->setupCtrlConn( uri->unparse() );
   	if (ret != true)
   	{
		ShowMsg("Error connecting to %s", uri->hostname().c_str() );
		return ret;
   	}

	saveThreadLog(0, fcp->getFtpBuf());
    ShowMsg("Connect Ok");

    ret = fcp->ftp_login();
    if (ret != true)
    {
		ShowMsg("Login Failed");
		fcp->closeCtrlConn();
		return ret;
    }

	saveThreadLog(0, fcp->getFtpBuf());
	ShowMsg("Login OK");

    ret = fcp->ftp_binary();
    if (ret != true)
    {
		ShowMsg("Binary Failed");
		fcp->closeCtrlConn();
		return ret;
    }

	saveThreadLog(0, fcp->getFtpBuf());
	ShowMsg("Type set to binary ok");

	int pos = uri->path().find_last_of('/');
	string  dir, file;
	if(pos != string::npos)
	{    
		dir = uri->path().substr(1, pos);
		file = uri->path().substr(pos + 1);
    
		if ( file.empty() )
    	{
			ShowMsg("No file specified.");
			return false ;
    	}

		ShowMsg("%s\t%s\t%s",  dir.c_str(), file.c_str(), uri->path().c_str() );
		if(!dir.empty())
		{
			ret = fcp->ftp_cwd( dir.c_str() );
			if (ret != true)
			{
				ShowMsg("CWD failed to change to directory %s", dir.c_str() );
				fcp->closeCtrlConn();
				return ret;
			}
		}
		else
		{
			ShowMsg("CWD is not needed");
		}
	}    

	ShowMsg("CWD OK");


	ret = fcp->ftp_rest(0);
    if (ret != true)
    {
		IsResumeSupport = false;
		ShowMsg("REST failed");
		saveThreadLog(0, fcp->getFtpBuf());
    } 
	else
    {
		IsResumeSupport = true;
		ShowMsg("REST ok");
		saveThreadLog(0, fcp->getFtpBuf());
    }

	/**
    ret = fcp->ftp_cwd(file.c_str());
    if (ret != true)
    {
		ShowMsg("you must be specify a FileName");
		return false;
    } 
	saveThreadLog(0, fcp->getFtpBuf());
	***/

	ret = fcp->ftp_size(file.c_str(), &FileSize );
	if(ret != true)
	{
		saveThreadLog(0, fcp->getFtpBuf());
		return false;
	}

	if(FileSize != -1 ) 
	{
		IsResumeSupport = true;
		ShowMsg("FileSize : %lld \t IsResumeSupport: %d ",  FileSize, IsResumeSupport);
		return true;
	}

	if( fcp->IsPasvMode() )
	{    
		ret = fcp->setupPasvDataConn();
		if( ret != true )
		{
			ShowMsg("Error while connecting, according to servers PASV info");
			fcp->closeCtrlConn();
			return false;
		}
		saveThreadLog(0, fcp->getFtpBuf());
	}    
	else 
	{    
		ret = fcp->setupPortDataConn();
		if(ret != true)
		{
			ShowMsg("Error while server connecting, according to Client Port info");
			fcp->closeCtrlConn();
			return false;
		}
		saveThreadLog(0, fcp->getFtpBuf());
	}    

    ret = fcp->ftp_ascii();
    if (ret != true)
 		return ret;
 	saveThreadLog(0, fcp->getFtpBuf());

    ret = fcp->ftp_list(file.c_str());
    if (ret != true)
	 	return ret;
	saveThreadLog(0, fcp->getFtpBuf());
	
    memset(szBuffer, 0, FTP_BUF_SIZE);
    if (fcp->recvFromDataConn(szBuffer, FTP_BUF_SIZE, 0, 100) == -1)
		return false;

    while ((tmp = strrchr(szBuffer, '\n')) || (tmp = strrchr(szBuffer, '\r')))
    {
		*tmp = 0;
    }

	ShowMsg(szBuffer);
	saveThreadLog(0, szBuffer);
	TFtpParser ftpparser;
    if (ftpparser.parse(szBuffer) == 0)
	{
		ShowMsg("%s", ftpparser.get_file());
		FileSize = ftpparser.get_size();
	}

	if(FileSize == -1 ) 
		IsResumeSupport = false;
	else
		IsResumeSupport = true;

	ShowMsg("FileSize : %lld \t IsResumeSupport: %d ",  FileSize, IsResumeSupport);

	fcp->closeCtrlConn();
	fcp->closeDataConn();
	return true;
}

bool TTask :: fetchMmsRemoteFileInfo( void )
{
	bool bret = false;
	int ret = -EINVAL;
	off_t maxsize = 0;

	TMmsCliEx mce( uri->unparse() );
	bret = mce.Connect();

	if (! bret)  return -1;

	ret = mce.Size(&maxsize);

	if (ret < 0)
	{
		errno = -ret;
		ShowMsg("Retrieve stream size failed");
		return -1;
	}
	else
	{
		ShowMsg("Retrieve stream size: %lu", maxsize);
		IsResumeSupport = true;
	}

	FileSize = maxsize;

	mce.Close();

	return true;
}
bool TTask :: fetchRtspRemoteFileInfo( void )
{
	bool bret = false;
	int ret = -EINVAL;
	off_t maxsize = 0;

	TRtspCliEx rce(uri->unparse());
	bret = rce.Connect();

	if (! bret)
	{
		ret = -ECONNREFUSED;	
		return -1;
	}
	ret = rce.Size(&maxsize);

	if (ret < 0)
	{
		errno = -ret;
		ShowMsg("Retrieve stream size failed");
		return -1;
	}
	else
	{
		ShowMsg("Retrieve stream size: %lu", maxsize);
		IsResumeSupport = true;
	}

	FileSize = maxsize;
	rce.Close();
	return true;
}

int TTask :: fetchRemoteFileInfo( void )
{
	bool ret;
	
	string scheme =  uri->scheme();
	strToLower( scheme, strlen(scheme.c_str()) ) ;


	if( scheme == "http" || scheme == "https" ) 	 
	{
		ret = fetchHttpRemoteFileInfo();
		return 0;	
	}
	
	if(scheme == "ftp" )	 
	{
		bool ret = fetchFtpRemoteFileInfo();
		if (ret != true)
		{
			ShowMsg("Remote File not Exist");
			return -1;
		}
		return 0;	
	}	

	if(scheme == "mms" )	 
	{
		bool bret = false;
		int ret = -EINVAL;
		off_t maxsize = 0;
				
		TMmsCliEx mce( uri->unparse() );
		bret = mce.Connect();
	
		if (! bret)  return -1;

		ret = mce.Size(&maxsize);
	
		if (ret < 0)
		{
			errno = -ret;
			ShowMsg("Retrieve stream size failed");
			return -1;
		}
		else
		{
			ShowMsg("Retrieve stream size: %lu", maxsize);
			IsResumeSupport = true;
		}
	
		FileSize = maxsize;

		mce.Close();
	
		return 0;
	}

	if(scheme == "rtsp" )	
	{
		bool bret = false;
		int ret = -EINVAL;
		off_t maxsize = 0;
		
		TRtspCliEx rce(uri->unparse());
		bret = rce.Connect();
	
		if (! bret)
		{
			ret = -ECONNREFUSED;	
			return -1;
		}
		ret = rce.Size(&maxsize);
	
		if (ret < 0)
		{
			errno = -ret;
			ShowMsg("Retrieve stream size failed");
			return -1;
		}
		else
		{
			ShowMsg("Retrieve stream size: %lu", maxsize);
			IsResumeSupport = true;
		}
	
		FileSize = maxsize;
		rce.Close();
		return 0;
	}
}

int TTask :: fixRealMediaFile(void)
{
	const int FILE_CHUNK = 8192;
	char buf[FILE_CHUNK];

	FILE  *ifp, *ofp;
	int i, j;
			
	char FileName[256];
			
   	memset(FileName, 0, sizeof(FileName));
   	sprintf(FileName, "%s%s", getOutFilePath().c_str(), DEFAULT_FILE_EXT);

	ofp = fopen(FileName, "wb");
	
	for (i = 0; i < ThreadCnt; i++)
	{
	   	memset(FileName, 0, sizeof(FileName));
	   	sprintf(FileName, "%s%s%d", getOutFilePath().c_str(), DEFAULT_FILE_EXT, i);

		if (!(ifp = fopen(FileName, "rb")))
				return -1;
				
		while ((j = fread(buf, sizeof(char), FILE_CHUNK, ifp)))
		{	
			if (fwrite(buf, sizeof(char), j, ofp) != j)
				return -1;
		}
		fclose(ifp);
		unlink(FileName);
	}

	return 0;
}


}

