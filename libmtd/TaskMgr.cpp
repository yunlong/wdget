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

#define USE_TASK_H
#define USE_TASKMGR_H
#define USE_URI_H
#define USE_UTILS_H
#define USE_THREAD_H

#include "matrix.h"

namespace matrix {


TTaskManager :: TTaskManager( void ) 
{
	for(int i = 0; i < MAX_TASK_NUM; i++)
    	TaskQue[i] = NULL;
	TaskCnt = 0; 
}
	
TTaskManager :: ~TTaskManager( void ) 
{
	TerminateAllTask();
	ClearAllTask();	
}

bool TTaskManager :: IsEmptyTskQue(void) 
{ 
	int i = 0;
	while(i < MAX_TASK_NUM)
	{
		if(TaskQue[i++] != NULL)
			return false;
	}	
	return true;
}

int TTaskManager :: FindFirstNullSlot(void) 
{ 
	int idx = 0;
	while(idx < MAX_TASK_NUM)
	{
		if(TaskQue[idx] == NULL)
			return idx;
		idx++;
	}
	return -1;
}

TTask * TTaskManager :: CreateTask( const char * sUrl)
{	
	char buf[16];
	int idx = 0;	
	
	TTask * NewTask = new TTask(sUrl);
	idx = AddTask(NewTask);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "TASK%d", idx);
	NewTask->setTaskName(buf);		
	NewTask->setTaskStatus(TSK_IDLE);
	return NewTask;
}

TTask * TTaskManager :: CreateTask(const char * sUrl, int nConn)
{
	char buf[16];
	int idx = 0;
	
	if (nConn > 10 || nConn < 0)
		return NULL;
	
	TTask * NewTask = new TTask(sUrl, nConn);
	idx = AddTask(NewTask);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "TASK%d", idx);
	NewTask->setTaskName(buf);
	NewTask->setTaskStatus(TSK_IDLE);	

	return NewTask;
}

TTask * TTaskManager :: GetTaskById( int idx )
{
	return TaskQue[idx];	
}

TTask * TTaskManager :: GetTaskByName(const char * name)
{
	const char * p = name;
	int i = atoi(p + 4);
	return TaskQue[i];
}

int TTaskManager :: AddTask(TTask * This)
{
	int i = -1;
	i = FindFirstNullSlot();
	if(i != -1)
	{
		TaskQue[i] = This;
		This->setTaskId(i);
		TaskCnt++;
		return i;
	}
}

void TTaskManager :: DeleteTask(TTask * This)
{
	int i;
	if(This != NULL)
	{
		i = This->getTaskId();	
		delete This;
		TaskQue[i] = NULL;
		TaskCnt--;
	}
}

void TTaskManager :: ClearAllTask(void)
{
	int i;
	for( i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			delete TaskQue[i];
			TaskQue[i] = NULL;
		}	
	}
	TaskCnt = 0;
}

void TTaskManager :: StartTask( TTask * This )
{
	if(This != NULL)
	{
		if(This->getTaskStatus() == TSK_IDLE)
		{
			This->setTaskStatus(TSK_START);
		}
	}	
}

int TTaskManager :: StopTask(TTask * This)
{
	if(This != NULL)
	{
		if(This->getTaskStatus() == TSK_RUNNING)
		{
			This->setTaskStatus(TSK_STOP);
		}
	}
}

void TTaskManager :: RestartTask( TTask * This )
{
	if(This != NULL)
	{
		if( This->getTaskStatus() == TSK_ZOMBIE )
		{
			This->setTaskStatus(TSK_RESTART);
		}
	}		
}

