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
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qtable.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qdir.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qslider.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qfileinfo.h>


#define USE_INIFILE_H
#define USE_UTILS_H

#include "matrix.h"
using namespace matrix;


#include "frmAddProxy.h"
#include "frmOptions.h"
#include "frmNewTask.h"

frmOptions::frmOptions( QWidget* parent, const char* name,
											bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Options" );

    btn_ok = new QPushButton( this, "btn_ok" );
    btn_ok->setGeometry( QRect( 340, 330, 70, 23 ) );

    btn_cancel = new QPushButton( this, "btn_cancel" );
    btn_cancel->setGeometry( QRect( 430, 330, 70, 23 ) );

    tabWidget = new QTabWidget( this, "tabWidget" );
    tabWidget->setGeometry( QRect( 10, 10, 490, 310 ) );
    
	
	tab1 = new QWidget( tabWidget, "tab1" );

	
    gb1 = new QGroupBox( tab1, "gb1" );
    gb1->setGeometry( QRect( 10, 10, 470, 120 ) );

	QLabel *lab1 = new QLabel( tr("Download directory"), gb1 );
	lab1->setGeometry( QRect( 10, 20, 100, 20 ) );
    lineEdit_download_dir = new QLineEdit( gb1, "lineEdit_download_dir" );
    lineEdit_download_dir->setGeometry( QRect( 120, 20, 280, 20 ) );

    btn_choose_dir = new QPushButton( gb1, "btn_choose_dir" );
    btn_choose_dir->setGeometry( QRect( 400, 20, 60, 20 ) );
	
	QLabel *lab2 = new QLabel( tr("Language"), gb1 );
    QComboBox * combo1 = new QComboBox( FALSE, gb1 );
    combo1->insertItem( tr("English") );
    combo1->insertItem( tr("Chinese") );
	lab2->setGeometry( QRect( 10, 50, 100, 20 ) );
	combo1->setGeometry( QRect( 120, 50, 300, 20 ) );
	
	connect( combo1, SIGNAL( activated( int ) ), this,
								SLOT( slotLangChanged(int ) ) );

											
	QLabel *lab3 = new QLabel( tr("Style"), gb1 );
    QComboBox * combo2 = new QComboBox( FALSE, gb1 );
   	lab3->setGeometry( QRect( 10, 80, 100, 20 ) );
	combo2->setGeometry( QRect( 120, 80, 300, 20 ) );

	QStringList list = QStyleFactory::keys();
    list.sort();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) 
	{
        QString style = *it;
		combo2->insertItem( style );
    }
	
	connect( combo2, SIGNAL( activated( int ) ), this,
										SLOT( slotStyleChanged(int ) ) );
											

	tabWidget->insertTab( tab1, QString("") );
