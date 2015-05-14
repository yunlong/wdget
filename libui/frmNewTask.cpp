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

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtable.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qclipboard.h>
#include <qurl.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qdragobject.h>

#include <assert.h>

#define USE_INIFILE_H
#include "matrix.h"
using namespace matrix;

#include "frmNewTask.h"
#include "frmAddMirror.h"

frmNewTask::frmNewTask(QString url, QString reffer, NewTaskParam& ntp, bool bload, QWidget* parent, 
			const char* name, bool modal, WFlags fl) : QDialog(parent, name, modal, fl)
{
    if ( !name )
		setName( "NewTask" );

    btn_ok = new QPushButton( this, "btn_ok" );
    btn_ok->setGeometry( QRect( 383, 441, 70, 23 ) );

    btn_cancel = new QPushButton( this, "btn_cancel" );
    btn_cancel->setGeometry( QRect( 470, 441, 70, 23 ) );

    tabWidget = new QTabWidget( this, "tabWidget" );
    tabWidget->setGeometry( QRect( 10, 10, 530, 420 ) );

    tab = new QWidget( tabWidget, "tab" );

    txt_reffer = new QLabel( tab, "txt_reffer" );
    txt_reffer->setGeometry( QRect( 20, 51, 70, 20 ) );

    txt_url = new QLabel( tab, "txt_url" );
    txt_url->setGeometry( QRect( 20, 21, 70, 20 ) );

    txt_saveto = new QLabel( tab, "txt_saveto" );
    txt_saveto->setGeometry( QRect( 20, 81, 70, 20 ) );

    txt_kind = new QLabel( tab, "txt_kind" );
    txt_kind->setGeometry( QRect( 20, 120, 70, 20 ) );

    txt_rename = new QLabel( tab, "txt_rename" );
    txt_rename->setGeometry( QRect( 20, 150, 70, 20 ) );

    txt_blocks = new QLabel( tab, "txt_blocks" );
    txt_blocks->setGeometry( QRect( 20, 179, 68, 20 ) );

    checkBox_auth = new QCheckBox( tab, "checkBox_auth" );
    checkBox_auth->setGeometry( QRect( 20, 210, 100, 20 ) );

    gb_task = new QGroupBox( tab, "gb_task" );
    gb_task->setGeometry( QRect( 20, 290, 240, 90 ) );

    radioButton_auto = new QRadioButton( gb_task, "radioButton_auto" );
    radioButton_auto->setGeometry( QRect( 10, 20, 120, 20 ) );

    radioButton_manual = new QRadioButton( gb_task, "radioButton_manual" );
    radioButton_manual->setGeometry( QRect( 10, 40, 130, 20 ) );

    radioButton_plan = new QRadioButton( gb_task, "radioButton_plan" );
    radioButton_plan->setGeometry( QRect( 10, 60, 94, 20 ) );

    lineEdit_reffer = new QLineEdit( tab, "lineEdit_reffer" );
    lineEdit_reffer->setGeometry( QRect( 99, 51, 410, 20 ) );

    lineEdit_saveto = new QLineEdit( tab, "lineEdit_saveto" );
    lineEdit_saveto->setGeometry( QRect( 100, 80, 350, 20 ) );

    lineEdit_rename = new QLineEdit( tab, "lineEdit_rename" );
    lineEdit_rename->setGeometry( QRect( 100, 149, 150, 20 ) );

    btn_choose = new QPushButton( tab, "btn_choose" );
    btn_choose->setGeometry( QRect( 460, 80, 50, 23 ) );

    gb_explain = new QGroupBox( tab, "gb_explain" );
    gb_explain->setGeometry( QRect( 270, 110, 240, 270 ) );

    textEdit = new QTextEdit( gb_explain, "textEdit" );
    textEdit->setGeometry( QRect( 10, 20, 220, 240 ) );

    lineEdit_url = new QLineEdit( tab, "lineEdit_url" );
    lineEdit_url->setGeometry( QRect( 99, 21, 410, 20 ) );

    comboBox_kind = new QComboBox( FALSE, tab, "comboBox_kind" );
    comboBox_kind->setGeometry( QRect( 100, 120, 150, 20 ) );

    spinBox = new QSpinBox( tab, "spinBox" );
    spinBox->setGeometry( QRect( 100, 178, 150, 20 ) );

    txt_user = new QLabel( tab, "txt_user" );
    txt_user->setGeometry( QRect( 20, 234, 60, 20 ) );

    lineEdit_user = new QLineEdit( tab, "lineEdit_user" );
    lineEdit_user->setGeometry( QRect( 100, 234, 106, 20 ) );

    txt_pwd = new QLabel( tab, "txt_pwd" );
    txt_pwd->setGeometry( QRect( 20, 261, 60, 20 ) );

    lineEdit_pwd = new QLineEdit( tab, "lineEdit_pwd" );
    lineEdit_pwd->setGeometry( QRect( 100, 261, 106, 20 ) );
    tabWidget->insertTab( tab, QString("") );

    TabPage = new QWidget( tabWidget, "TabPage" );

    checkBox_use_mirror = new QCheckBox( TabPage, "checkBox_use_mirror" );
    checkBox_use_mirror->setGeometry( QRect( 10, 20, 230, 20 ) );

    txt_explain = new QLabel( TabPage, "txt_explain" );
    txt_explain->setGeometry( QRect( 10, 50, 380, 20 ) );

    btn_auto_scan = new QPushButton( TabPage, "btn_auto_scan" );
    btn_auto_scan->setGeometry( QRect( 430, 80, 80, 30 ) );

    btn_del_mirror = new QPushButton( TabPage, "btn_del_mirror" );
    btn_del_mirror->setGeometry( QRect( 430, 120, 80, 30 ) );

    btn_add_mirror = new QPushButton( TabPage, "btn_add_mirror" );
    btn_add_mirror->setGeometry( QRect( 430, 160, 80, 30 ) );

    btn_clear_all = new QPushButton( TabPage, "btn_clear_all" );
    btn_clear_all->setGeometry( QRect( 430, 200, 80, 30 ) );

    btn_verify_mirror = new QPushButton( TabPage, "btn_verify_mirror" );
    btn_verify_mirror->setGeometry( QRect( 430, 240, 80, 30 ) );

    table_mirror = new QTable( TabPage, "table_mirror" );
    table_mirror->setGeometry( QRect( 10, 80, 400, 150 ) );
    table_mirror->setNumRows( 3 );
    table_mirror->setNumCols( 2 );

	buttonGroup1 = new QButtonGroup( TabPage, "buttonGroup1" );
    buttonGroup1->setGeometry( QRect( 10, 230, 400, 150 ) );
	
	http_kind = new QLabel( buttonGroup1, "http_kind" );
    http_kind->setGeometry( QRect( 40, 20, 70, 20 ) );
	comboBox_HTTP = new QComboBox( FALSE, buttonGroup1, "comboBox_HTTP" );
    comboBox_HTTP->setGeometry( QRect( 150, 20, 200, 20 ) );
	
	ftp_kind = new QLabel( buttonGroup1, "ftp_kind" );
    ftp_kind->setGeometry( QRect( 40, 50, 70, 20 ) );
    comboBox_FTP = new QComboBox( FALSE, buttonGroup1, "comboBox_FTP" );
    comboBox_FTP->setGeometry( QRect( 150, 50, 200, 20 ) );
	
	mms_kind = new QLabel( buttonGroup1, "mms_kind" );
    mms_kind->setGeometry( QRect( 40, 80, 70, 20 ) );
    comboBox_MMS = new QComboBox( FALSE, buttonGroup1, "comboBox_MMS" );
    comboBox_MMS->setGeometry( QRect( 150, 80, 200, 20 ) );
	
	rtsp_kind = new QLabel( buttonGroup1, "rtsp_kind" );
    rtsp_kind->setGeometry( QRect( 40, 110, 70, 20 ) );
    comboBox_RTSP = new QComboBox( FALSE, buttonGroup1, "comboBox_RTSP" );
    comboBox_RTSP->setGeometry( QRect( 150, 110, 200, 20 ) );



    tabWidget->insertTab( TabPage, QString("") );
	
    languageChange();
    resize( QSize(550, 472).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( tabWidget, lineEdit_url );
    setTabOrder( lineEdit_url, lineEdit_reffer );
    setTabOrder( lineEdit_reffer, lineEdit_saveto );
    setTabOrder( lineEdit_saveto, btn_choose );
    setTabOrder( btn_choose, comboBox_kind );
    setTabOrder( comboBox_kind, lineEdit_rename );
    setTabOrder( lineEdit_rename, spinBox );
    setTabOrder( spinBox, checkBox_auth );
    setTabOrder( checkBox_auth, lineEdit_user );
    setTabOrder( lineEdit_user, lineEdit_pwd );
    setTabOrder( lineEdit_pwd, textEdit );
    setTabOrder( textEdit, radioButton_auto );
    setTabOrder( radioButton_auto, radioButton_manual );
    setTabOrder( radioButton_manual, radioButton_plan );
    setTabOrder( radioButton_plan, checkBox_use_mirror );
    setTabOrder( checkBox_use_mirror, table_mirror );
    setTabOrder( table_mirror, btn_auto_scan );
    setTabOrder( btn_auto_scan, btn_del_mirror );
    setTabOrder( btn_del_mirror, btn_add_mirror );
    setTabOrder( btn_add_mirror, btn_clear_all );
    setTabOrder( btn_clear_all, btn_verify_mirror );

    setTabOrder( btn_ok, btn_cancel );
	
    setFixedSize(QSize(550,472));

    Init();


	lineEdit_url->setText(url);
	lineEdit_reffer->setText(reffer);

	QString file = getFile(url);
	lineEdit_rename->setText(file);


	QString path = "" ;

	if (path.isEmpty())
    {
        path = getenv("HOME");
        path += "/download";
    }

	lineEdit_saveto->setText(path);

	if (bload == true)
	{
		restore(ntp);
	}

	refreshProxyInfo();

    connect(btn_choose, SIGNAL(clicked()), this, SLOT(slt_btn_choose()));
    connect(btn_ok, SIGNAL(clicked()), this, SLOT(slt_btn_ok()));
    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(slt_btn_cancel()));
   
    connect(radioButton_auto, SIGNAL(clicked()), this, SLOT(slt_radiobtn_auto()));
    connect(radioButton_manual, SIGNAL(clicked()), this, SLOT(slt_radiobtn_manual()));
    connect(radioButton_plan, SIGNAL(clicked()), this, SLOT(slt_radiobtn_plan()));

    connect(checkBox_auth, SIGNAL(clicked()), this, SLOT(slt_checkbox_auth()));
    connect(checkBox_use_mirror, SIGNAL(clicked()), this,
									 SLOT(slt_checkbox_use_mirror()));


    connect(btn_auto_scan, SIGNAL(clicked()), this, SLOT(slt_btn_autoscan()));
    connect(btn_add_mirror, SIGNAL(clicked()), this, SLOT(slt_btn_addmirror()));
    connect(btn_del_mirror, SIGNAL(clicked()), this, SLOT(slt_btn_delmirror()));
    connect(btn_verify_mirror, SIGNAL(clicked()), this, SLOT(slt_btn_verify()));
    connect(btn_clear_all, SIGNAL(clicked()), this, SLOT(slt_clear_all()));

    connect(tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(slt_current_changed(QWidget*)));

	connect(lineEdit_url, SIGNAL(textChanged(const QString&)), this, SLOT(slt_url_changed(const QString&)));

	btn_verify_mirror->hide();
}

