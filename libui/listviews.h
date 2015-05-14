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

#ifndef LISTVIEWS_H
#define LISTVIEWS_H

#include <qsplitter.h>
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qlistbox.h>

#include <stdarg.h>

#include "pic.h"
#include "clistnode.h"
#include "TaskMsgList.h"

class QListView;
class QLabel;
class QPainter;
class QColorGroup;
class QObjectList;
class QPopupMenu;

class Message;
class Folder;

// ----------------------------------------------------------------
class ThreadMsg : public QObject
{
   Q_OBJECT
private:
    QString ThreadId;
    QString Status;
    QString Received;
   	QString AvgSpeed;
	QString Percent;

public:
	enum State { Read = 0, Unread};
	State mstate;
	
public:
	ThreadMsg( const QString & _ThreadId, const QString & _Status,
				const QString & _Received, const QString & _AvgSpeed,
				const QString & _Percent) :	
			ThreadId( _ThreadId ),
			Status( _Status ), 	
			Received( _Received ),
			AvgSpeed(_AvgSpeed), 
			Percent(_Percent), 
			mstate( Unread )
    {
		//Empty
	}
	
	
	void UpdateThreadMsg( 
					const QString & _ThreadId, 
					const QString & _Status,
					const QString & _Received, 
					const QString & _AvgSpeed,
					const QString & _Percent
					)
	{	
		ThreadId	=	_ThreadId;
		Status		= 	_Status;  	
		Received 	= 	_Received;
		AvgSpeed 	=	_AvgSpeed; 
		Percent 	=	_Percent; 
		mstate  	= 	Unread;
	}


	void setState( const State &s ) { mstate = s; }
	State state() { return mstate; }
	
    QString GetThreadId() const  { return ThreadId; }
	void SetThreadId(QString & _ThreadId) { ThreadId = _ThreadId; }
	
	QString GetStatus() const { return Status; }
	void SetStatus(QString & _Status) { Status = _Status; }
	
	QString GetReceived() const	{ return Received; }
	void SetReceived(QString & _Received) { Received = _Received; }
	
	QString GetAvgSpeed() const	{ return AvgSpeed; }
	void SetAvgSpeed(QString & _AvgSpeed) { AvgSpeed = _AvgSpeed; }
	
	QString GetPercent() const { return Percent; }
	void SetPercent(QString & _Percent) { Percent = _Percent; }
	
};

class ThreadMsgListItem : public QListViewItem
{
protected:
    ThreadMsg *myThreadMsg;

public:
    ThreadMsgListItem( QListView *parent, ThreadMsg *m );

    virtual void paintCell( QPainter *p, const QColorGroup &cg, 
								int column,	int width, int alignment );

    ThreadMsg * threadmsg() { return myThreadMsg; }

};

class MessageListItem;
/////////////Task Info ////////////////
class Message : public QObject
{
   Q_OBJECT
public:
    enum State { Read = 0, Unread};
public:
	int 	mTaskId;
    QString mFileName; 
	QString mFileSize;
	QString mStatus;
	QString mUrl;
    QDateTime mdatetime;
	QString mPercent;
	QString mReceived;
	QString mTotalAvgSpeed;
	QString mTimeLeft;
	State mstate;    
	
	QPtrList<ThreadMsg> lstThreadMsgs;
public:
//	QListView 	* ThreadMsgs;
//	QTabWidget  * tabWidget;
	MessageListItem * MsgItem;
	QListView* 	list;
	TLedImpl* 	led;
	MapChart *  map;
public:
    Message( const int _TaskId, const QString & _FileName, 
			const QString & _FileSize, 
			const QString & _TotalAvgSpeed,
			const QString & _Received, 
			const QString & _Percent,
			const QString & _TimeLeft, 
			const QString & _Url, 
			const QDateTime & _datetime) : 
			mTaskId( _TaskId ),
			mFileName( _FileName ), 
			mFileSize( _FileSize ), 			
			mTotalAvgSpeed(_TotalAvgSpeed),
			mReceived( _Received ),
			mPercent(_Percent), 
			mTimeLeft( _TimeLeft ), 
			mUrl( _Url ), 
			mdatetime( _datetime ), 
			mstate( Unread )
    {
//		ThreadMsgs = NULL;
//		tabWidget = NULL;
		lstThreadMsgs.setAutoDelete( true );
		lstThreadMsgs.clear();
		mStatus = "";
		
		MsgItem = NULL;
		list = NULL;
	 	led  = NULL;
		map  = NULL;
	}