//////////////////////////////////////////////////////
    tab2 = new QWidget( tabWidget, "tab2" );
    table_proxy = new QTable( tab2, "table_proxy" );
    table_proxy->setGeometry( QRect( 10, 10, 470, 230 ) );
    table_proxy->setNumRows( 3 );
    table_proxy->setNumCols( 3 );

    btn_new = new QPushButton( tab2, "btn_new" );
    btn_new->setGeometry( QRect( 10, 250, 70, 23 ) );

    btn_edit = new QPushButton( tab2, "btn_edit" );
    btn_edit->setGeometry( QRect( 90, 250, 70, 23 ) );

    btn_del = new QPushButton( tab2, "btn_del" );
    btn_del->setGeometry( QRect( 170, 250, 70, 23 ) );

    btn_verify = new QPushButton( tab2, "btn_verify" );
    btn_verify->setGeometry( QRect( 250, 250, 70, 23 ) );

    btn_import = new QPushButton( tab2, "btn_import" );
    btn_import->setGeometry( QRect( 330, 250, 70, 23 ) );

    btn_export = new QPushButton( tab2, "btn_export" );
    btn_export->setGeometry( QRect( 410, 250, 70, 23 ) );
    tabWidget->insertTab( tab2, QString("") );

	///////////////////////////////////////////////////
    tab3 = new QWidget( tabWidget, "tab3" );

    checkBox_use_default = new QCheckBox( tab3, "checkBox_use_default" );
    checkBox_use_default->setGeometry( QRect( 10, 10, 140, 20 ) );

	gb2 = new QGroupBox( tab3, "gb1" );
    gb2->setGeometry( QRect( 10, 30, 470, 100 ) );

    txt_max_task_num = new QLabel( gb2, "txt_max_task_num" );
    txt_max_task_num->setGeometry( QRect( 10, 10, 100, 20 ) );
    spinBox_task_num = new QSpinBox( gb2, "spinBox_task_num" );
    spinBox_task_num->setGeometry( QRect( 120, 10, 50, 20 ) );
	
    txt_max_thread_num = new QLabel( gb2, "txt_max_thread_num" );
    txt_max_thread_num->setGeometry( QRect( 200, 10, 100, 20 ) );
    spinBox_thread_num = new QSpinBox( gb2, "spinBox_thread_num" );
    spinBox_thread_num->setGeometry( QRect( 310, 10, 50, 20 ) );

    txt_retry_times = new QLabel( gb2, "txt_retry_times" );
    txt_retry_times->setGeometry( QRect( 10, 40, 100, 20 ) );
    spinBox_retry_times = new QSpinBox( gb2, "spinBox_retry_times" );
    spinBox_retry_times->setGeometry( QRect( 120, 40, 50, 20 ) );

    txt_retry_interval = new QLabel( gb2, "txt_retry_interval" );
    txt_retry_interval->setGeometry( QRect( 200, 40, 100, 20 ) );
    spinBox_retry_interval = new QSpinBox( gb2, "spinBox_retry_interval" );
    spinBox_retry_interval->setGeometry( QRect( 310, 40, 50, 20 ) );
    txt_retry_sec = new QLabel( gb2, "txt_retry_sec" );
    txt_retry_sec->setGeometry( QRect( 370, 40, 40, 20 ) );
	
	txt_max_redir = new QLabel( gb2, "txt_max_redir" );
    txt_max_redir->setGeometry( QRect( 10, 70, 100, 20 ) );
    spinBox_max_redir = new QSpinBox( gb2, "spinBox_max_redir" );
    spinBox_max_redir->setGeometry( QRect( 120, 70, 50, 20 ) );

    txt_timeout_wait = new QLabel( gb2, "txt_timeout_wait" );
    txt_timeout_wait->setGeometry( QRect( 200, 70, 100, 20 ) );
    spinBox_timeout_wait = new QSpinBox( gb2, "spinBox_timeout_wait" );
    spinBox_timeout_wait->setGeometry( QRect( 310, 70, 50, 20 ) );
    txt_timeout_sec = new QLabel( gb2, "txt_timeout_sec" );
    txt_timeout_sec->setGeometry( QRect( 370, 70, 40, 20 ) );

	gb3 = new QGroupBox( tab3, "gb3" );
    gb3->setGeometry( QRect( 10, 140, 470, 100 ) );
	
	txt_min_block_size = new QLabel( gb3, "txt_min_block_size" );
    txt_min_block_size->setGeometry( QRect( 10, 10, 100, 20 ) );
    spinBox_min_block_size = new QSpinBox( gb3, "spinBox_min_block_size" );
    spinBox_min_block_size->setGeometry( QRect( 140, 10, 50, 20 ) );
    txt_min_KB = new QLabel( gb3, "txt_min_KB" );
    txt_min_KB->setGeometry( QRect( 200, 10, 40, 20 ) );

    txt_socket_buffer = new QLabel( gb3, "txt_socket_buffer" );
    txt_socket_buffer->setGeometry( QRect( 10, 40, 100, 20 ) );
    spinBox_socket_buffer = new QSpinBox( gb3, "spinBox_socket_buffer" );
    spinBox_socket_buffer->setGeometry( QRect( 140, 40, 50, 20 ) );
    txt_socket_KB = new QLabel( gb3, "txt_socket_KB" );
    txt_socket_KB->setGeometry( QRect( 200, 40, 40, 20 ) );

    checkBox_sync = new QCheckBox( gb3, "checkBox_sync" );
    checkBox_sync->setGeometry( QRect( 10, 70, 100, 20 ) );

    spinBox_sync_size = new QSpinBox( gb3, "spinBox_sync_size" );
    spinBox_sync_size->setGeometry( QRect( 140, 70, 50, 20 ) );

    txt_sync_KB = new QLabel( gb3, "txt_sync_KB" );
    txt_sync_KB->setGeometry( QRect( 200, 70, 40, 20 ) );
    tabWidget->insertTab( tab3, QString("") );

    languageChange();
    resize( QSize(512, 360).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( lineEdit_download_dir, btn_choose_dir );
    setTabOrder( btn_choose_dir, table_proxy );
    setTabOrder( table_proxy, btn_new );
    setTabOrder( btn_new, btn_edit );
    setTabOrder( btn_edit, btn_del );
    setTabOrder( btn_del, btn_verify );
    setTabOrder( btn_verify, btn_import );
    setTabOrder( btn_import, btn_export );
    setTabOrder( btn_export, checkBox_use_default );
    setTabOrder( checkBox_use_default, spinBox_task_num );
    setTabOrder( spinBox_task_num, spinBox_min_block_size );
    setTabOrder( spinBox_min_block_size, spinBox_retry_times );
    setTabOrder( spinBox_retry_times, spinBox_retry_interval );
    setTabOrder( spinBox_retry_interval, spinBox_timeout_wait );
    setTabOrder( spinBox_timeout_wait, spinBox_socket_buffer );
    setTabOrder( spinBox_socket_buffer, spinBox_sync_size );
	
    setTabOrder( spinBox_sync_size, checkBox_sync );
    setTabOrder( btn_ok, btn_cancel );
    setTabOrder( btn_cancel, tabWidget );

    setFixedSize(QSize(512, 360));
///////////////////////////////////////////////////////////	
	init_proxy_table();
///////////////////////////////////////////////////////////

	connect(btn_choose_dir, SIGNAL(clicked()), this, SLOT(slt_btn_choose_dir()));
	connect(btn_del, SIGNAL(clicked()), this, SLOT(slt_btn_del_proxy()));
	connect(btn_new, SIGNAL(clicked()), this, SLOT(slt_btn_add_proxy()));
	connect(btn_edit, SIGNAL(clicked()), this, SLOT(slt_btn_edit_proxy()));

	connect(btn_import, SIGNAL(clicked()), this, SLOT(slt_btn_import_proxy()));
	connect(btn_export, SIGNAL(clicked()), this, SLOT(slt_btn_export_proxy()));

	connect(btn_ok, SIGNAL(clicked()), this, SLOT(slt_btn_ok()));
	connect(btn_cancel, SIGNAL(clicked()), this, SLOT(slt_btn_cancel()));

	connect(checkBox_use_default, SIGNAL(clicked()), this,
										 SLOT(slt_checkBox_use_default()));
										 
	connect(checkBox_sync, SIGNAL(clicked()), this, SLOT(slt_checkBox_sync()));

	checkBox_use_default->setChecked(true);
	slt_checkBox_use_default();
	
	init_param();

}

