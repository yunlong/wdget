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

#include <qlabel.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qiconview.h>
#include <qtable.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qobject.h>
#include <qprogressbar.h>


#define USE_URI_H
#define USE_THREAD_H
#define USE_TASKMGR_H
#define USE_TASK_H
#define USE_INIFILE_H

#include "matrix.h"
using namespace matrix;


#include "clistnode.h"
#include "Led.h"
#include "listviews.h"
#include "frmMainWin.h"
#include "frmNewTask.h"

extern FrmMainWin * dmw;

Folder::Folder( Folder *parent, const QString &name )
    : QObject( parent, name ), fName( name )
{
//	lstMessages.setAutoDelete( FALSE );
	lstMessages.setAutoDelete( TRUE );
	lstMessages.clear();
	MsgList = NULL;
}

// -----------------------------------------------------------------

FolderListItem::FolderListItem( QListView *parent, Folder *f )
    : QListViewItem( parent )
{
    myFolder = f;
	myFolder->FolderItem = this;
	
    setText( 0, f->folderName() );

    if ( myFolder->children() )
		insertSubFolders( myFolder->children() );
}

FolderListItem::FolderListItem( FolderListItem *parent, Folder *f )
    : QListViewItem( parent )
{

    myFolder = f;
	myFolder->FolderItem = this;
	QPixmap icon0(INSTALLDIR"images/stopwait.png");	
	setPixmap(0, icon0);
    setText( 0, f->folderName() );

    if ( myFolder->children() )
		insertSubFolders( myFolder->children() );
}

void FolderListItem::insertSubFolders( const QObjectList *lst )
{
    Folder *f;
    for ( f = ( Folder* )( ( QObjectList* )lst )->first(); f; 
							f = ( Folder* )( ( QObjectList* )lst )->next() )
	(void)new FolderListItem( this, f );
}

// -----------------------------------------------------------------
MessageListItem::MessageListItem( QListView *parent, Message *m )
    : QListViewItem( parent )
{
	
	QPixmap autoicon(INSTALLDIR"images/start.png");
	
	myMessage = m;
	myMessage->MsgItem = this;
	mTaskId = m->mTaskId;
	
	setPixmap(0, autoicon);
    setText( 0, myMessage->FileName() );
    setText( 1, myMessage->FileSize() );
    setText( 2, myMessage->GetAvgSpeed());
    setText( 3, myMessage->GetReceived() );
	setText( 4, myMessage->GetPercent() );
    setText( 5, myMessage->GetTimeLeft() );
	setText( 6, myMessage->Url() );
	setText( 7, myMessage->datetime().toString() );
}

void MessageListItem::UpdateItemContent(Message *m)
{
	QPixmap pauseIcon(INSTALLDIR"images/pause.png");
	
	QPixmap startIcon(INSTALLDIR"images/start.png");
		
	myMessage = m;
	mTaskId = m->mTaskId;

	
	TTask * tsk = dmw->TaskMgr->GetTaskById(mTaskId);
	if(tsk->getTaskStatus() == TSK_ZOMBIE)
		setPixmap(0, pauseIcon);
	if(tsk->getTaskStatus() == TSK_RUNNING)
		setPixmap(0, startIcon);
   	
	setText( 0, m->FileName() );
    setText( 1, m->FileSize() );
    setText( 2, m->GetAvgSpeed());
    setText( 3, m->GetReceived() );
	setText( 4, m->GetPercent() );
    setText( 5, m->GetTimeLeft() );
	setText( 6, m->Url() );
	setText( 7, m->datetime().toString() );
}

void MessageListItem::paintCell( QPainter *p, const QColorGroup &cg,
				 int column, int width, int alignment )
{

    QColorGroup _cg( cg );
    QColor c = _cg.text();

    if ( myMessage->state() == Message::Unread )
		_cg.setColor( QColorGroup::Text, Qt::red );

    QListViewItem::paintCell( p, _cg, column, width, alignment );

    _cg.setColor( QColorGroup::Text, c );

}
// -----------------------------------------------------------------