void frmNewTask::refreshProxyInfo(void)
{
	IniFile cfg(INSTALLDIR"config/wdget.ini");

	for (int i = 0; i < 4; ++i)
	{   
		Proxy_t it; 
		string proxyname = "proxy";
		proxyname += i + '0';

		it.name = proxyname;
		string proxytype = cfg.read(proxyname, "type");

		if( proxytype.empty() ) 
			break;

		if(proxytype == "http")
		{
			comboBox_HTTP->insertItem( it.name.c_str() );  
		}

		if(proxytype == "ftp")
		{
			comboBox_FTP->insertItem( it.name.c_str() );
		}

		if(proxytype == "sock4" || proxytype == "sock5")
		{
			comboBox_MMS->insertItem( it.name.c_str() );
			comboBox_RTSP->insertItem( it.name.c_str() );
		}
	}   
}

/*
 *  Destroys the object and frees any allocated resources
 */
frmNewTask::~frmNewTask()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
 
void frmNewTask::languageChange()
{
    setCaption( tr( "CreateNewTask" ) );
    btn_ok->setText( tr( "OK" ) );
    btn_cancel->setText( tr( "Cancel" ) );
    txt_reffer->setText( tr( "Reference" ) );
    txt_url->setText( tr( "Target URL" ) );
    txt_saveto->setText( tr( "Save to" ) );
    txt_kind->setText( tr( "Category" ) );
    txt_rename->setText( tr( "File name" ) );
    txt_blocks->setText( tr( "Split blocks" ) );
    checkBox_auth->setText( tr( "Authentication" ) );
    gb_task->setTitle( tr( "Task Execute" ) );
    radioButton_auto->setText( tr( "Auto" ) );
    radioButton_manual->setText( tr( "Manual" ) );
    radioButton_plan->setText( tr( "Plan" ) );
    btn_choose->setText( tr( "..." ) );
    gb_explain->setTitle( tr( "Comment" ) );
	
    comboBox_kind->clear();
    comboBox_kind->insertItem( tr( "Soft" ) );
    comboBox_kind->insertItem( tr( "Game" ) );
    comboBox_kind->insertItem( tr( "Music" ) );
    comboBox_kind->insertItem( tr( "Movie" ) );
	
    txt_user->setText( tr( "Username" ) );
    txt_pwd->setText( tr( "Password" ) );
    tabWidget->changeTab( tab, tr( "General" ) );
    checkBox_use_mirror->setText( tr( "Use Mirror URL" ) );
    txt_explain->setText( tr( "You can also drag the target url into this box directly" ) );
    btn_auto_scan->setText( tr( "Auto scan" ) );
    btn_del_mirror->setText( tr( "Delete" ) );
    btn_add_mirror->setText( tr( "Add" ) );
    btn_clear_all->setText( tr( "Clear All" ) );
    btn_verify_mirror->setText( tr( "Verify" ) );
	
	buttonGroup1->setTitle( tr("Proxy") );
	http_kind->setText( tr( "HTTP" ) ); 
	ftp_kind->setText( tr( "FTP" ) ); 
	mms_kind->setText( tr( "MMS" ) );
	rtsp_kind->setText( tr( "RTSP" ) );
	comboBox_HTTP->insertItem( tr( "http direct" ) ); 
    comboBox_FTP->insertItem( tr( "ftp direct" ) ); 
    comboBox_MMS->insertItem( tr( "mms direct" ) ); 
	comboBox_RTSP->insertItem( tr( "rtsp direct" ) ); 

    tabWidget->changeTab( TabPage, tr( "Advanced" ) );
   
}