frmOptions::~frmOptions()
{
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void frmOptions::languageChange()
{
    setCaption( tr( "Options / Setting" ) );
    btn_ok->setText( tr( "OK" ) );
    btn_cancel->setText( tr( "Cancel" ) );
    gb1->setTitle( tr( "" ) );
    btn_choose_dir->setText( tr( "..." ) );
    tabWidget->changeTab( tab1, tr( "General" ) );
	
    btn_new->setText( tr( "New" ) );
    btn_edit->setText( tr( "Edit" ) );
    btn_del->setText( tr( "Delete" ) );
    btn_verify->setText( tr( "Verify" ) );
    btn_import->setText( tr( "Import" ) );
    btn_export->setText( tr( "Export" ) );
    tabWidget->changeTab( tab2, tr( "Proxy" ) );
	
    checkBox_use_default->setText( tr( "Use default" ) );
    txt_max_task_num->setText( tr( "Maximized task number" ) );
	txt_max_thread_num->setText( tr( "Maximized thread number" ) );
    txt_min_block_size->setText( tr( "Minimized file block size" ) );
    txt_retry_times->setText( tr( "Retry times" ) );
    txt_retry_interval->setText( tr( "Retry interval" ) );
	txt_max_redir->setText(tr("Max Redirection") );
    txt_timeout_wait->setText( tr( "Timeout wait" ) );
    txt_socket_buffer->setText( tr( "Receive buffer size" ) );
    checkBox_sync->setText( tr( "Sync to disk when receive" ) );
    txt_timeout_sec->setText( tr( "(Sec)" ) );
    txt_retry_sec->setText( tr( "(Sec)" ) );
    txt_min_KB->setText( tr( "(KB)" ) );
    txt_socket_KB->setText( tr( "(KB)" ) );
    txt_sync_KB->setText( tr( "(KB)" ) );
    tabWidget->changeTab( tab3, tr( "Performance" ) );
	
}


void frmOptions::init_param()
{
	QString path = "";
	if (path.isEmpty())
	{
		path = getenv("HOME");
		path += "/download";
	}

	lineEdit_download_dir->setText(path);

    IniFile cfg(INSTALLDIR"config/wdget.ini");

	int def_max_task_num = 5;

   	if( !cfg.read("global", "max_task_num").empty() )
		def_max_task_num = atoi( cfg.read("global", "max_task_num").c_str() );

	int def_min_blk_size = 32;

   	if( !cfg.read("global", "min_blk_size").empty() )
		def_min_blk_size = atoi( cfg.read("global", "min_blk_size").c_str() );

	int def_retry_times = 100;

   	if( !cfg.read("global", "retry_times").empty() )
		def_retry_times = atoi( cfg.read("global", "retry_times").c_str() );

	int def_retry_interval = 15;

   	if( !cfg.read("global", "retry_interval").empty() )
		def_retry_interval  =atoi( cfg.read("global", "retry_interval").c_str() );

	int def_timeout_wait = 120;

   	if( !cfg.read("global", "timeout_wait").empty() )
		def_timeout_wait = atoi( cfg.read("global", "timeout_wait").c_str() );

	int def_receive_buffer_size = 4;

   	if( !cfg.read("global", "recv_buf_size").empty() )
		def_receive_buffer_size = atoi( cfg.read("global", "recv_buf_size").c_str() );

	int def_sync_buffer_size = 256;

   	if( !cfg.read("global", "sync_cache_size").empty() )
		def_sync_buffer_size = atoi( cfg.read("global", "sync_cache_size").c_str() );

	spinBox_task_num->setMinValue(1);
	spinBox_task_num->setMaxValue(10);
	spinBox_task_num->setValue(def_max_task_num);
	
	spinBox_thread_num->setMinValue(1);
	spinBox_thread_num->setMaxValue(6);
	spinBox_thread_num->setValue(4);

     
	spinBox_min_block_size->setMinValue(32);
	spinBox_min_block_size->setMaxValue(1024);
	spinBox_min_block_size->setValue(def_min_blk_size);

	spinBox_retry_times->setMinValue(1);
	spinBox_retry_times->setMaxValue(30);
	spinBox_retry_times->setValue(def_retry_times);

	spinBox_retry_interval->setMinValue(1);
	spinBox_retry_interval->setMaxValue(30);
	spinBox_retry_interval->setValue(def_retry_interval);

	spinBox_timeout_wait->setMinValue(1);
	spinBox_timeout_wait->setMaxValue(30);
	spinBox_timeout_wait->setValue(def_timeout_wait);
	

	spinBox_max_redir->setMinValue(1);
	spinBox_max_redir->setMaxValue(10);
	spinBox_max_redir->setValue(1);

	spinBox_socket_buffer->setMinValue(4);
	spinBox_socket_buffer->setMaxValue(128);
	spinBox_socket_buffer->setValue(def_receive_buffer_size);

	spinBox_sync_size->setMinValue(256);
	spinBox_sync_size->setMaxValue(1024);
	spinBox_sync_size->setValue(def_sync_buffer_size);

	if (def_max_task_num != 5 || def_min_blk_size != 32 ||
		def_retry_times != 10 || def_retry_interval != 5 ||
		def_timeout_wait != 5 || def_receive_buffer_size != 4 ||
		def_sync_buffer_size != 0)
	{
		checkBox_use_default->setChecked(false);
		slt_checkBox_use_default();
		if (def_sync_buffer_size != 0)
		{
			checkBox_sync->setChecked(true);
			slt_checkBox_sync();
		}
	}
}

void frmOptions::slt_btn_choose_dir()
{
	QString path = lineEdit_download_dir->text();
	path = QFileDialog::getExistingDirectory(path);
	lineEdit_download_dir->setText(path);
}

void frmOptions::slt_checkBox_sync()
{
	if (checkBox_sync->isChecked())
	{
		spinBox_sync_size->setEnabled(true);
	}
	else
	{
		spinBox_sync_size->setEnabled(false);
	}
}

void frmOptions::slt_checkBox_use_default()
{
	if (checkBox_use_default->isChecked())
	{
		max_task_num = 3;
		min_blk_size = 32;
		retry_times = 10;
		retry_interval = 5;
		timeout_wait = 5;
		recv_buf_size = 4;
		sync_cache_size = 0;		// means: not use


		spinBox_task_num->setValue(3);
		spinBox_min_block_size->setValue(32);
		spinBox_retry_times->setValue(10);
		spinBox_retry_interval->setValue(5);
		spinBox_timeout_wait->setValue(5);
		spinBox_socket_buffer->setValue(4);
		spinBox_sync_size->setValue(512);	// just to show
	

		set_enabled(false);
	}
	else
	{
		set_enabled(true);
		if (!checkBox_sync->isChecked())
			spinBox_sync_size->setEnabled(false);
	}
}

void frmOptions::set_enabled(bool b)
{
	txt_max_task_num->setEnabled(b);
	txt_min_block_size->setEnabled(b);
	txt_retry_times->setEnabled(b);
	txt_retry_interval->setEnabled(b);
	txt_timeout_sec->setEnabled(b);
	txt_socket_KB->setEnabled(b);
	txt_sync_KB->setEnabled(b);
	txt_timeout_wait->setEnabled(b);	
	txt_socket_buffer->setEnabled(b);
	txt_retry_sec->setEnabled(b);
	txt_min_KB->setEnabled(b);
	txt_sync_KB->setEnabled(b);
	
	spinBox_task_num->setEnabled(b);
	spinBox_min_block_size->setEnabled(b);
	spinBox_retry_times->setEnabled(b);
	spinBox_retry_interval->setEnabled(b);
	spinBox_timeout_wait->setEnabled(b);
	spinBox_socket_buffer->setEnabled(b);
	spinBox_sync_size->setEnabled(b);
	checkBox_sync->setEnabled(b);

}

void frmOptions::slt_btn_cancel()
{
	done(0);
}

void frmOptions::slt_btn_ok()
{
	engine_param_apply();
	proxy_apply();
	done(1);
}


void frmOptions::slt_btn_del_proxy()
{
	if (!check_select())
		return;

	int idx = table_proxy->currentRow();
	table_proxy->removeRow(idx);
}

void frmOptions::slt_btn_add_proxy()
{
	Proxy_t proxy;
	frmAddNewProxy ad;
	
	int res = ad.exec();

	if (res == 0)
	{
		return;
	}

	int n = table_proxy->numRows();

	if(n >= 4)
	{
        QMessageBox::warning(this, tr("Info"), tr("max proxy num is 4"));
		return;
	}

	table_proxy->setNumRows(n + 1);

	table_proxy->setText(n, 0, ad.name);
	proxy.name = ad.name.latin1();

	if (ad.type == 0)
		table_proxy->setText(n, 1, "direct");
	else if (ad.type == 1)
		table_proxy->setText(n, 1, "http");
	else if (ad.type == 2)
		table_proxy->setText(n, 1, "ftp");
	else if (ad.type == 3)
		table_proxy->setText(n, 1, "sock4");
	else if (ad.type == 4)
		table_proxy->setText(n, 1, "sock5");

	proxy.type = (ProxyType)ad.type;	

	table_proxy->setText(n, 2, ad.server);

	proxy.host = ad.server.latin1();	

	char port[32];
	memset(port, 0, sizeof(port));
	sprintf(port, "%d", ad.port);
	
	table_proxy->setText(n, 3, port);

	proxy.port = ad.port;	//proxy port

	if (ad.auth)
	{
		table_proxy->setText(n, 4, "Yes");
		table_proxy->setText(n, 5, ad.user);
		table_proxy->setText(n, 6, ad.pwd);

		proxy.auth = true;
		proxy.user = ad.user.latin1();
		proxy.pass = ad.pwd.latin1();
	}
	else
	{
		proxy.auth = false;
		table_proxy->setText(n, 4, "No");
	}

}


void frmOptions::slt_btn_edit_proxy()
{
	if (!check_select())
		return;

	Proxy_t proxy;
	int idx = table_proxy->currentRow();

	QString name = table_proxy->text(idx, 0);
	QString type = table_proxy->text(idx, 1);
	QString server = table_proxy->text(idx, 2);
	QString port = table_proxy->text(idx, 3);
	QString auth = table_proxy->text(idx, 4);
	QString user = table_proxy->text(idx, 5);
	QString pwd = table_proxy->text(idx, 6);

	int ntype = 0;
	int nport = atoi(port.latin1());

	if (type == "http")
		ntype = 1;
	else if (type == "ftp")
		ntype = 2;
	else if (type == "sock4")
		ntype = 3;
	else if (type == "sock5")
		ntype = 4;
	
	frmAddNewProxy ad(name, server, user, pwd, nport, ntype);

	int res = ad.exec();
	if (res == 0)
	{
		return;
	}

	table_proxy->setText(idx, 0, ad.name);
	proxy.name = ad.name.latin1();

	if (ad.type == 0)
		table_proxy->setText(idx, 1, "direct");
	else if (ad.type == 1)
		table_proxy->setText(idx, 1, "http");
	else if (ad.type == 2)
		table_proxy->setText(idx, 1, "ftp");
	else if (ad.type == 3)
		table_proxy->setText(idx, 1, "sock4");
	else if (ad.type == 4)
		table_proxy->setText(idx, 1, "sock5");

	proxy.type = (ProxyType) ad.type;

	table_proxy->setText(idx, 2, ad.server);
	proxy.host = ad.server.latin1();

	char sport[32];
	memset(sport, 0, sizeof(sport));
	sprintf(sport, "%d", ad.port);
	
	table_proxy->setText(idx, 3, sport);
	proxy.port = ad.port;

	if (ad.auth)
	{
		table_proxy->setText(idx, 4, "Yes");
		table_proxy->setText(idx, 5, ad.user);
		table_proxy->setText(idx, 6, ad.pwd);

		proxy.auth = true;
		proxy.user = ad.user.latin1();
		proxy.pass = ad.pwd.latin1();
	}
	else
	{
		proxy.auth = false;
		table_proxy->setText(idx, 4, "No");
	}

}

void frmOptions::slt_btn_import_proxy()
{
	QString path = QFileDialog::getOpenFileName("/");

	if (path.isEmpty())
	{
		return;
	}

	refresh_proxy_table();

}

void frmOptions::slt_btn_export_proxy()
{
	QString path = QFileDialog::getSaveFileName("/");

	if (path.isEmpty())
	{
		return;
	}

	write_proxy_inf(path.latin1());
}

void frmOptions::proxy_apply()
{
	write_proxy_inf(INSTALLDIR"config/wdget.ini");
}

void frmOptions::write_engine_inf(void)
{
	IniFile cfg(INSTALLDIR"config/wdget.ini");
	cfg.write("global", "max_task_num", 	max_task_num);
	cfg.write("global", "min_blk_size", 	min_blk_size);
	cfg.write("global", "retry_times", 		retry_times);
	cfg.write("global", "retry_interval", 	retry_interval);
	cfg.write("global", "timeout_wait", 	timeout_wait);
	cfg.write("global", "recv_buf_size", 	recv_buf_size);
	cfg.write("global", "sync_cache_size", 	sync_cache_size);

	//download_dir
	QString _path = lineEdit_download_dir->text();
	if (access(_path.latin1(), F_OK) < 0)
	{
		int res = QMessageBox::question(this, tr("Question"), 
				tr("The directory not exist,\n Do you want create it?"),
				tr("Yes"), tr("No"));

		if (res == 0)
		{
			QString cmd = "mkdir -p ";
			cmd += _path;
			system(cmd.latin1());
		}
	}

	string download_path = _path.latin1();
	cfg.write("global", "download_dir_path", download_path );

}

void frmOptions::write_proxy_inf(const char* path)
{
	IniFile cfg(path);

	int nRows = table_proxy->numRows();
	for (int i = 0; i < nRows; ++i)
	{

		QString name = table_proxy->text(i, 0);
		string proxyname = name.latin1();
		
		QString type = table_proxy->text(i, 1);
		string proxytype = type.latin1();
		cfg.write(proxyname, "type", proxytype );
	

		QString host = table_proxy->text(i, 2);
		string proxyhost = host.latin1();
		cfg.write(proxyname, "host", proxyhost );

		QString port = table_proxy->text(i, 3);
		string proxyport = port.latin1();
		cfg.write(proxyname, "port", proxyport );
	
		QString auth = table_proxy->text(i, 4);
		if (auth == "Yes")
		{
			cfg.write(proxyname, "auth", true);

			QString user = table_proxy->text(i, 5);
			if( !user.isEmpty() )
			{
				string proxyuser = user.latin1();
				cfg.write(proxyname, "user", proxyuser );
			}
			else
			{
				cfg.write(proxyname, "user", string("") );
			}

			QString pwd = table_proxy->text(i, 6);
			if( !pwd.isEmpty() )
			{
				string proxypwd = pwd.latin1();
				cfg.write(proxyname, "pass", proxypwd );
			}
			else
			{
				cfg.write(proxyname, "pass", string("") );
			}
		}
		else
		{
			cfg.write(proxyname, "auth", false);

			QString user = table_proxy->text(i, 5);
			if( !user.isEmpty() )
			{
				string proxyuser = user.latin1();
				cfg.write(proxyname, "user", proxyuser );
			}
			else
			{
				cfg.write(proxyname, "user", string("") );
			}
		
			QString pwd = table_proxy->text(i, 6);
			if( !pwd.isEmpty() )
			{
				string proxypwd = pwd.latin1();
				cfg.write(proxyname, "pass", proxypwd );
			}
			else
			{
				cfg.write(proxyname, "pass", string("") );
			}

		}

	}

}


bool frmOptions::check_select()
{
	int idx = table_proxy->currentRow();

	if (idx < 0)
	{
		QMessageBox::warning(this, tr("Info"), tr("Please select one"));
		return false;
	}

	return true;
}

void frmOptions::init_proxy_table()
{
	table_proxy->verticalHeader()->hide();
    table_proxy->setLeftMargin(0);
    table_proxy->setSelectionMode(QTable::SingleRow);
    table_proxy->setReadOnly(true);
    table_proxy->setShowGrid(false);
    table_proxy->setFocusPolicy(QWidget::NoFocus);

    table_proxy->setNumCols( 0 );
    table_proxy->setNumRows( 0 );

    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("ProxyName") );
    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("Type") );
    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("Server") );
    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("Port") );
    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("Auth") );
    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("User") );
    table_proxy->setNumCols( table_proxy->numCols() + 1 );
    table_proxy->horizontalHeader()->setLabel( table_proxy->numCols() - 1, tr("Password") );

	table_proxy->hideColumn (5);
	table_proxy->hideColumn (6);

	refresh_proxy_table();
}