ThreadMsgListItem::ThreadMsgListItem( QListView * parent, ThreadMsg * m )
    : QListViewItem( parent )
{
    myThreadMsg = m;
	setText( 0, myThreadMsg->GetThreadId() );
    setText( 1, myThreadMsg->GetStatus() );
    setText( 2, myThreadMsg->GetReceived() );
	setText( 3, myThreadMsg->GetAvgSpeed() );
    setText( 4, myThreadMsg->GetPercent() );

}

void ThreadMsgListItem::paintCell( QPainter *p, const QColorGroup &cg,
				 int column, int width, int alignment )
{
    QColorGroup _cg( cg );
    QColor c = _cg.text();

    if ( myThreadMsg->state() == ThreadMsg::Unread )
		_cg.setColor( QColorGroup::Text, Qt::red );

    QListViewItem::paintCell( p, _cg, column, width, alignment );

    _cg.setColor( QColorGroup::Text, c );
}

// -----------------------------------------------------------------

ListViews::ListViews( QWidget *parent, const char *name )
    : QSplitter( Qt::Horizontal, parent, name )
{	
//	lstFolders.setAutoDelete( FALSE );
  	lstFolders.setAutoDelete( TRUE );
    folders = new QListView( this );
	folders->setSelectionMode( QListView::Single );
    folders->header()->setClickEnabled( FALSE );
	folders->header()->setStretchEnabled( TRUE );
	folders->addColumn( tr("dgetwin") );
	initFolders();
    setupFolders();
	folders->setSorting(-1);
	/*
	folders->setSorting(0, false);
	folders->sort();
	***/


    folders->setRootIsDecorated( TRUE );
    setResizeMode( folders, QSplitter::KeepSize );

	folders->firstChild()->setOpen( TRUE );
	Folder * f = GetNewTaskBox();
    folders->setSelected( f->folderItem(), TRUE );

	/////////////////////////////////////////////////////////////////
	QSplitter *vsplitter = new QSplitter( Qt::Vertical, this );	
	MsgLists = new TTaskMsgList( vsplitter );
	initFoldersTaskMsgList();
	
	contextmenu = new QPopupMenu( f->MsgList );	
	
	f->MsgList->show();  

	connect( 
			f->MsgList,
			SIGNAL( contextMenuRequested( QListViewItem *, const QPoint& , int ) ),
	    	this, SLOT( slotRMB( QListViewItem *, const QPoint &, int ) )
	);
	
//  vsplitter->setResizeMode( MsgLists->OrigList, QSplitter::KeepSize );	
//  vsplitter->setResizeMode( MsgLists, QSplitter::KeepSize );

	connect( folders, SIGNAL( selectionChanged(QListViewItem *) ),
	     				this, SLOT( slotFolderChanged( QListViewItem* ) ) );

	connect( folders, SIGNAL( selectionChanged() ),
	     				this, SLOT( slotFolderChanged() ) );

   
	connect( f->MsgList, SIGNAL( selectionChanged() ),
	     				this, SLOT( slotMessageChanged() ) );
	
						
	connect( f->MsgList, SIGNAL( selectionChanged(QListViewItem *) ),
    				this, SLOT( slotMessageChanged( QListViewItem * ) ) );
	
	MyCurFolder = f; 
	MyCurTaskMsg = NULL;
	MyCurThreadId = -1;

	/////////////////debug by yunlong.lee/////////////////////
	QSplitter * hsplitter = new QSplitter(QSplitter::Horizontal,
												vsplitter, "split_h");


	listnode = new CListNode(hsplitter);		
	mpic    = new CPic(hsplitter);	
	mpic->OrigListbox->hide();
	//mpic->OrigLed->hide();
/////////////////////////////////////////	
	vsplitter->setOpaqueResize(TRUE);    
    hsplitter->setOpaqueResize(TRUE);   	 
    setOpaqueResize(TRUE);    
		
	QValueList<int> lst;
    lst.append( 170 );
    setSizes( lst );
}

	
Message * ListViews :: FindTaskMsgById(int taskid)
{	
	Folder * item = GetNewTaskBox();   	
	for ( Message* tskmsg = item->firstMessage(); tskmsg; 
									tskmsg = item->nextMessage() )
	{
		if(tskmsg->GetTaskId() == taskid)
			return tskmsg;	
	}
}