    Message( const Message &m )  : 
			mTaskId( m.mTaskId ),
			mFileName( m.mFileName ),
			mFileSize( m.mFileSize ), 
			mStatus( m.mStatus ), 
			mdatetime( m.mdatetime ),
			mUrl( m.mUrl ), 
			mTimeLeft( m.mTimeLeft ), 
			mstate( Unread )
    {
	
	}


    Message &operator=( const Message & mh )
	{
		mTaskId = mh.mTaskId;
		mFileName = mh.mFileName;
		mFileSize = mh.mFileSize;
		mStatus =  mh.mStatus; 
		mdatetime = mh.mdatetime;
		mUrl = mh.mUrl;
		mTimeLeft = mh.mTimeLeft;
		mstate = Unread;
    	return *this;
	}
	
	 void UpdateTaskMsg( 
	 				const int _TaskId, 
	 				const QString & _FileName, 
					const QString & _FileSize, 
					const QString & _TotalAvgSpeed,
					const QString & _Received, 
					const QString & _Percent,
					const QString & _TimeLeft, 
					const QString & _Url, 
					const QDateTime & _datetime
					)  
    {
			mTaskId 		= _TaskId ;
			mFileName 		= _FileName ; 
			mFileSize 		= _FileSize ; 			
			mTotalAvgSpeed 	= _TotalAvgSpeed;
			mReceived 		= _Received ;
			mPercent 		= _Percent; 
			mTimeLeft 		= _TimeLeft; 
			mUrl 			= _Url; 
			mdatetime 		= _datetime; 
			mstate 			=  Unread; 
	}


	int GetTaskId() { return mTaskId; }
	QString FileName() { return mFileName; }
	QString FileSize() { return mFileSize; }
	QDateTime datetime() { return mdatetime; }
    QString Status() { return mStatus; }
	QString Url() {	return mUrl; }
	
	QString GetPercent() { return mPercent; }
    QString GetTimeLeft() { return mTimeLeft; }
	void SetTimeLeft(QString & _TimeLeft)  { mTimeLeft = _TimeLeft; }
	
	QString GetReceived() { return mReceived; }
	void SetReceived(QString & _Received) {  mReceived = _Received; }
	
	void setState( const State &s ) { mstate = s; }
    State state() {	return mstate; }
	
	void SetFileName(QString & _FileName) { mFileName = _FileName; }
	void SetFileSize(QString & _FileSize) { mFileSize = _FileSize; }
	void Setdatetime(QDateTime & _datetime) { mdatetime = _datetime; }
	void SetStatus(QString & _Status) { mStatus = _Status; }
	void SetUrl(QString & _Url) { mUrl = _Url; }

	void SetAvgSpeed(QString& _TotalAvgSpeed) {	mTotalAvgSpeed = _TotalAvgSpeed; }
	QString GetAvgSpeed(void) {	return mTotalAvgSpeed; }

	void addThreadMsg( ThreadMsg *m ) {	lstThreadMsgs.append( m ); }
	ThreadMsg *firstThreadMsg() { return lstThreadMsgs.first(); }
    ThreadMsg *nextThreadMsg() { return lstThreadMsgs.next(); }

};


class MessageListItem : public QListViewItem
{
protected:
    Message *myMessage;
public:
	int 	mTaskId;
public:	
    MessageListItem( QListView *parent, Message *m );

	void UpdateItemContent(Message * m);

    virtual void paintCell( QPainter *p, const QColorGroup &cg, 
								int column,	int width, int alignment );

    Message *message() { return myMessage; }
};

// --------------------------------------------------
class FolderListItem;
class Folder : public QObject
{
    Q_OBJECT

public:
    QString fName;
    QPtrList<Message> lstMessages;

public:
	QListView * MsgList;
	FolderListItem * FolderItem;
public:

