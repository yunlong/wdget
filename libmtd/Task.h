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


#ifndef __TASK_THREAD_H__
#define __TASK_THREAD_H__

namespace matrix {

typedef enum 
{ 
	THS_IDLE, 
	THS_CONNECTING, 
	THS_LOGGININ, 
	THS_DOWNLOADING, 
	THS_ALLDONE, 
	THS_LOGINFAIL, 
	THS_CONREJECT, 
	THS_REMOTEFATAL, 
	THS_LOCALFATAL, 
	THS_TIMEDOUT, 
	THS_MAXTRYS,
	THS_UNKNOWN
}ThreadStatus; 

extern  const string ThreadStatusList[];

typedef pthread_t       threadobj_t;
class TTask;

class TaskThread
{
	friend void* task_thread_start( void* );
private:
    int         m_retval;
    threadobj_t m_thread;
//	pthread_key_t keyself;

public:	
	pthread_t  	ThreadId;
	string 	   	ThreadName; 
	TTask * 	tsk;  // Owner of Thread
	ThreadStatus Status;	

public:
	string 	LocalFile;
	string 	FileMode;

public:
	off_t BlkSize;
	off_t RemoteStartPos;
 	off_t RemoteEndPos;
	off_t RemoteBytesReceived;
	off_t LocalStartPos;

public:
	struct timeval BeginTime;
    struct timeval EndTime;
	struct timeval RetryStart;

public:
	bool Retry;
	int TryAttempts;
	int Retval;

public:
    TaskThread( void );
    virtual ~TaskThread( void );

public:
    int create( void );
    void suspend( void );
	void joinSelf(void);
    void resume( void );
    static TaskThread* This( void );

public:
	TTask * getOwner(void);
	void setOwner(TTask * val);

protected:
    virtual bool init( void );
    virtual void run( void );
    virtual int  exit( void );
};

class THttpCli;
class THttpThread : public TaskThread
{
private:
	THttpCli* hcp;
public:
	bool retrKnownSizeFile(void);
	bool retrUnknownSizeFile(void);
	bool retrFileChunk(void);
public:
	THttpThread( void );
	~THttpThread( void ) { if (hcp != NULL) delete hcp; }
	bool init(void);
	void run(void);
	int exit(void);
};

class TFtpCli;
class TFtpThread : public TaskThread
{
private:
	TFtpCli * fcp;
public:
	bool retrFileChunk(void);
	bool retrKnownSizeFile( void );  
	bool retrUnknownSizeFile( void );
public:
	TFtpThread( void );
	~TFtpThread( void ) { if (fcp != NULL) delete fcp; }
	bool init(void);
	void run(void);
	int exit(void);
};


class TMmsCliEx;
class TMmsThread : public TaskThread
{
private:
	TMmsCliEx  * mcp;
public:
	bool retrFileChunk(void);
	bool retrKnownSizeFile(void);
public:
	TMmsThread( void );
	~TMmsThread( void ) { if (mcp != NULL) delete mcp; }
	bool init(void);
	void run(void);
	int exit(void);
};

class TRtspCliEx;
class TRtspThread : public TaskThread
{
private:
	TRtspCliEx * rcp;
public:	
	bool retrFileChunk(void);
	bool retrKnownSizeFile(void);
public:
	TRtspThread( void );
	~TRtspThread( void ) { if (rcp != NULL) delete rcp; }
	bool init(void);
	void run(void);
	int exit(void);
};

class TTask;
class TTaskThread;

/*
 * 	Log File Format
 * -------------------------------
 *	 LogHeader
 * -------------------------------
 * 	 Url String
 * -------------------------------
 * 	ThreadRec
 * 	ThreadRec
 * 	ThreadRec
 * 	.........
 * -------------------------------
 */
typedef struct 
{
	off_t FileSize;
	unsigned int UrlAddrOff;		/* URL String offset		     */
	unsigned int UrlAddrSize;		/* URL String Length		     */
	unsigned int ThreadRecOff;		/* ThreadRec table entry size    */
	unsigned int ThreadRecSize; 	/* ThreadRec table entry size    */
	unsigned int ThreadRecNum;		/* ThreadRec table entry count   */
}LogHeader;

typedef struct
{
	off_t BlkSize;
	off_t RemoteStartPos;
 	off_t RemoteEndPos;
	off_t RemoteBytesReceived;
	off_t LocalStartPos;			
} ThreadRec;

//cocurrent Task status
typedef enum 
{
	TSK_IDLE,
	TSK_START,				// Get File Size 
	TSK_RUNNING,			
	TSK_FINISHED,			
	TSK_RESTART,				
	TSK_RESUME,				// Resume download 
	TSK_STOP,				// only for running Task;
	TSK_ZOMBIE,				// may be wakeup Resume
	TSK_DEAD,				// before be cleanup completely	
	TSK_CONNFAIL,			//conn fail
	TSK_THREADERR,			
	TSK_ERROR,
	TSK_FATAL,
	TSK_UNKNOWN
}TaskStatus; 

typedef enum { TSK_RUN_NORMAL, TSK_RUN_RESUME } RunMode;

typedef struct 
{
	off_t offset;
	off_t finished;
	off_t increase;
} ThreadInc;

typedef struct 
{
	string time;
	string info;
}ThreadLogRec;

class TTaskManager;
class TTask 
{
private :
	int TaskId;
	string TaskName;

	int Prio;
	int Policy;

private:
	TaskStatus ts;
	
	RunMode Mode;
	TTaskManager * TskMgr; 	// owner of Task	