/////////////////debug by balancesli/////////////////////	
	
void ListViews :: slotMessageChanged( QListViewItem* i)
{
	QString s1 = tr("NewTaskBox");
	QListViewItem * j = folders->findItem ( s1, 0);

	MyCurTaskMsg = NULL;

	if ( !j )
		return;
    if ( !j->isSelected() ) 
		return;

    if ( !i )
		return;
	if ( !i->isSelected() ) 
		return;
	
	MyCurTaskMsg = ((MessageListItem *)i)->message();

	if(MyCurTaskMsg != NULL)
	{
		MyCurTaskMsg->setState( Message::Read );
		int idx = MyCurTaskMsg->GetTaskId();
		ShowGraphic(idx);
		ShowList(idx);
	}
}

void ListViews::slotMessageChanged()
{	
	QString s1 = tr("NewTaskBox");
	QListViewItem * j = folders->findItem ( s1, 0);
	
	MyCurTaskMsg = NULL;
	if ( !j )
		return;
    if ( !j->isSelected() ) 
		return;
	
    QListViewItem *i = ((FolderListItem *)j)->folder()->MsgList->currentItem();
	
	if ( !i )
		return;
	
    if ( !i->isSelected() ) 
		return;
	
	MyCurTaskMsg = ((MessageListItem *)i)->message();
	
	if(MyCurTaskMsg != NULL)
	{
		MyCurTaskMsg->setState( Message::Read );
		int idx = MyCurTaskMsg->GetTaskId();
		ShowGraphic(idx);
		ShowList(idx);
	}
}

/////////////////folders process/////////////////////
Folder * ListViews::GetCurrentFolder()
{
	QListViewItem * i = folders->currentItem();
	FolderListItem * item = ( FolderListItem * )i;
	return item->folder();
}

Folder * ListViews :: GetFolderByName(QString& foldername)
{
	QListViewItem * i = folders->findItem ( foldername, 0);
	FolderListItem * item = ( FolderListItem * )i;
	return item->folder();
}

// it be called for Add a message or delete message
void ListViews::RefreshFolder(Folder * item)
{
    item->MsgList->clear();
   	for ( Message* msg = item->firstMessage(); msg; 
										msg = item->nextMessage() )
	{
		(void)new MessageListItem( item->MsgList, msg );
	}
}

Folder * ListViews :: GetNewTaskBox(void)
{
	QString folderName = tr("NewTaskBox");
	Folder * f = GetFolderByName(folderName);
	return f;
}

Folder * ListViews :: GetOldTaskBox(void)
{
	QString folderName = tr("OldTaskBox");
	Folder * f = GetFolderByName(folderName);
	return f;
}

Folder * ListViews :: GetTrashTaskBox(void)
{
	QString folderName = tr("TrashBox");
	Folder * f = GetFolderByName(folderName);
	return f;
}


void ListViews::initFolders( void )
{
    unsigned int mcount = 1;

	QString dlist[3] = {
		tr("TrashBox"),
		tr("OldTaskBox"),
		tr("NewTaskBox"),
	};
	
    for ( unsigned int i = 1; i < 2; i++ ) 
    {
		QString str;
		str = QString( tr("dget") );

		Folder *f1 = new Folder( 0, str );
		for ( unsigned int j = 0; j < 3; j++ )
		{
	    	QString str2;
			str2 = dlist[j];
	    	Folder *f2 = new Folder( f1, str2 );
			
			/* 
			initFolder( f2, mcount );   
	    	for ( unsigned int k = 1; k < 3; k++ ) 
			{
				QString str3;
				str3 = QString( "Sub Sub Folder %1" ).arg( k );
				Folder *f3 = new Folder( f2, str3 );
				initFolder( f3, mcount );
	    	}
			*/
			
		}
		lstFolders.append( f1 );
    }
}