void frmNewTask::restore(NewTaskParam& ntp)
{
	//tab 1
	lineEdit_saveto->setText(ntp.path);
	lineEdit_rename->setText(ntp.file);
	textEdit->setText(ntp.comm);
	spinBox->setValue(ntp.nblks);

	if (ntp.kind == 0)
		comboBox_kind->setCurrentText(tr("Soft"));
	else if (ntp.kind == 1)
		comboBox_kind->setCurrentText(tr("Game"));
	else if (ntp.kind == 2)
		comboBox_kind->setCurrentText(tr("Music"));
	else if (ntp.kind == 3)
		comboBox_kind->setCurrentText(tr("Movie"));

	if (!(ntp.user.empty()))
	{
		checkBox_auth->setChecked(true);
		slt_checkbox_auth();
		lineEdit_user->setText(ntp.user.c_str());
		lineEdit_pwd->setText(ntp.pwd.c_str());
	}
	if ( ntp.type == TSK_MANNUAL_EXEC )
	{
		slt_radiobtn_manual();
	}

	//tab2
	if (ntp.vSurl.size() > 1)
	{
		checkBox_use_mirror->setChecked(true);
		slt_checkbox_use_mirror();

		for (unsigned int i = 1; i < ntp.vSurl.size(); ++i)
			add_mirror_url_item(ntp.vSurl[i], true);
	}
}


