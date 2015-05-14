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

// QT library
#include <qapplication.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qclipboard.h>
#include <qfiledialog.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <qbitmap.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <qobjectlist.h>


#define USE_URI_H
#define USE_THREAD_H
#define USE_TASKMGR_H
#define USE_TASK_H
#define USE_INIFILE_H
#define USE_UTILS_H

#include "matrix.h"
using namespace matrix;


#include "frmMainWin.h"
#include "frmOptions.h"

#include "netwm.h"
#include "dnd.h"
#include "dock.h"
#include "frmNewTask.h"
#include "listviews.h"


using namespace std;
/*********************  dget - download get for linux  **********/


FrmMainWin * dmw = NULL;
extern QApplication* GlobalApp;

FrmMainWin::FrmMainWin( QWidget* parent, const char* name, WFlags fl )
    									: QMainWindow( parent, name, fl )
{	
	(void)statusBar();
    if ( !name )
	setName( "FrmMainWin" );

	startTimer(1000);
	setAcceptDrops(TRUE);
	setUsesTextLabel(true); 
	setDockMenuEnabled(true);
	usesTextLabel();
	
	setCaption("wdget");
    
    fileNewUrlAction = new QAction( this, "fileNewUrlAction" );
	QPixmap icon0(INSTALLDIR"images/openlog.png");
	fileNewUrlAction->setIconSet( QIconSet(icon0) );
	    
	filePrefAction = new QAction( this, "filePrefAction" );
	QPixmap icon1(INSTALLDIR"images/preferences.png");
    filePrefAction->setIconSet( QIconSet(icon1) );
	
	fileOpenDirAction = new QAction( this, "fileOpenDirAction" );
	QPixmap icon8(INSTALLDIR"images/find.png");
    fileOpenDirAction->setIconSet( QIconSet(icon8) );
	
	fileExitAction = new QAction( this, "fileExitAction" );
	QPixmap icon2(INSTALLDIR"images/add.png");
	fileExitAction->setIconSet( QIconSet(icon2) );
		
    editStartAction = new QAction( this, "editStartAction" );
	QPixmap icon3(INSTALLDIR"images/run.png");
	editStartAction->setIconSet( QIconSet(icon3) );
	
    editStopAction = new QAction( this, "editStopAction" );
	QPixmap icon4(INSTALLDIR"images/stop.png");
    editStopAction->setIconSet( QIconSet(icon4) );

/***	
	editRestartAction = new QAction( this, "editRestartAction" );
	QPixmap icon5(INSTALLDIR"images/runbad.png");
    editRestartAction->setIconSet( QIconSet(icon5) );
	
	editResumeAction = new QAction( this, "editResumeAction" );
	QPixmap icon6(INSTALLDIR"images/runpart.png");
    editResumeAction->setIconSet( QIconSet(icon6) );
***/

	editRemoveAction = new QAction( this, "editRemoveAction" );
	QPixmap icon7(INSTALLDIR"images/del.png");
    editRemoveAction->setIconSet( QIconSet(icon7) );

		
	helpContentsAction = new QAction( this, "helpContentsAction" );
    helpIndexAction = new QAction( this, "helpIndexAction" );
    helpAboutAction = new QAction( this, "helpAboutAction" );

    Toolbar = new QToolBar( QString(""), this, DockTop );
	fileNewUrlAction->addTo( Toolbar );
    filePrefAction->addTo( Toolbar );
	fileOpenDirAction->addTo( Toolbar );
	
	Toolbar->addSeparator();
    editStartAction->addTo( Toolbar );
    editStopAction->addTo( Toolbar );

	/***
    editRestartAction->addTo( Toolbar );
	editResumeAction->addTo( Toolbar );
	Toolbar->addSeparator();
	***/

	editRemoveAction->addTo( Toolbar );
    fileExitAction->addTo( Toolbar );

    // menubar
    MenuBar = new QMenuBar( this, "MenuBar" );
    fileMenu = new QPopupMenu( this );
    fileNewUrlAction->addTo( fileMenu );
    filePrefAction->addTo( fileMenu );
	fileOpenDirAction->addTo( fileMenu );
	
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    MenuBar->insertItem( QString(""), fileMenu, 1 );

    editMenu = new QPopupMenu( this );
    editStartAction->addTo( editMenu );
    editStopAction->addTo( editMenu );
    editMenu->insertSeparator();

	/***
    editRestartAction->addTo( editMenu );
    editResumeAction->addTo( editMenu );
	***/

   	editRemoveAction->addTo( editMenu );
	MenuBar->insertItem( QString(""), editMenu, 2 );

    helpMenu = new QPopupMenu( this );
    helpContentsAction->addTo( helpMenu );
    helpIndexAction->addTo( helpMenu );
   	helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    MenuBar->insertItem( QString(""), helpMenu, 3 );
	
    languageChange();
    //resize( QSize(705, 448).expandedTo(minimumSizeHint()) );
    resize( QSize(850, 550).expandedTo(minimumSizeHint()) );
	clearWState( WState_Polished );
////////////////////////////////////////////////////////////////// 

    lvs = new ListViews(this, "MyView");
    lvs->resize( 640, 480 );
	lvs->setCaption( "Task Manager" );
	
	lvs->contextmenu->insertItem( icon0, tr("NewTask"), this, SLOT( fileNewUrl()) );
    lvs->contextmenu->insertItem( icon3, tr("Start"), this, SLOT( dlStart()) );
    lvs->contextmenu->insertItem( icon4, tr("Stop"), this, SLOT( dlStop() ) );

	/***
    lvs->contextmenu->insertItem( icon5, tr("Restart"), this, SLOT( dlRestart() ) );
    lvs->contextmenu->insertItem( icon6, tr("Resume"), this, SLOT( dlResume() ) );
	***/

    lvs->contextmenu->insertItem( icon7,  tr("Remove"), this, SLOT( dlRemove() ) );
	setCentralWidget(lvs);

	StatusMsg = new QLabel( statusBar(), tr("message") );
    StatusMsg->setAlignment( AlignCenter );
    QFont boldfont;
	boldfont.setWeight(QFont::Bold);
    StatusMsg->setFont( boldfont );
   	statusBar()->addWidget( StatusMsg, 4);
    statusBar()->message( tr("Welcome to wdget"), 1000 );
	
/////////////////////////////////////////////////////////	
	TaskMgr = new TTaskManager();
/////////////////////////////////////////////////////////


	InitDnd();
	ShowDnd(false);
    dock = new TDock(this);
    dock->show();

	/*****signals and slots connections ****/
    connect( fileNewUrlAction, SIGNAL( activated() ), this, SLOT( fileNewUrl() ) );
    connect( filePrefAction, SIGNAL( activated() ), this, SLOT( filePref() ) );
	connect( fileOpenDirAction, SIGNAL( activated() ), this, SLOT( fileOpenDir() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
	connect( editStartAction, SIGNAL( activated() ), this, SLOT( dlStart() ) );
    connect( editStopAction, SIGNAL( activated() ), this, SLOT( dlStop() ) );

	/***
    connect( editRestartAction, SIGNAL( activated() ), this, SLOT( dlRestart() ) );
    connect( editResumeAction, SIGNAL( activated() ), this, SLOT( dlResume() ) );
	***/

    connect( editRemoveAction, SIGNAL( activated() ), this, SLOT( dlRemove() ) );
	connect( helpIndexAction, SIGNAL( activated() ), this, SLOT( helpIndex() ) );
    connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
	/***************************************/
}

/*
 *  Destroys the object and frees any allocated resources
 */
FrmMainWin::~FrmMainWin()
{
//	delete pdnd;
// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void FrmMainWin::languageChange()
{
    setCaption( tr( "dget" ) );
   	fileNewUrlAction->setText( tr( "NewTask" ) );
   	fileNewUrlAction->setMenuText( tr( "NewTask" ) );

	filePrefAction->setText( tr( "Preference" ) );
   	filePrefAction->setMenuText( tr( "Preference" ) );
	
	fileOpenDirAction->setText( tr( "OpenDownloadDir" ) );
   	fileOpenDirAction->setMenuText( tr( "OpenDownloadDir" ) );


	fileExitAction->setText( tr( "Exit" ) );
   	fileExitAction->setMenuText( tr( "Exit" ) );
	editStartAction->setText( tr( "Start" ) );
    editStartAction->setMenuText( tr( "Start" ) );

	editStopAction->setText( tr( "Stop" ) );
    editStopAction->setMenuText( tr( "Stop" ) );

	/***
	editRestartAction->setText( tr( "Restart" ) );
    editRestartAction->setMenuText( tr( "Restart" ) );
	
	editResumeAction->setText( tr( "Resume" ) );
    editResumeAction->setMenuText( tr( "Resume" ) );
	***/

	editRemoveAction->setText( tr( "Remove" ) );
    editRemoveAction->setMenuText( tr( "Remove" ) );

	helpContentsAction->setText( tr( "Contents" ) );
    helpContentsAction->setMenuText( tr( "Contents..." ) );

	helpIndexAction->setText( tr( "Index" ) );
    helpIndexAction->setMenuText( tr( "Index" ) );

	helpAboutAction->setText( tr( "About" ) );
    helpAboutAction->setMenuText( tr( "About" ) );

	Toolbar->setLabel( tr( "Toolbar" ) );
	
    if (MenuBar->findItem(1))
       	MenuBar->findItem(1)->setText( tr( "File" ) );
    if (MenuBar->findItem(2))
       	MenuBar->findItem(2)->setText( tr( "Task" ) );
    if (MenuBar->findItem(3))
       	MenuBar->findItem(3)->setText( tr( "Help" ) );	
}

void FrmMainWin :: fileNewUrl()
{
	char buf[512];

	QClipboard * cb = QApplication::clipboard();
    QString sUrl = cb->text(QClipboard::Clipboard);
	
	if (!sUrl.isEmpty())
    {
    	if (sUrl.find("http://") >= 0 || sUrl.find("ftp://") >= 0 || 
			sUrl.find("https://") >= 0 || sUrl.find("rtsp://") >= 0 || 
			sUrl.find("mms://") >= 0)
        {
        
        }
		else
		{
			sUrl = "";
		}
	}

	sprintf(buf, "%s/%s", getenv("HOME"), "download");

	if(!IsTargetFileExist(buf))
		mkdir(buf, 0777);

	NewTaskParam ntp; 
	frmNewTask nt(sUrl, "", ntp);
	int res = nt.exec();

	if ( res = 0)   return;

	ntp = nt.result();
	sUrl = ntp.origUrl;

	if(sUrl.isEmpty()) return;
	NewTaskEvent* e = new NewTaskEvent(ntp);
	QApplication::postEvent(dmw, e);     


}

void FrmMainWin::filePref()
{
	frmOptions frmPref;
	int ret = frmPref.exec();
	if( ret == 0 ) return;
}

void FrmMainWin::fileOpenDir( void )
{	
	IniFile cfg(INSTALLDIR"config/wdget.ini");
	string shellcmd = "/usr/bin/nautilus --browser ";
	string dlDir = cfg.read("global", "download_dir_path");
	shellcmd += dlDir;
	system(shellcmd.c_str());
}

void FrmMainWin::fileExit()
{	
	
	int ret = QMessageBox::question(this, tr("Question"), 
							tr("Do you want to really exit?"), 
							tr("Yes"), tr("No"));
    if (ret == 0)
	{
		if(TaskMgr->IsEmptyTskQue() != true)
		{       
			delete TaskMgr; //result in Segment fault
		}       
		GlobalApp->quit();
	}
}

void FrmMainWin :: dragEnterEvent(QDragEnterEvent *event)
{
	event->accept(QUriDrag::canDecode(event)||QTextDrag::canDecode(event));
}

void FrmMainWin :: dropEvent(QDropEvent *event)
{
	QStrList list;
    QString str;

    if ( QUriDrag::decode( event, list ) )
    {
        for(const char* u = list.first(); u; u = list.next())
		{
        	if(u) 
				str = u;
		}
    }
    else
    {
        str = QString(event->encodedData("text/plain"));
    }

    if (str.isEmpty()) return;

	QApplication::clipboard()->setText(str);
    fileNewUrl();
}

void FrmMainWin :: closeEvent( QCloseEvent* ce )
{
    ce->ignore();
    hide(); 	
}

void FrmMainWin :: ShowDnd(bool b)
{
    if (b)  
    {
        pdnd->show();
        pdnd->SetOnTop();
    }
    else    
    {
        pdnd->hide();
    }
}

void FrmMainWin:: InitDnd(void)
{
    QString fn = INSTALLDIR"images/dnd.png";
    if ( ! QFile::exists( fn ) )
        exit( 1 );

    QImage img( fn );
    QPixmap p;
    p.convertFromImage( img );

    if ( !p.mask() )
    {
        if ( img.hasAlphaBuffer() ) 
        {
            QBitmap bm;
            bm = img.createAlphaMask();
            p.setMask( bm );
        } 
        else 
        {
            QBitmap bm;
            bm = img.createHeuristicMask();
            p.setMask( bm );
        }       
    }

    pdnd = new dnd(0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | NET::SkipTaskbar);
	pdnd->setBackgroundMode( Qt::PaletteForeground );
    pdnd->setBackgroundPixmap( p );
    pdnd->setFixedSize( p.size() );
	
    if ( p.mask() )
        pdnd->setMask( *p.mask() );
}

void FrmMainWin :: customEvent(QCustomEvent * e)
{    
	if (e->type() != QEvent::User + 1000 && e->type() != QEvent::User + 1005) 
    	return; 
    	
    NewTaskEvent * te = (NewTaskEvent *)e;
    if (te->type() == QEvent::User + 1005) 
    {
		ProcessNewTask(te->m_ntp);
    }
}

void FrmMainWin :: timerEvent( QTimerEvent* event)
{
//	cout << "Timer ID:" << event->timerId() << endl;
	dmw->ScheduleTask();
    dmw->WatchUnique();
}


void FrmMainWin :: UpdateCurThreadsInfo(Message * tskmsg)
{
	int idx = tskmsg->GetTaskId();
	TTask * tsk = TaskMgr->GetTaskById(idx);
	int nConn = tsk->getTaskThreadCnt();
	for(int i = 0; i < nConn; i++)
	{
		tsk->CHS[i].increase = tsk->ThreadQue[i]->RemoteBytesReceived - tsk->CHS[i].finished;
		lvs->DrawLed(idx, tsk->CHS[i].offset, tsk->CHS[i].increase);
		tsk->CHS[i].finished = tsk->ThreadQue[i]->RemoteBytesReceived;
		tsk->CHS[i].offset = tsk->ThreadQue[i]->RemoteStartPos;			
	}
	 
	if(lvs->MyCurThreadId != -1)
		ReloadThreadLog(idx, lvs->MyCurThreadId);
}

void FrmMainWin :: UpdateTaskInfo( TTask * tsk )
{
/////////////Task Info //////////////
	double TotalAvgSpeed;
	off_t AllRemoteBytesReceived;
	double Percent;
/////////////////////////////////////
	QDateTime dt;
	Message* CurTaskMsg;
	TaskStatus ts;
	int idx; 

	if(lvs->GetCurrentFolder()->folderName() == tr("OldTaskBox")) return;
	if(lvs->GetCurrentFolder()->folderName() == tr("TrashBox")) return;

	if(tsk == NULL) return;

	Folder * f = lvs->GetNewTaskBox();

	if(TaskMgr->IsEmptyTskQue()) 
		return;

	if(lvs->MyCurFolder != f)
		return;

	if(f->MsgList->childCount() == 0)
		return;

	idx = tsk->getTaskId();
	CurTaskMsg = lvs->FindTaskMsgById(idx);
	MessageListItem *item = CurTaskMsg->MsgItem;

/////////////////////////Refresh Task Infos ///////////////////////////
	AllRemoteBytesReceived = tsk->getAllRemoteBytesReceived();
	TotalAvgSpeed = tsk->getTotalAvgSpeed();
	tsk->setTotalAvgSpeed(TotalAvgSpeed);
	if(tsk->FileSize != -1) Percent = tsk->getCurTaskPercent();

	dt = QDateTime::currentDateTime();
	dt = dt.addSecs( 60 * 6 ); 


	int pos = tsk->uri->path().find_last_of('/');
	string file = tsk->uri->path().substr(pos + 1); 

	if(tsk->FileSize == -1)
	{    
		QString str1 = file.c_str();
		QString str2 = tr("UNKNOWN");
		QString str3 = QString("%1K/(Sec)").arg(TotalAvgSpeed, 5, 'f',  1);  

		QString str4 = QString("%1K").arg((double)AllRemoteBytesReceived / 1024,
			5, 'f',  2);
		QString str5 = tr("UNKNOWN");
		QString str6 = tr("UNKNOWN");
		QString str7 = tsk->uri->unparse().c_str();
		QString str8 = CurTaskMsg->datetime().toString();

		CurTaskMsg->UpdateTaskMsg(idx, str1, str2, str3, str4, str5, str6, str7, dt);
		item->UpdateItemContent(CurTaskMsg);
	}
	else
	{

		int TimeLeft  = tsk->getEstTime(tsk->FileSize - AllRemoteBytesReceived,	TotalAvgSpeed);
		QString str1 = file.c_str();
		QString str2 = QString("%1K").arg(tsk->FileSize / 1024, 5, 'f',  2);
		QString str3 = QString("%1K/(Sec)").arg(TotalAvgSpeed, 5, 'f',  1);
		QString str4 = QString("%1K").arg((double)AllRemoteBytesReceived / 1024,
			5, 'f',  2);
		QString str5 = QString("%1\%").arg(Percent, 5, 'f', 1);
		QString str6 = QString("%1(Sec)").arg(TimeLeft);
		QString str7 = tsk->uri->unparse().c_str();

		CurTaskMsg->UpdateTaskMsg(idx, str1, str2, str3, str4, str5, str6, str7, dt);
		item->UpdateItemContent(CurTaskMsg);
	}

}

int FrmMainWin :: ProcessNewTask( NewTaskParam& ntp )
{
	int idx, nConn = 0, Retval = 0;

	string Outdir, OutFilePath, LogFilePath;
	
	if (nConn > 10 || nConn < 0)
	{
		ShowMsg("Max Connections is 10");
		return -1;
	}
	
	if(TaskMgr->GetTaskCnt() > MAX_TASK_NUM)
	{
		ShowMsg("Max Concurrent Task is 32");
		return -1;
	}
	
	Uri uri(ntp.origUrl.c_str());

    int pos = uri.path().find_last_of('/');
    string file = uri.path().substr(pos + 1);  
	
	if (file.empty())
	{
		ShowMsg("File not Exist");
		return -1;
	}
	
	if (!ntp.path.empty())
	{
		Outdir = ntp.path;
	}
	else
	{
		Outdir = getenv("HOME");
		Outdir += "/download";
	}
	
	OutFilePath = Outdir + "/" +  file;
	LogFilePath = Outdir + "/" + file + DEFAULT_LOG_EXT;

	if (IsTargetFileExist( OutFilePath.c_str() ) == true)
    {
    	ShowMsg("Target File Exist");
        return -1;
    }
		
	ShowMsg(" %s %s %s \n", Outdir.c_str(), OutFilePath.c_str(), LogFilePath.c_str() );

	StatusMsg->setText("start connecting ...");

	TTask * tsk = TaskMgr->CreateTask( uri.unparse().c_str() );

	if(!ntp.proxyname.empty())
	{
		Proxy_t it; 
		string proxyname = ntp.proxyname;

		it.name = proxyname;

		IniFile cfg(INSTALLDIR"config/wdget.ini");

		string proxytype = cfg.read(proxyname, "type");

		if( !proxytype.empty() ) 
		{
			if (proxytype == "direct")
				it.type = PROXY_DIRECT;
			else if (proxytype == "http")
				it.type = PROXY_HTTP;
			else if (proxytype == "ftp")
				it.type = PROXY_FTP;
			else if (proxytype == "sock4")
				it.type = PROXY_SOCK4;
			else if (proxytype == "sock5")
				it.type = PROXY_SOCK5;

			it.host = cfg.read(proxyname, "host");
			it.port = atoi( cfg.read(proxyname, "port").c_str() );
			it.user = cfg.read(proxyname, "user");
			it.pass = cfg.read(proxyname, "pass");

			if( cfg.read(proxyname, "auth") == "true")
				it.auth = true;
			else
				it.auth = false;

			tsk->m_proxy = it;
			tsk->UseProxy = true;
		}
	}

	tsk->setTaskMgr(TaskMgr);
	tsk->setTaskThreadCnt(ntp.nblks);
	tsk->setOutDir(Outdir);
	tsk->setOutFilePath(OutFilePath);
	tsk->setLogFile(LogFilePath);
	idx = tsk->getTaskId();

//////// Init Task Message for NEWTASK QT GUI ///////////////
	QString sFileSize = tr("UnKnown Size");					
	QString FileName = file.c_str();
	QDateTime dt = QDateTime::currentDateTime();
	dt = dt.addSecs( 60 * 6 );
	
	Message * NewMsg = new Message( idx, FileName, sFileSize, "0.0K", "0.0K", 
										"0.0%", "0(sec)", uri.unparse().c_str(), dt);
										
	Folder * f = lvs->GetNewTaskBox();		
	f->addMessage(NewMsg);	
	MessageListItem * it = new MessageListItem(f->MsgList, NewMsg);
	lvs->folders->setSelected( f->folderItem(), TRUE );
	lvs->AddNewGraphic(idx);
    lvs->AddNewList(idx);
	
    if (idx == 0)
    {
       lvs->ShowGraphic(idx);
       lvs->ShowList(idx);
	   f->MsgList->setSelected( it, TRUE );
    }	
//////////////Init Task Message GUI END//////////////////////


}


void FrmMainWin :: ProcessStartTask(TTask * tsk)
{
	int idx, nConn = 1, Retval = 0;
	char FilePath[1024];
	char LogFilePath[1024];

	if(tsk == NULL) return;
	
	ShowMsg("Init download task...");
	ShowMsg("Getting Remote file Info ...");
	tsk->saveThreadLog(0, "Init download task...");
	tsk->saveThreadLog(0, "Getting Remote file Info ...");
	Retval = tsk->fetchRemoteFileInfo();

	if( Retval == -1 )
    {
    	ShowMsg("Target File Exist or Connect Failed");
		tsk->setTaskStatus(TSK_CONNFAIL);
		return;
    }

	if( tsk->IsResumeSupport == false )
	{
		nConn = 1; 
		tsk->setTaskThreadCnt(nConn);
	}
	else
	{
    	if (tsk->FileSize < MIN_FILE_SIZE)
		{
			nConn = 1;
			tsk->setTaskThreadCnt(nConn);
		}
	}
	
	tsk->saveThreadLog(0, "Init download thread Queue...");
	TaskMgr->InitTask(tsk);			
	idx = tsk->getTaskId();	 
	if(tsk->createLogFile() == -1)
	{
		ShowMsg("Error opening file  for writing logfile");
		return;
	}					

	nConn = tsk->getTaskThreadCnt();
	lvs->InitLed(idx, tsk->FileSize, 16 * 1024);
    lvs->InitListNode(idx, nConn);
	
	for(int i = 0; i < nConn; i++)
	{
		
		lvs->DrawLed(
			idx,
			tsk->ThreadQue[i]->RemoteStartPos,
			tsk->ThreadQue[i]->RemoteBytesReceived
		);
		tsk->CHS[i].offset   = tsk->ThreadQue[i]->RemoteStartPos;
		tsk->CHS[i].finished = tsk->ThreadQue[i]->RemoteBytesReceived;
		tsk->CHS[i].increase = 0;
	}

	TaskMgr->SwitchToTask(tsk);
}

void FrmMainWin :: ProcessRunningTask(TTask * tsk)
{
 	char buffer[MAX_PATH_LEN];

	if(tsk == NULL) return;
///// Schedule Thread //////
	tsk->scheduleThread();
///// Update Task Log File //////
   	tsk->updateLogFile();
	
	UpdateTaskInfo(tsk);
	
	if (tsk->IsAllCompleted() == true)
	{
		tsk->setdlEndTime();
		
		ShowMsg("Total cost time of download is : %d Sec",
											tsk->getTotalCostTime());
	   	ShowMsg("File Succesfully Retreived ");
	    
	   	memset(buffer, 0, sizeof(buffer));
	   	sprintf(buffer, "%s%s", tsk->getOutFilePath().c_str(), DEFAULT_FILE_EXT);


        string scheme =  tsk->uri->scheme();
        strToLower( scheme, strlen(scheme.c_str()) ) ; 

		if(scheme == "rtsp")
		{
			tsk->fixRealMediaFile();
		}
		
		rename(buffer, tsk->getOutFilePath().c_str() );
		
		if ( tsk->deleteLogFile() == -1 )
			ShowMsg( "logfile doesn't exist" );

		ShowMsg( "All Done: Download Succesfull!" );
				
		tsk->setTaskStatus(TSK_FINISHED);
	}
}

void FrmMainWin :: ProcessFinshedTask(TTask * tsk)
{
	if(tsk == NULL) return;
	
	Folder * f1 = lvs->GetNewTaskBox();
	Folder * f2 = lvs->GetOldTaskBox();
	int idx = tsk->getTaskId();
	Message * CurMsg = lvs->FindTaskMsgById(idx);
				
	lvs->DelGraphic(idx);
	lvs->DelList(idx);
				
	MessageListItem *item = CurMsg->MsgItem;
	//f2->MsgList->insertItem(item);
	(void)new MessageListItem( f2->MsgList, CurMsg );
	f1->MsgList->takeItem(item);
							
	f2->addMessage(CurMsg);
	f1->delMessage(CurMsg);

	tsk->setTaskStatus(TSK_DEAD);
	ShowMsg("%s be deleted", tsk->getTaskName().c_str() );
	TaskMgr->DeleteTask(tsk);
	lvs->mpic->OrigListbox->clear();	
}

void FrmMainWin :: ProcessRestartTask(TTask * tsk)
{
	ProcessResumeTask(tsk);
}

void FrmMainWin :: ProcessResumeTask(TTask * tsk)
{
	int idx, nConn;
	char FilePath[1024];
	char LogFilePath[1024];

	if(tsk != NULL)
	{
		tsk->setRunMode(TSK_RUN_RESUME);

		if ( tsk->loadLogFile() == -1 )
		{
			ShowMsg( "Error opening LogFile for reading" );
			return;
		}
		
		nConn = tsk->getTaskThreadCnt();
		idx = tsk->getTaskId();
		lvs->InitLed(idx, tsk->FileSize, 16 * 1024);
    	lvs->InitListNode(idx, nConn);
		
		for(int i = 0; i < nConn; i++)
		{
			tsk->CHS[i].offset = tsk->ThreadQue[i]->BlkSize * i;
			tsk->CHS[i].increase = tsk->ThreadQue[i]->RemoteBytesReceived;
													
			lvs->DrawLed(idx,tsk->CHS[i].offset,tsk->CHS[i].increase);
		
			tsk->CHS[i].finished = tsk->ThreadQue[i]->RemoteBytesReceived;
			tsk->CHS[i].offset = tsk->ThreadQue[i]->RemoteStartPos;
		}
		TaskMgr->SwitchToTask(tsk);  // set  RUNNING
	}
}


void FrmMainWin :: ProcessDeadTask(TTask * tsk)
{
	Folder * f1 = lvs->GetNewTaskBox();
	Folder * f2 = lvs->GetOldTaskBox();
	int idx = tsk->getTaskId();
	Message * CurMsg = lvs->FindTaskMsgById(idx);
				
	lvs->DelGraphic(idx);
	lvs->DelList(idx);
				
	MessageListItem *item = CurMsg->MsgItem;
	(void)new MessageListItem( f2->MsgList, CurMsg );
	f1->MsgList->takeItem(item);
							
	f2->addMessage(CurMsg);
	f1->delMessage(CurMsg);
							
	ShowMsg("%s be deleted", tsk->getTaskName().c_str() );
	TaskMgr->DeleteTask(tsk);	
}

void FrmMainWin :: ProcessStopedTask(TTask * tsk)
{
    if(tsk != NULL)
    {     
        tsk->terminateThreads();
        tsk->setTaskStatus(TSK_ZOMBIE);
    }    
}

void FrmMainWin :: ProcessZombieTask(TTask * tsk)
{
}

void FrmMainWin :: ProcessErrorTask(TTask * tsk)
{
}

void FrmMainWin :: ProcessIdleTask(TTask * tsk)
{
}

void FrmMainWin :: ProcessConnFailTask(TTask * tsk)
{
}

void FrmMainWin :: ProcessThreadErrTask(TTask * tsk)
{
}

void FrmMainWin :: ProcessFatalTask(TTask * tsk)
{
}

void FrmMainWin :: ProcessUnknownTask(TTask * tsk)
{
}

void  FrmMainWin :: ScheduleTask(void)
{
	int nConn = 0;
	int Retval = 0;

	char FilePath[1024];
	char LogFilePath[1024];
	TTask * tsk;
	TaskStatus ts;


	if(TaskMgr->IsEmptyTskQue() == true)
		return;

	for ( int idx = 0; idx < MAX_TASK_NUM; idx++ )
	{
		tsk = TaskMgr->GetTaskById(idx);
		if(tsk == NULL) continue;

		ts = tsk->getTaskStatus();

		if(TaskMgr->QueryRunningTaskCnt() >= 5)
		{	
			if(ts == TSK_START)
				tsk->setTaskStatus(TSK_IDLE);
		}
		else
		{
			if(ts == TSK_IDLE)
				TaskMgr->StartTask(tsk);
		}	


		switch (ts)
		{
		case TSK_START	 	:	ProcessStartTask(tsk); 		break;			
		case TSK_IDLE		: 	ProcessIdleTask(tsk);		break;
		case TSK_RUNNING 	:	ProcessRunningTask(tsk); 	break;
		case TSK_FINISHED	: 	ProcessFinshedTask(tsk); 	break;
		case TSK_RESTART	:	ProcessRestartTask(tsk); 	break;
		case TSK_RESUME		: 	ProcessResumeTask(tsk); 	break;
		case TSK_STOP	 	: 	ProcessStopedTask(tsk); 	break;
		case TSK_ZOMBIE		:	ProcessZombieTask(tsk); 	break;
		case TSK_DEAD		:	ProcessDeadTask(tsk); 		break;
		case TSK_CONNFAIL	:	ProcessConnFailTask(tsk);	break;
		case TSK_THREADERR	:	ProcessThreadErrTask(tsk);	break;
		case TSK_ERROR		:	ProcessErrorTask(tsk); 		break;
		case TSK_FATAL		:	ProcessFatalTask(tsk); 		break;
		case TSK_UNKNOWN	:	ProcessUnknownTask(tsk); 	break;
		}
	}
	

	if(lvs->MyCurTaskMsg != NULL)
	{
		int idx = lvs->MyCurTaskMsg->GetTaskId();
		TTask * tsk = TaskMgr->GetTaskById(idx);
		if(tsk != NULL)
		{
			TaskStatus ts = tsk->getTaskStatus();

           // only update for TSK_IDLE, TSK_ZOMBIE, TSK_RUNNING
		   //if(ts == TSK_IDLE || ts == TSK_ZOMBIE || ts == TSK_RUNNING)
			if(ts == TSK_RUNNING)
			{
				UpdateCurThreadsInfo(lvs->MyCurTaskMsg);
			}
		}
	}
}

// from command line input
void  FrmMainWin :: WatchUnique(void)
{
	FILE* fp = fopen("/tmp/.wdget.lock", "r");

	if (!fp) return;

	vector<QString> vAgrv;

	while (!feof(fp))
	{
		char buf[256];
		memset(buf, 0, sizeof(buf));

		fgets(buf, sizeof(buf), fp);
		QString str = buf;

		if (!str.isEmpty())
			vAgrv.push_back(str);
	}

	fclose(fp);
	
	int size = vAgrv.size();

	if (size <= 2)
	{
		return;
	}

	//////////////////////////////////////
	fp = fopen("/tmp/.wdget.lock", "w");
	if (!fp) return;

	fputs(vAgrv[0].latin1(), fp);
	fputs(vAgrv[1].latin1(), fp);
	fclose(fp);

	for (int i = 0; i < size; ++i)
		printf("%s", vAgrv[i].latin1());
}

void FrmMainWin::dlStart()
{
	/***
	int idx;
	if(lvs->MyCurTaskMsg != NULL)
	{
		idx = lvs->MyCurTaskMsg->GetTaskId();
		TTask * tsk = TaskMgr->GetTaskById(idx);
		if(tsk != NULL)
		{
			if(tsk->getTaskStatus() == TSK_STOP)
				TaskMgr->StartTask(tsk);
		}
	}
	****/

	int idx;
	TTask * tsk;

	if(lvs->MyCurTaskMsg != NULL)
	{
		idx = lvs->MyCurTaskMsg->GetTaskId();
		tsk = TaskMgr->GetTaskById(idx);
		if(tsk != NULL)
		{	
			if( IsTargetFileExist(tsk->getLogFile().c_str()) )
			{
				TaskMgr->ResumeTask(tsk);
			}
		}
	}
}

void FrmMainWin::dlStop()
{
	int idx;
	if(lvs->MyCurFolder == lvs->GetNewTaskBox())
	{
		if(lvs->MyCurTaskMsg != NULL)
		{
			idx = lvs->MyCurTaskMsg->GetTaskId();
			TTask * tsk = TaskMgr->GetTaskById(idx);
			if(tsk != NULL)
			{
				TaskMgr->SupendTask(tsk);
			}
		}
	}
}

void FrmMainWin::dlRestart()
{
	int idx;
	if(lvs->MyCurTaskMsg != NULL)
	{
		idx = lvs->MyCurTaskMsg->GetTaskId();
		TTask * tsk = TaskMgr->GetTaskById(idx);
		if(tsk != NULL)
		{
			TaskMgr->RestartTask(tsk);
		}
	}
}

void FrmMainWin::dlResume()
{	
	int idx;
	TTask * tsk;

	if(lvs->MyCurTaskMsg != NULL)
	{
		idx = lvs->MyCurTaskMsg->GetTaskId();
		tsk = TaskMgr->GetTaskById(idx);
		if(tsk != NULL)
		{	
			if( IsTargetFileExist(tsk->getLogFile().c_str()) )
			{
				TaskMgr->ResumeTask(tsk);
			}
		}
	}
}

void FrmMainWin :: dlRemove()
{
	int idx;
	if(lvs->MyCurTaskMsg != NULL)
	{
		idx = lvs->MyCurTaskMsg->GetTaskId();
		TTask * tsk = TaskMgr->GetTaskById(idx);
		if(tsk != NULL)
		{
			TaskMgr->RemoveTask(tsk);
		}
	}
}


///////////////////////////////////////////////////////////////////////
void FrmMainWin::ReloadThreadLog(int TaskId, int ThreadId)
{
	TTask * tsk;
	int nRec;
	
	lvs->mpic->OrigListbox->clear();	
	tsk = TaskMgr->GetTaskById(TaskId);
	if(tsk == NULL)	return;

	nRec = tsk->ThreadLogQue[ThreadId].size();
	if(nRec == 0) return;
	
	QPixmap pix_time(INSTALLDIR"images/time.png");
	QPixmap pix_info(INSTALLDIR"images/info.png");
	
	vector<ThreadLogRec>& vt = tsk->ThreadLogQue[ThreadId];
	for (int i = nRec - 1 ; i >= 0; i--)
	{
		QString time = vt[i].time.c_str();
		QString info = vt[i].info.c_str();
		CMyQListViewItem* it = new CMyQListViewItem(lvs->mpic->OrigListbox);
		
		it->setText(0, time);
		it->setText(1, info);
		it->setPixmap(0, pix_info);
	}

}

//////////////////////////////////////////////////////////////////////////////

void FrmMainWin::helpIndex()
{
	StatusMsg->setText("helpIndex(): Not implemented yet"INSTALLDIR );
	// 	test FTP LIST CMD over
	//	TFtpCli fc("ftp://www.kernel.org/pub/linux/kernel/v2.0/");	
	//	fc.GetDirList("pub/linux/kernel/v2.0/");
	
}

void FrmMainWin::helpContents()
{	
	StatusMsg->setText("helpContents(): Not implemented yet" );
}

void FrmMainWin::helpAbout()
{
	QMessageBox::about( this, "About <Application>",
		"dget is a multi-thread download tool based on linux platform.\n"
		"develop using C++,  support FTP, HTTP, HTTPS, MMS, RTSP protocol.\n"
		"The Max Thread number limits to 6 for each task.\n"
        "dget 0.5v with QT by yunlong.lee.\n"
		"Email :<yunlong.lee@163.com>\n"
        "WebSite : http://sourceforge.net/projects/dget\n" 
    );
}

void FrmMainWin :: Version(void)
{
}