void ListViews::initFolder( Folder *folder, unsigned int &count )
{

/*** 
    unsigned int mcount = 0;
    for ( unsigned int i = 0; i < 1; i++, mcount++ )
    {
		QString str;
		str = "ftp://ftp.gtk.org/pub/gtk/v2.8/pango-1.10.3.tar.bz2";
		QDateTime dt = QDateTime::currentDateTime();
		dt = dt.addSecs( 60 * count );
		Message * msg = new Message( "pango-1.10.3.tar.bz2", 
										"1234k", "DOWNLOADING", str, dt, "100Sec");
		folder->addMessage( msg );
    }
***/

}


void ListViews::setupFolders()
{
    folders->clear();

    for ( Folder* f = lstFolders.first(); f; f = lstFolders.next() )
	{
		(void)new FolderListItem( folders, f );	
	}
}

void ListViews::slotRMB( QListViewItem* Item, const QPoint & point, int )
{
    if( Item )
		contextmenu->popup( point );
}

void ListViews::slotFolderChanged( QListViewItem *i )
{
    if ( !i )
		return;
	
	FolderListItem *item = ( FolderListItem* )i;
	Folder * f = item->folder();
	dmw->StatusMsg->setText(f->folderName());
	if(f->MsgList == NULL)
		return;	
	ShowTaskMsgList(f);
	MyCurFolder = f;
}

void ListViews::slotFolderChanged( void )
{
	FolderListItem *item = ( FolderListItem*)folders->currentItem();
	Folder * f = item->folder();
	dmw->StatusMsg->setText(f->folderName());
	if(f->MsgList == NULL)
		return;
	ShowTaskMsgList(f);
}
/////////////////////////////////////////////////////

void ListViews :: initFoldersTaskMsgList(void)
{
	for ( Folder* it = lstFolders.first(); it; it = lstFolders.next() )
	{
	    if ( it->children() )
		{
			it->MsgList = NULL;
			const QObjectList *lst = it->children();
			
    		for ( Folder* fdr = ( Folder* )( ( QObjectList* )lst )->first(); fdr; 
							fdr = ( Folder* )( ( QObjectList* )lst )->next() )
			{
				AddTaskMsgList(fdr);
			}
		}
	}
}

void ListViews::AddTaskMsgList(Folder * f)
{
    f->MsgList = new QListView( MsgLists );
	f->MsgList->setSelectionMode( QListView::Single );
	f->MsgList->clear();
	f->MsgList->addColumn( tr("FileName") );
	f->MsgList->setColumnWidth(0, 100);
	f->MsgList->setColumnAlignment(0, Qt::AlignLeft);
	
    f->MsgList->addColumn( tr("FileSize") );
	f->MsgList->setColumnWidth(1, 100);
	f->MsgList->setColumnAlignment(1, Qt::AlignRight);

	f->MsgList->addColumn( tr("AvgSpeed") );
	f->MsgList->setColumnWidth(2, 60);
	f->MsgList->setColumnAlignment(2, Qt::AlignRight);

	f->MsgList->addColumn( tr("Received") );
	f->MsgList->setColumnWidth(3, 100);
	f->MsgList->setColumnAlignment(3, Qt::AlignRight);
	
	f->MsgList->addColumn( tr("Percent") );
	f->MsgList->setColumnWidth(4, 60);
	f->MsgList->setColumnAlignment(4, Qt::AlignRight);

	f->MsgList->addColumn( tr("TimeLeft") );
	f->MsgList->setColumnWidth(5, 100);
 	f->MsgList->setColumnAlignment(5, Qt::AlignRight);

	f->MsgList->addColumn( tr("URL") );
	f->MsgList->setColumnWidth(6, 100);
	f->MsgList->setColumnAlignment(6, Qt::AlignHCenter);

	f->MsgList->addColumn( tr("Date") );  
	f->MsgList->setColumnWidth(7, 100);
	f->MsgList->setColumnAlignment(7, Qt::AlignRight);
	
	///test
	/*
	f->MsgList->addColumn( f->folderName() );
	f->MsgList->setColumnWidth(0, 100);
	f->MsgList->setColumnAlignment(1, Qt::AlignLeft);
	*/
    
	f->MsgList->setAllColumnsShowFocus( TRUE );
    f->MsgList->setShowSortIndicator( TRUE );
	f->MsgList->setSelected( f->MsgList->firstChild(), TRUE );
    f->MsgList->setCurrentItem( f->MsgList->firstChild() );	

	/*
	connect( f->MsgList, SIGNAL( selectionChanged(QListViewItem *) ),
    				this, SLOT( slotMessageChanged( QListViewItem * ) ) );
	*/
	
	MsgLists->TaskMsgListLayout->addWidget( f->MsgList, 0, 0 );
    f->MsgList->hide();
//	InitTaskMsgList(f);
}