void frmNewTask::slt_set_caption(QString str)
{
	setCaption(str);
}

void frmNewTask::slt_btn_choose()
{
	QString path = lineEdit_saveto->text();

	path = QFileDialog::getExistingDirectory(path);

	if (!path.isEmpty())
		lineEdit_saveto->setText(path);

}

void frmNewTask::slt_current_changed(QWidget*)
{

}

string frmNewTask :: getProxyName( QString sUrl)
{
	IniFile cfg(INSTALLDIR"config/wdget.ini");

   	if ( sUrl.find("http://") >= 0 || sUrl.find("https://") >= 0 )
	{
		return string( comboBox_HTTP->currentText().latin1() );
	}
 	
	if(sUrl.find("ftp://") >= 0)
	{
		return string( comboBox_FTP->currentText().latin1() ); 
	}
	
	if(sUrl.find("mms://") >= 0)
	{
		return string( comboBox_MMS->currentText().latin1() ); 
	}
	
	if(sUrl.find("rtsp://") >= 0)
	{
		return string( comboBox_RTSP->currentText().latin1() ) ; 
	}

	return string("");
}

void frmNewTask::slt_btn_ok()
{
	m_ntp.origUrl  = "";
	m_ntp.reffer	= "";
	m_ntp.path		= "";
	m_ntp.file		= "";
	m_ntp.nblks		= 1;
	m_ntp.comm		= "";
	m_ntp.user		= "";
	m_ntp.pwd		= "";
	m_ntp.kind		= FILE_TYPE_SOFT;
	m_ntp.proxyname = "";
	m_ntp.vSurl.clear();

	QString url = lineEdit_url->text();
	m_ntp.origUrl = lineEdit_url->text().latin1();
	m_ntp.reffer = lineEdit_reffer->text().latin1();
	m_ntp.path = lineEdit_saveto->text().latin1();
	m_ntp.file = lineEdit_rename->text().latin1();
	m_ntp.nblks = spinBox->value();
	m_ntp.comm = textEdit->text().latin1();

	if (url.isEmpty() || m_ntp.file.empty())
	{
		QMessageBox::warning(this, tr("warning"), tr("please check your input!"));
		return;
	}

	m_ntp.kind = (FileType)comboBox_kind->currentItem();

	if (checkBox_auth->isChecked())
	{
		m_ntp.user = lineEdit_user->text().latin1();
		m_ntp.pwd = lineEdit_pwd->text().latin1();
	}

	m_ntp.proxyname = getProxyName(url);
	///////////////////////////
	m_ntp.vSurl.push_back(url);

	QClipboard *cb = QApplication::clipboard();
	cb->setText(url, QClipboard::Clipboard);


	if (checkBox_use_mirror->isChecked() && spinBox->value() > 1)
	{
		int nRows = table_mirror->numRows();

		for (int i = 0; i < nRows; ++i)
		{
			QCheckBox* chk = (QCheckBox *) table_mirror->cellWidget(i, 1);

			if (chk->isChecked())
			{
				QString _url = table_mirror->text(i, 0);
				m_ntp.vSurl.push_back(_url);
			}
		}
	}


	if (radioButton_auto->isChecked())
		m_ntp.type = TSK_AUTO_EXEC;
	else if (radioButton_manual->isChecked())
		m_ntp.type = TSK_MANNUAL_EXEC;
	else if (radioButton_plan->isChecked())
		m_ntp.type = TSK_PLAN_EXEC;

	done(1);
}