void TTaskManager :: InitTask( TTask * Task )
{
	char buffer[MAX_PATH_LEN];
	
	FILE * fp = NULL;
	int i, nConn;
	
	nConn = Task->getTaskThreadCnt();

	for(i = 0; i < nConn; i++)
	{
		if(Task->uri != NULL)
		{
			string scheme =  Task->uri->scheme();
			strToLower( scheme, strlen(scheme.c_str()) ) ;

			if(scheme == "http" )
			{
				Task->ThreadQue[i] = new THttpThread(); 
			}

			if(scheme == "https" )
			{
				Task->ThreadQue[i] = new THttpThread();
			}

			if(scheme == "ftp" )
			{
				Task->ThreadQue[i] = new TFtpThread();
			}

			if(scheme == "mms" )
			{
				Task->ThreadQue[i] = new TMmsThread();
			}

			if(scheme == "rtsp" )
			{
				Task->ThreadQue[i] = new TRtspThread();
			}

			Task->ThreadQue[i]->setOwner(Task);
		}
	}

	if(!Task->getOutFilePath().empty() )
    {
    	memset(buffer, 0, sizeof(buffer));
    	sprintf(buffer, "%s%s", Task->getOutFilePath().c_str(), DEFAULT_FILE_EXT);
 		if ((fp = fopen(buffer, "wb")) == NULL)
			return;
	
		if(fseeko(fp, Task->FileSize, SEEK_SET) != -1)
			fclose(fp);
     }
     
	Task->setRunMode(TSK_RUN_NORMAL);
	Task->initTaskThreadQue();	
	
}

void TTaskManager :: TerminateAllTask(void)
{
	int i;
	for( i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
			StopTask(TaskQue[i]);			
	}
}

void TTaskManager :: SwitchToTask( TTask * This )
{
	This->setdlStartTime();
	This->setTaskStatus(TSK_RUNNING);
	This->execDownloads();	
}

void TTaskManager :: SupendTask(TTask * This)
{
	if(This != NULL)
	{
		if(This->getTaskStatus() == TSK_RUNNING)
		{
			StopTask(This);	
		}
	}
}

void TTaskManager :: ResumeTask(TTask * This)
{
	/***
	if(This != NULL)
	{
		if(This->getTaskStatus() == TSK_STOP)
		{
			This->setTaskStatus(TSK_RESUME);
		}
	}
	***/

	if(This != NULL)
	{   
		if( This->getTaskStatus() == TSK_ZOMBIE )
		{    
			This->setTaskStatus(TSK_RESUME);
		}
	}   
}

void TTaskManager :: RemoveTask(TTask * This)
{
	if(This != NULL)
	{
		TaskStatus ts = This->getTaskStatus();
		if( ts == TSK_RUNNING )
		{     
			This->setTaskStatus(TSK_STOP);
			This->setTaskStatus(TSK_DEAD);
		}
		else
		{     
			This->setTaskStatus(TSK_DEAD);
		}
	}
}

//////////////////////////////////////////////////
bool TTaskManager :: IsAllTaskCompleted(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() != TSK_FINISHED)
				return false;	
		}
	}
	return true;
}

int TTaskManager :: QueryRunningTaskCnt(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() == TSK_RUNNING)
				cnt++;	
		}
	}
	return cnt;
}

int TTaskManager :: QueryStopTaskCnt(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() == TSK_STOP)
				cnt++;
		}	
	}
	return cnt;
}

int TTaskManager :: QueryDeadTaskCnt(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() == TSK_DEAD)
				cnt++;
		}	
	}
	return cnt;
}

int TTaskManager :: QueryFinishedTaskCnt(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() == TSK_FINISHED)
				cnt++;	
		}
	}
	return cnt;
}

int TTaskManager :: QueryIdleTaskCnt(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() == TSK_IDLE)
				cnt++;	
		}
	}
	return cnt;
}


int TTaskManager :: QueryStartTaskCnt(void)
{
	int cnt = 0;
	for( int i = 0; i < MAX_TASK_NUM; i++)
	{
		if(TaskQue[i] != NULL)
		{
			if(TaskQue[i]->getTaskStatus() == TSK_START)
				cnt++;	
		}
	}
	return cnt;
}

/////////////////////////////////////////////////////////

TaskStatus TTaskManager :: QueryTaskStatus(TTask * This)
{
	return This->getTaskStatus();
}

int QueryTaskPrio(TTask * This)
{
	return This->getTaskPrio();
}

int TTaskManager :: GetTaskCnt( void ) const
{
	return TaskCnt;
}

}