void ListViews::InitTaskMsgList(Folder * f)
{

}


void ListViews::DelTaskMsgList(Folder * f)
{
	delete f->MsgList;
	f->MsgList = NULL;
}

void ListViews::ShowTaskMsgList(Folder * f)
{

	f->MsgList->show();
	
	for ( Folder* it = lstFolders.first(); it; it = lstFolders.next() )
	{		
	    if ( it->children() )
		{
			const QObjectList *lst = it->children();
			
    		for ( Folder* fdr = ( Folder* )( ( QObjectList* )lst )->first(); fdr; 
							fdr = ( Folder* )( ( QObjectList* )lst )->next() )
			{
				if(fdr == f)
					continue;
					
				fdr->MsgList->hide();			
			}
		}
	}
}


////////////////////////////////////////////

void ListViews::ListNodeSelectChanged(QListViewItem* i)
{
	if (i == NULL)
		return;

	int numChild = ((CMyQListViewItem *)i)->childCount();
	int threadID = ((CMyQListViewItem *)i)->ThreadId;

	//cout << numChild << ":" << threadID << endl;
	if (threadID == -1 && numChild == 0)
	{
        fprintf(stdout, "entern into root i->idx: %d, i->thread: %d", ((CMyQListViewItem *)i)->idx, threadID);
        return; 
    }

	if (numChild != 0)
	{
		mpic->OrigListbox->hide();
		if(MyCurTaskMsg != NULL)
		{
			ShowGraphic(MyCurTaskMsg->GetTaskId());
		}
	}
	else
	{

		if(MyCurTaskMsg != NULL)
		{
			int taskid = MyCurTaskMsg->GetTaskId();
			dmw->ReloadThreadLog(taskid, threadID);			
			MyCurThreadId = threadID;
		}
		ShowListBox();
	}

}

void ListViews::SetOrigListNodeShow(bool b)
{
	if (b)
		listnode->OrigList->show();
	else
		listnode->OrigList->hide();
}

void ListViews::AddNewList(int idx)
{
	Message * msg = FindTaskMsgById(idx);
    msg->list = new QListView(listnode);
    msg->list->header()->hide();
    msg->list->addColumn("");
    listnode->CListNodeLayout->addWidget( msg->list, 0, 0 );
    msg->list->hide();
    InitListNode(idx, 1);
	
	connect(msg->list, SIGNAL(selectionChanged(QListViewItem*)), this,
					       SLOT(ListNodeSelectChanged(QListViewItem*)));
}

void ListViews::DelList(int idx)
{
	Message * msg = FindTaskMsgById(idx);
	delete msg->list;
	msg->list = NULL;
}