void frmOptions::refresh_proxy_table()
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

		int n = table_proxy->numRows();
		table_proxy->setNumRows(n + 1);
		table_proxy->setText(n, 0, it.name);

		if ( it.type == PROXY_DIRECT )
			table_proxy->setText(n, 1, "direct");
		else if ( it.type == PROXY_HTTP )
			table_proxy->setText(n, 1, "http");
		else if ( it.type == PROXY_FTP )
			table_proxy->setText(n, 1, "ftp");
		else if ( it.type == PROXY_SOCK4 )
			table_proxy->setText(n, 1, "sock4");
		else if ( it.type == PROXY_SOCK5 )
			table_proxy->setText(n, 1, "sock5");

		table_proxy->setText(n, 2, it.host);

		char port[32];
		memset(port, 0, sizeof(port));
		sprintf(port, "%d", it.port);
		table_proxy->setText(n, 3, port);

		if(it.user.empty())
		{
			table_proxy->setText(n, 4, "No");
		}
		else
		{
			table_proxy->setText(n, 4, "Yes");
			table_proxy->setText(n, 5, it.user);
			table_proxy->setText(n, 6, it.pass);
		}

	}
}

void frmOptions::engine_param_apply()
{
	if (!checkBox_use_default->isChecked())
	{
		max_task_num = spinBox_task_num->value();
		min_blk_size = spinBox_min_block_size->value();
		retry_times = spinBox_retry_times->value();
		retry_interval = spinBox_retry_interval->value();
		timeout_wait = spinBox_timeout_wait->value();
		recv_buf_size = spinBox_socket_buffer->value();

		if (checkBox_sync->isChecked())
		{
			sync_cache_size = spinBox_sync_size->value();
		}
		else
		{
			sync_cache_size = 0;
		}

	}
	else
	{
		max_task_num = 3;
		min_blk_size = 32;
		retry_times = 10;
		retry_interval = 5;
		timeout_wait = 5;
		recv_buf_size = 4;
		sync_cache_size = 0;
	}

	write_engine_inf();
}

void frmOptions::slotLangChanged( int i )
{
	extern QTranslator ts;
    QString base = QDir(INSTALLDIR"lang/").absPath();	
	QString qmfile;
	switch (i) 
	{
    case 0: qmfile = "dget.qm"; break;
    case 1: qmfile = "dget_zh_CN.qm"; break;
    }

    qApp->removeTranslator(&ts);
    ts.load(qmfile, base);
    qApp->installTranslator(&ts);
}

void frmOptions::slotStyleChanged( int i )
{    
	QString style;	
	
	switch (i) 
	{
    case 0: style = "CDE"; break;
    case 1: style = "Motif"; break;
    case 2: style = "MotifPlus"; break;
    case 3: style = "Platinum"; break;
    case 4: style = "SGI"; break;
    case 5: style = "Windows"; break;
    }
    
	QStyle * s = QStyleFactory::create( style );
    if ( s ) QApplication::setStyle( s );
}

