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

#ifndef __TASKMGR_H__
#define __TASKMGR_H__

namespace matrix {

class TTaskManager
{
private :
	TTask * TaskQue[MAX_TASK_NUM];
	TaskStatus Status;
	long TaskCnt;
	
public :
	TTaskManager( void ); 
	~TTaskManager( void );
	
	TTask * CreateTask( const char * sUrl, int nConn );	
	TTask * CreateTask( const char * sUrl);	
	int AddTask( TTask * This );
	void DeleteTask( TTask * This );
	void ClearAllTask( void );

	void InitTask( TTask * Task );
	
	void TerminateAllTask( void );		
	void SwitchToTask( TTask * This );
	
	int StopTask( TTask * This );
	void StartTask( TTask * This );
	void SupendTask( TTask * This );
	void ResumeTask( TTask * This );
	void RestartTask( TTask * This );
	void RemoveTask(TTask * This);
	
	TTask * FirstTsk( void ) ;
	TTask * NextTsk( void ) ;
	TTask * LastTsk( void ) ;
	
	int FindFirstNullSlot( void );
	TTask * GetTaskById( int idx );
	bool IsEmptyTskQue( void ) ;
	
	TTask * GetTaskByName(const char * name);
	TaskStatus QueryTaskStatus(TTask * This);	
////////////////////////////////////////////

	bool IsAllTaskCompleted(void);
	int QueryRunningTaskCnt(void);
	int QueryStopTaskCnt(void);
	int QueryDeadTaskCnt(void);
	int QueryFinishedTaskCnt(void);
	int QueryStartTaskCnt(void);
	int QueryIdleTaskCnt(void);

//////////////////////////////////////////////////		
	void InitTaskQue(void);	// for batch Task
	void CreateTaskQue(int nTask); // for batch Task
	int ExecBatchDownloads( void );	// for batch Task
		
////////////////////////////////////////////////////
	
	int  QueryTaskPrio(TTask * This);

	int GetTaskCnt( void ) const;

	
};

}

#endif