void ListViews::ShowListBox(void)
{
	Folder * item = GetNewTaskBox();
	for(Message * cur = item->firstMessage(); cur; cur=item->nextMessage())
	{
		cur->led->hide();
	}
	mpic->OrigListbox->show();	
}

void ListViews::ShowList(int idx)
{
	Message * msg = FindTaskMsgById(idx);
	msg->list->show();

	Folder * item = GetNewTaskBox();
	for(Message * cur = item->firstMessage(); cur; cur=item->nextMessage())
	{
		if (cur == msg)
			continue;
		cur->list->hide();
	}
}


void ListViews::InitListNode(int idx, int num)
{
	QPixmap pix_graphic(INSTALLDIR"images/graphic.png");
	QPixmap pix_thread(INSTALLDIR"images/thread.png");

	Message * msg = FindTaskMsgById(idx);
    msg->list->clear();

    CMyQListViewItem* rtLV = new CMyQListViewItem(msg->list);
    rtLV->setText(0, tr("Graphic/Log") );
    rtLV->setOpen(TRUE);
    rtLV->setPixmap(0, pix_graphic);
    rtLV->setExpandable(TRUE);

    for (int i = 0; i < num; ++i)
    {
		QString s;
		s.setNum(i + 1);
        QString t = tr("Thread");
        t += s;
        CMyQListViewItem * node = new CMyQListViewItem(rtLV);
        node->ThreadId = i;
        node->setText(0, t);
        node->setPixmap(0, pix_thread);
    }
}

void ListViews::InitLed(int idx, off_t file_total_size, off_t per_size)
{
	Message * msg = FindTaskMsgById(idx);
	return msg->led->InitLed(file_total_size, per_size);
}

void ListViews::DrawLed(int idx, off_t offset, off_t length)
{
	Message * msg = FindTaskMsgById(idx);
	return msg->led->DrawLed(offset, length);
}

void ListViews::AddNewGraphic(int idx)
{
	Message * msg = FindTaskMsgById(idx);
//	TTask * tsk = dmw->TaskMgr->GetTaskById(idx);
	
	msg->led = new TLedImpl(mpic);
	mpic->CPicLayout->addWidget( msg->led, 0, 0 );
	msg->led->hide();
	
//////////////////////////////////////////////
/*
	msg->map = new MapChart(dmw->pdnd, 0, 0, 500);
    msg->map->setStep(3);
	int *p = &(tsk->TotalAvgSpeed);
    msg->map->start(p);
	dmw->pdnd->mapLayout->addWidget(msg->map, 0, 0 );
    msg->map->hide();
*/
///////////////////////////////////////////////////
}

void ListViews::DelGraphic(int idx)
{
	Message * msg = FindTaskMsgById(idx);
	delete msg->led;
//	delete msg->map;
	
	msg->led = NULL;
//	msg->map = NULL;
	mpic->OrigListbox->clear();
}

void ListViews::ShowGraphic(int idx)
{
	Message * msg = FindTaskMsgById(idx);
	msg->led->show();
//	msg->map->show();
	
	Folder * item = GetNewTaskBox();
	for(Message * cur = item->firstMessage(); cur; cur=item->nextMessage())
	{
		if (msg == cur)
			continue;
		cur->led->hide();
//		cur->map->hide();
	}
}

void ListViews::SetPicHide(bool flag)
{
	if (flag)
		mpic->hide();
	else
		mpic->show();
}


void ListViews :: ClearNewTaskBox(void)
{
	Folder * f = GetNewTaskBox();
}

void ListViews :: ClearOldTaskBox(void)
{

	Folder * f = GetOldTaskBox();
	
	for(Message * cur = f->firstMessage(); cur; cur=f->nextMessage())
	{
		f->MsgList->takeItem(cur->MsgItem);
		f->delMessage(cur);
	}

}