void frmNewTask::slt_btn_cancel()
{
	done(0);
}

void frmNewTask::slt_radiobtn_auto()
{
	radioButton_auto->setChecked(true);
	radioButton_manual->setChecked(false);
	radioButton_plan->setChecked(false);
}

void frmNewTask::slt_radiobtn_manual()
{
	radioButton_manual->setChecked(true);
	radioButton_auto->setChecked(false);
	radioButton_plan->setChecked(false);
}

void frmNewTask::slt_radiobtn_plan()
{
	radioButton_plan->setChecked(true);
	radioButton_auto->setChecked(false);
	radioButton_manual->setChecked(false);
}

void frmNewTask::slt_checkbox_use_mirror()
{
	if (spinBox->value() <= 1)
	{
		if (checkBox_use_mirror->isChecked())
		{
			QMessageBox::warning(this, tr("Warning"), 
					tr("You've specify one thread to download the target!"));
		}

		setAcceptDrops(false);
	    txt_explain->setEnabled(false);
	    table_mirror->setEnabled(false);
	    btn_auto_scan->setEnabled(false);
	    btn_del_mirror->setEnabled(false);
	    btn_add_mirror->setEnabled(false);
	    btn_verify_mirror->setEnabled(false);
		btn_clear_all->setEnabled(false);
		checkBox_use_mirror->setChecked(false);
		return;
	}

	if (checkBox_use_mirror->isChecked())
	{
		setAcceptDrops(true);
	    txt_explain->setEnabled(true);
	    table_mirror->setEnabled(true);
	    btn_auto_scan->setEnabled(true);
	    btn_del_mirror->setEnabled(true);
	    btn_add_mirror->setEnabled(true);
	    btn_verify_mirror->setEnabled(true);
		btn_clear_all->setEnabled(true);

	}
	else
	{
		setAcceptDrops(false);
	    txt_explain->setEnabled(false);
	    table_mirror->setEnabled(false);
	    btn_auto_scan->setEnabled(false);
	    btn_del_mirror->setEnabled(false);
	    btn_add_mirror->setEnabled(false);
	    btn_verify_mirror->setEnabled(false);
		btn_clear_all->setEnabled(false);
	}
}