    Folder( Folder *parent, const QString &name );
    ~Folder() { }

    void addMessage( Message *m ) { lstMessages.append( m ); }
	void delMessage( Message *m ) { lstMessages.remove( m ); }
    QString folderName() { return fName; }
    Message *firstMessage() { return lstMessages.first(); }
    Message *lastMessage() { return lstMessages.last(); }
 	Message *nextMessage() { return lstMessages.next(); }
  	Message *prevMessage() { return lstMessages.prev(); }
    
	FolderListItem * folderItem() { return FolderItem; }
};

// --------------------------------------------------

class FolderListItem : public QListViewItem
{
protected:
    Folder *myFolder;
	
public:
    FolderListItem( QListView *parent, Folder *f );
    FolderListItem( FolderListItem *parent, Folder *f );

    void insertSubFolders( const QObjectList *lst );

    Folder *folder() { return myFolder; }
};

// -----------------------------------------------------------------

class ListViews : public QSplitter
{
    Q_OBJECT

public:
	QListView * folders;
    QPtrList<Folder> lstFolders;
	QPopupMenu* contextmenu;
public:
    ListViews( QWidget *parent = 0, const char *name = 0 );	
    ~ListViews() {}
    
	void initFolders();
    void initFolder( Folder * folder, unsigned int &count );
	
	void setupFolders();

public:
/////////////////////////////////////////////////////////////
	Folder * GetCurrentFolder();	
	Folder * GetFolderByName(QString& foldername);		
	void RefreshFolder(Folder * item);
////////////////////////////////////////////	
	Folder * GetNewTaskBox(void);
	Folder * GetOldTaskBox(void);
	Folder * GetTrashTaskBox(void);
/////////////////////////////////////////////
	void ClearNewTaskBox(void);
	void ClearOldTaskBox(void);
	void ClearTrashBox(void);

	void SaveOldTaskBox(void);
	void LoadOldTaskBox(void);
	
///////////////////////////////////////////////
	void SaveHistoryTaskList(QString file);
	void ImportTaskList( void );
	void ExportTaskList( void );
	void SaveRunTaskList(QString file);
	void ReadHistoryList(void);
	void ReadRunTaskList(QString path);
	void WriteTaskMsgEx(Message* TskMsg);
////////////////////////////////////////////////////	
	Message * FindTaskMsgById(int taskid);
	
//////////the following all is Container of Widget/////////////	
	CListNode 		* 	listnode;
	CPic 	  		* 	mpic  ;  
	TTaskMsgList 	* 	MsgLists;
	Folder			* 	MyCurFolder;
	Message 		* 	MyCurTaskMsg;
	int					MyCurThreadId;
/////////////////// debug Thread output /////////////////////	

public	slots:
    void slotFolderChanged( QListViewItem* );
	void slotFolderChanged( void );
    void slotMessageChanged();
	void slotMessageChanged( QListViewItem* );
    void slotRMB( QListViewItem*, const QPoint &, int );
	////////////////////for graphic/////////////
	void ListNodeSelectChanged(QListViewItem* i);
public:

//////////////////// for TaskMsgList //////////////
	void AddTaskMsgList(Folder * f);
	void DelTaskMsgList(Folder * f);
	void ShowTaskMsgList(Folder * f);
	void InitTaskMsgList(Folder * f);
	void initFoldersTaskMsgList(void);
//////////////////// for listnode //////////////
	void SetOrigListNodeShow(bool b);
	void AddNewList(int idx);
	void DelList(int idx);
	void ShowList(int idx);
	void ShowListBox(void);
	void InitListNode(int idx, int num);	
///////////////////// for graphic //////////////
	void AddNewGraphic(int idx);
	void DelGraphic(int idx);
	void ShowGraphic(int idx);
//////////////////////////////////////////////////
	void InitLed(int idx, off_t file_total_size, off_t per_size);
	void DrawLed(int idx, off_t offset, off_t length);
	void SetPicHide(bool flag);

};

#endif