void ListViews :: ClearTrashBox(void)
{	
	Folder * f = GetTrashTaskBox();
	
	for(Message * cur = f->firstMessage(); cur; cur=f->nextMessage())
	{
		f->MsgList->takeItem(cur->MsgItem);
		f->delMessage(cur);
	}
}

void ListViews :: SaveOldTaskBox(void)
{
	Folder * f = GetTrashTaskBox();
	
	for(Message * cur = f->firstMessage(); cur; cur=f->nextMessage())
	{
		//write Message to OldTask.ini
	}

}

void ListViews :: LoadOldTaskBox(void)
{
		
/**************************************
	Folder * f = GetTrashTaskBox();
	do
	{
		read item from oldtask.ini
		new Message()
	f->addMessage(Msg)
	} while()
****************************************/
		
}

void ListViews :: SaveHistoryTaskList(QString file)
{
	if (file.isEmpty())
		return;

	FILE* fp = fopen(file.latin1(), "w");

	if (!fp) return;
	
	Folder * f = GetTrashTaskBox();
	for(Message * cur = f->firstMessage(); cur; cur=f->nextMessage())
	{
		WriteTaskMsgEx(cur);
	}
	fclose(fp);
}

void ListViews :: ImportTaskList( void )
{
	QString path = QFileDialog::getOpenFileName("/");

	if (path.isEmpty())
		return;

	return ReadRunTaskList(path);
}

void ListViews :: ExportTaskList( void )
{
	QString path = QFileDialog::getSaveFileName("/");

	if (path.isEmpty())
		return;

	qWarning(path);

	return SaveRunTaskList(path);
}

void ListViews :: SaveRunTaskList(QString file)
{
	if (file.isEmpty())
		return;

	FILE* fp = fopen(file.latin1(), "w");

	if (!fp) return;
	
	Folder * f = GetNewTaskBox();
	for(Message * cur = f->firstMessage(); cur; cur=f->nextMessage())
	{
		WriteTaskMsgEx(cur);
	}

	fclose(fp);
}


void ListViews :: WriteTaskMsgEx(Message* TskMsg )
{
	IniFile cfg(INSTALLDIR"config/history.ini");

	string str;

	string TaskName = "TaskItem";
    TaskName += TskMsg->GetTaskId() + '0';

	str = TskMsg->FileName().latin1();
    cfg.write(TaskName, "filename",     str);

	str = TskMsg->FileSize().latin1();
    cfg.write(TaskName, "filesize",     str);

	str = TskMsg->GetAvgSpeed().latin1();
    cfg.write(TaskName, "avgspeed",      str);

	str = TskMsg->GetReceived().latin1();
    cfg.write(TaskName, "received",   str);

	str = TskMsg->GetPercent().latin1();
    cfg.write(TaskName, "percent",     str);

	str = TskMsg->GetTimeLeft().latin1();
    cfg.write(TaskName, "timeleft",    str);

	str = TskMsg->Url().latin1();
    cfg.write(TaskName, "url",  		str);

	str = TskMsg->datetime().toString().latin1();
    cfg.write(TaskName, "datetime",  	str);
}

void ListViews :: ReadHistoryList(void)
{
	FILE* fp = fopen(INSTALLDIR"config/history.inf", "r");

	if (!fp) return;

	int idx = 0;

	while (!feof(fp))
	{
		int flag = 0;
/*
		tk = ReadTaskMsg(fp, &flag);

		if (flag == 1)
		{
			tk.idx = idx++;

			__add_his(tk);
		}
*/
	}

	fclose(fp);
}

void ListViews :: ReadRunTaskList(QString path)
{
	if (path.isEmpty())
		path = INSTALLDIR"config/NewTaskBox.ini";

	FILE* fp = fopen(path.latin1(), "r");

	if (!fp) return;


	while (!feof(fp))
	{
		int flag = 0;
/*
		Message* TskMsg = ReadTaskMsg(fp, &flag);

		if (flag == 1)
		{
			__add_row(tk);
		}
*/
	}
	fclose(fp);

}

