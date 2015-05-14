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

#ifndef FRMMAINWIN_H
#define FRMMAINWIN_H

#include <qvariant.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qtranslator.h>
#include <qevent.h>

#define USE_URI_H
#define USE_TASK_H
#define USE_INIFILE_H
#define USE_UTILS_H
#include "matrix.h"

using namespace matrix;

#include "listviews.h"
#include "dock.h"
#include "dnd.h"
#include "frmNewTask.h"



class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QProgressBar;
class QLabel;
class TDock;


class NewTaskEvent : public QCustomEvent
{
public:
    NewTaskEvent(NewTaskParam& ntp) :QCustomEvent(QEvent::User + 1005), m_ntp(ntp)
    {
	}
    NewTaskParam m_ntp;
};                                                                                 


class FrmMainWin : public QMainWindow
{
	Q_OBJECT

public:
	FrmMainWin( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~FrmMainWin();

    QMenuBar *MenuBar;
    QPopupMenu *fileMenu;
    QPopupMenu *editMenu;
    QPopupMenu *helpMenu;


    QToolBar *Toolbar;
    QAction* fileNewUrlAction;
    QAction* filePrefAction;
	QAction* fileOpenDirAction;
    QAction* fileExitAction;

	QAction* editStartAction;
    QAction* editStopAction;
	QAction* editRestartAction;
	QAction* editResumeAction;
	QAction* editRemoveAction;
	

	QAction* helpContentsAction;
    QAction* helpIndexAction;
    QAction* helpAboutAction;
    	
public :
    void customEvent(QCustomEvent *);
    void timerEvent( QTimerEvent*);

	void InitDnd(void);
	void Version(void);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void closeEvent( QCloseEvent* ce );
	
public:	

	TTaskManager * TaskMgr;

	ListViews * lvs;
    QLabel   *StatusMsg;
    
    dnd * pdnd;
    TDock* dock;

public:

	void UpdateTaskInfo( TTask * tsk );
	void UpdateCurThreadsInfo(Message * tskmsg);    
	void ReloadThreadLog(int TaskId, int ThreadId);
	///////////////////////////////////
	void ScheduleTask(void);
	void WatchUnique(void);

/////////////////////////////////////////////
	void ProcessStartTask(TTask * tsk);
	void ProcessIdleTask(TTask * tsk);
	void ProcessConnFailTask(TTask * tsk);
	void ProcessThreadErrTask(TTask * tsk);
	void ProcessRunningTask(TTask * tsk);
	void ProcessFinshedTask(TTask * tsk);
	void ProcessRestartTask(TTask * tsk);
	void ProcessResumeTask(TTask * tsk);
	void ProcessStopedTask(TTask * tsk);
	void ProcessZombieTask(TTask * tsk);
	void ProcessDeadTask(TTask * tsk);
	void ProcessErrorTask(TTask * tsk);
	void ProcessFatalTask(TTask * tsk);
	void ProcessUnknownTask(TTask * tsk);
    int ProcessNewTask( NewTaskParam& ntp );
//////////////////////////////////////////////

public slots:
    virtual void fileNewUrl();
    virtual void filePref();
	virtual void fileOpenDir();
    virtual void fileExit();
	
    virtual void dlStart();
    virtual void dlStop();
    virtual void dlRestart();
    virtual void dlResume();
    virtual void dlRemove();
    
	virtual void helpIndex();
    virtual void helpContents();
    virtual void helpAbout();
    
	void ShowDnd(bool b);
	
protected slots:
    virtual void languageChange();

};

#endif // FRMMAINWIN_H