	int ThreadCnt;
private:
	string OutDir;
	string OutFilePath;
	string LogFile;

private:
	struct timeval dlStartTime;
	struct timeval dlEndTime;
	long TimeOfCost;		// Time of ExecTask 

public: 
	//protect Thread Status after mutiple Thread Exec
	TaskThread 	* ThreadQue[MAX_THREADS];
	pthread_mutex_t StatusMutex;
	pthread_cond_t ConnectingCond; 
public:
///////////////////thread Que info///////////////////////////////////
	Uri			* MirrorQue[MAX_THREADS];  // for mutiple Url download remote file
	ThreadInc 	CHS[MAX_THREADS];

public:
////////////////// thread log Que info//////////////////////////
	vector<ThreadLogRec> ThreadLogQue[MAX_THREADS];
	void saveThreadLog(int idx, string info);
	void loadThreadLog(int idx);

public:
///////////////////////////////////////////////////////////////	
	Uri* uri;
//////////////////////////////////////////////////////////////	
	off_t FileSize;
	bool IsResumeSupport;
/////////////////////////////////////////////////////////////////
	int TotalAvgSpeed;
	int MaxSimConns;

//	off_t AllRemoteBytesReceived;
//	ThreadFunc fn;	
///////////////////////////////////////////////////////////////
	bool UseProxy;
	Proxy_t m_proxy;
//////////////////////////////////////////////////////////////
public:

	TTask(const string& sUrl, int n);
	TTask(const string& sUrl);
	~TTask(void);

	TTaskManager * getTaskMgr(void); 
	void setTaskMgr(TTaskManager * val);

	int getTaskId(void);
	void setTaskId(int val);
	
	const string getTaskName(void);
	void setTaskName(const string& s);
		
	int getTaskPrio(void);
	void setTaskPrio(int value);
			
	TaskStatus getTaskStatus(void);
	void setTaskStatus(TaskStatus s);
		
	const string getOutFilePath( void ); 
	void setOutFilePath( const string& Path ) ;
	
	const string getLogFile( void ); 
	void setLogFile( const string&  s ) ;
	
	const string getOutdir(void);
	void setOutDir( const string&  s);

///////////////////////////////////////////////////////////
	TaskThread * getTaskThreadById(int idx);
	Uri *	  getMirrorUrlById(int idx);

/*********************************************************/
	int createThread(TaskThread * This);
	void joinThread(TaskThread * This);
	
	void cancelThread(TaskThread * This);
	void exitTaskThread(TaskThread * This);

	void supendThread(TaskThread * This);
	void resumeThread(TaskThread * This);

	void lockThread(TTask * This);
	void unlockThread(TTask * This);
	void threadWaitCond(TTask * This);
	void broadcastCond(TTask * This);

/********************************************************/ 
    double getCurThreadAvgSpeed(TaskThread * This);
	double getTotalAvgSpeed(void);
	void setTotalAvgSpeed(double val);
	double getCurThreadPercent(TaskThread * This);
	const string getCurThreadStatus(TaskThread * This);
	double getCurThreadBlkSize(TaskThread * This);
	double getCurThreadBytesReceived(TaskThread * This);

	off_t getAllRemoteBytesReceived(void);
	double getCurTaskPercent(void);

/////////////////////////////////////////////////////////////
	off_t getEstTime(off_t BytesLeft, double CurSpeed);	
	void setdlStartTime(void);
	void setdlEndTime(void);
	long getTotalCostTime(void);
/////////////////////////////////////////////////////////////
	bool IsAllCompleted(void);
	bool IsAllLoginFailed(void);
	bool IsAllConnRejected(void);
	bool IsAllRemoteFailed(void);
////////////////////////////////////////////////	
	void setTaskThreadCnt(int Val);
	int getTaskThreadCnt(void) const;
/////////////////////////////////////////////////
	int queryAllDoneCnt(void);
	int queryConnectingCnt(void);
	int queryLoggingCnt(void);
	int queryDownloadingCnt(void);
	int queryLoginfailCnt(void);
	int queryRemoteErrorConnCnt(void);
//////////////////////////////////////////////////		
	void initTaskThreadQue(void);
	void createThreadQue(int nConn);
	int execDownloads( void );	
		
	void setRunMode(RunMode val) { Mode = val; }
	RunMode getRunMode(void) { return Mode; }
////////////////////////////////////////////////////
	void terminateThreads(void);
	int scheduleThread( void );
////////////////////////////////////////////
	int processIdleThread(TaskThread * This);
	int processConnectingThread(TaskThread * This);
	int processLogginInThread(TaskThread * This);
	int processDownloadingThread(TaskThread * This);
	int processAllDoneThread(TaskThread * This);
	int processLoginFailThread(TaskThread * This);
	int processConRejecTaskThread(TaskThread * This);
	int processRemoteFatalThread(TaskThread * This);
	int processLocalFatalThread(TaskThread * This);
	int processTimeOuTaskThread(TaskThread * This);
	int processMaxTrysThread(TaskThread * This);
	int processUnknownThread(TaskThread * This);

/////////////////////////////////////////////////////
	int createLogFile(void);
	int updateLogFile(void);
	int loadLogFile();
	int writeLogFile(void);
	int deleteLogFile(void);
	bool IsLogFileExist(void);
///////////////////////////////////////////////////////
	bool fetchHttpRemoteFileInfo( void );
	bool fetchFtpRemoteFileInfo( void );
	bool fetchMmsRemoteFileInfo( void );
	bool fetchRtspRemoteFileInfo( void );
	int fetchRemoteFileInfo(void);
//////////////////////////////////////////////////////

	int fixRealMediaFile(void);
};

}

#endif