void frmNewTask::slt_checkbox_auth()
{
	if (checkBox_auth->isChecked())
	{
		txt_user->setEnabled(true);
		txt_pwd->setEnabled(true);
		lineEdit_user->setEnabled(true);
		lineEdit_pwd->setEnabled(true);
	}
	else
	{
		txt_user->setEnabled(false);
		txt_pwd->setEnabled(false);
		lineEdit_user->setEnabled(false);
		lineEdit_pwd->setEnabled(false);

		lineEdit_user->setText("");
		lineEdit_pwd->setText("");
	}
}

void frmNewTask::Init()    
{
	spinBox->setMaxValue(6);
	spinBox->setMinValue(1);
	spinBox->setValue(4);
	
    //General
    radioButton_auto->setChecked(true);
    txt_user->setEnabled(false);
    txt_pwd->setEnabled(false);
    lineEdit_user->setEnabled(false);
    lineEdit_pwd->setEnabled(false);
	lineEdit_pwd->setEchoMode(QLineEdit::Password);

    //Advance
    txt_explain->setEnabled(false);
    table_mirror->setEnabled(false);
    btn_auto_scan->setEnabled(false);
    btn_del_mirror->setEnabled(false);
    btn_add_mirror->setEnabled(false);
    btn_verify_mirror->setEnabled(false);
    btn_clear_all->setEnabled(false);
	
    init_mirror_table();
}


void frmNewTask::init_mirror_table()
{	
    table_mirror->verticalHeader()->hide();
    table_mirror->setLeftMargin(0);
    table_mirror->setSelectionMode(QTable::SingleRow);
    table_mirror->setReadOnly(true);
    table_mirror->setShowGrid(false);
    table_mirror->setFocusPolicy(QWidget::NoFocus);

    table_mirror->setNumCols( 0 );
    table_mirror->setNumRows( 0 );

    table_mirror->setNumCols( table_mirror->numCols() + 1 );
    table_mirror->horizontalHeader()->setLabel( table_mirror->numCols() - 1, tr("Mirror URL") );
    table_mirror->setNumCols( table_mirror->numCols() + 1 );
    table_mirror->horizontalHeader()->setLabel( table_mirror->numCols() - 1, tr("Choose [y/n]") );

    table_mirror->horizontalHeader()->resizeSection(0, 250);
    table_mirror->horizontalHeader()->resizeSection(1, 150);
}

void frmNewTask::mousePressEvent( QMouseEvent *e )
{
    clickPos = e->pos();
}

void frmNewTask::mouseMoveEvent( QMouseEvent *e )
{
    move( e->globalPos() - clickPos );
}

void frmNewTask::dragEnterEvent( QDragEnterEvent *e )
{
    e->accept(QUriDrag::canDecode(e) || QTextDrag::canDecode(e));
}

void frmNewTask::dropEvent( QDropEvent * e )
{
	if (!checkBox_use_mirror->isChecked())
		return;

    QStrList list;
    QString str;

    if ( QUriDrag::decode( e, list ) )
    {
        //for (const char* u=list.first(); u; u=list.next())
        str = list.first();
    }
    else
    {
        str = QString(e->encodedData("text/plain"));
    }

    int nRows = table_mirror->numRows();
    table_mirror->setNumRows(nRows + 1);
    table_mirror->setText(nRows, 0, str);

	QColor c(255, 255, 255);
	QCheckBox* chkbox = new QCheckBox(table_mirror, "chkbox");
	chkbox->setPaletteBackgroundColor(c);
	table_mirror->setCellWidget (nRows, 1, chkbox);
}


void frmNewTask::slt_clear_all()
{
    int nRows = table_mirror->numRows();
    for (int i = nRows; i >= 0; --i)
		table_mirror->removeRow (i - 1);
}

void frmNewTask::get_current_html_page()
{
	QMessageBox::warning(this, tr("Warning"), tr("Not find any mirror urls"));

	QString reffer = lineEdit_reffer->text();

	if (reffer.isEmpty())
		return;
}

void frmNewTask::slt_url_changed(const QString& new_url)
{
	QString url = new_url;
	lineEdit_url->setText(url);
	QString file = getFile(url);
	lineEdit_rename->setText(file);
}

void frmNewTask::slt_btn_autoscan()
{
	slt_clear_all();
	get_current_html_page();
}

void frmNewTask::add_mirror_url_item(QString url, bool b)
{
	int nRows = table_mirror->numRows();
	table_mirror->setNumRows(nRows + 1);
	table_mirror->setText(nRows, 0, url);

	QColor c(255, 255, 255);
	QCheckBox* chkbox = new QCheckBox(table_mirror, "chkbox");
	chkbox->setPaletteBackgroundColor(c);
	chkbox->setChecked(b);
	table_mirror->setCellWidget (nRows, 1, chkbox);
}

void frmNewTask::slt_btn_addmirror()
{
	frmAddMirror  am;
	int res = am.exec();

	if (res == 0)
		return;

	QString url = am.url;

	if (!url.isEmpty())
		add_mirror_url_item(url);
}

void frmNewTask::slt_btn_delmirror()
{
    int idx = table_mirror->currentRow ();
    if (idx >= 0)
        table_mirror->removeRow(idx);
}

void frmNewTask::slt_btn_verify()
{
	qWarning("to Verify the proxy server!");
}


QString frmNewTask::getFile(QString url)
{
	if (url.isEmpty())
		return "";

	int idx = url.findRev('/');
	if (idx < 0)
		return "";
	return url.mid(idx + 1);
}

