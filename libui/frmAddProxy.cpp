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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>

#include "frmAddProxy.h"

frmAddNewProxy::frmAddNewProxy( QWidget* parent, const char* name, 
												bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AddNewProxy" );

    groupBox20 = new QGroupBox( this, "groupBox20" );
    groupBox20->setGeometry( QRect( 10, 10, 360, 270 ) );

    txt_name = new QLabel( groupBox20, "txt_name" );
    txt_name->setGeometry( QRect( 10, 30, 60, 20 ) );

    txt_server = new QLabel( groupBox20, "txt_server" );
    txt_server->setGeometry( QRect( 10, 60, 60, 20 ) );

    lineEdit_name = new QLineEdit( groupBox20, "lineEdit_name" );
    lineEdit_name->setGeometry( QRect( 70, 31, 270, 20 ) );

    lineEdit_server = new QLineEdit( groupBox20, "lineEdit_server" );
    lineEdit_server->setGeometry( QRect( 70, 60, 150, 20 ) );

    txt_port = new QLabel( groupBox20, "txt_port" );
    txt_port->setGeometry( QRect( 230, 60, 30, 20 ) );

    spinBox_port = new QSpinBox( groupBox20, "spinBox_port" );
    spinBox_port->setGeometry( QRect( 270, 59, 70, 21 ) );

    gb_type = new QGroupBox( groupBox20, "gb_type" );
    gb_type->setGeometry( QRect( 190, 90, 150, 170 ) );

    radiobtn_direct = new QRadioButton( gb_type, "radiobtn_direct" );
    radiobtn_direct->setGeometry( QRect( 10, 20, 94, 20 ) );
    
	radiobtn_http = new QRadioButton( gb_type, "radiobtn_http" );
    radiobtn_http->setGeometry( QRect( 10, 50, 120, 20 ) );

    radiobtn_ftp = new QRadioButton( gb_type, "radiobtn_ftp" );
    radiobtn_ftp->setGeometry( QRect( 10, 80, 120, 20 ) );
    
	radiobtn_socks4 = new QRadioButton( gb_type, "radiobtn_socks4" );
    radiobtn_socks4->setGeometry( QRect( 10, 110, 120, 20 ) );

    radiobtn_socks5 = new QRadioButton( gb_type, "radiobtn_socks5" );
    radiobtn_socks5->setGeometry( QRect( 10, 140, 120, 20 ) );

    gb_auth = new QGroupBox( groupBox20, "gb_auth" );
    gb_auth->setGeometry( QRect( 10, 100, 170, 160 ) );

    txt_user = new QLabel( gb_auth, "txt_user" );
    txt_user->setGeometry( QRect( 10, 20, 40, 20 ) );

    txt_pwd = new QLabel( gb_auth, "txt_pwd" );
    txt_pwd->setGeometry( QRect( 10, 50, 50, 20 ) );

    lineEdit_user = new QLineEdit( gb_auth, "lineEdit_user" );
    lineEdit_user->setGeometry( QRect( 60, 20, 100, 20 ) );

    lineEdit_pwd = new QLineEdit( gb_auth, "lineEdit_pwd" );
    lineEdit_pwd->setGeometry( QRect( 60, 50, 100, 20 ) );

    checkBox_auth = new QCheckBox( groupBox20, "checkBox_auth" );
    checkBox_auth->setGeometry( QRect( 20, 90, 110, 20 ) );

    btn_cancel = new QPushButton( this, "btn_cancel" );
    btn_cancel->setGeometry( QRect( 280, 290, 70, 23 ) );

    btn_ok = new QPushButton( this, "btn_ok" );
    btn_ok->setGeometry( QRect( 200, 290, 70, 23 ) );
    languageChange();
    resize( QSize(360, 322).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( lineEdit_name, lineEdit_server );
    setTabOrder( lineEdit_server, radiobtn_http );
    setTabOrder( radiobtn_http, radiobtn_ftp );
    setTabOrder( radiobtn_ftp, radiobtn_socks4 );
    setTabOrder( radiobtn_socks4, radiobtn_socks5 );
    setTabOrder( radiobtn_socks5, checkBox_auth );
    setTabOrder( checkBox_auth, lineEdit_user );
    setTabOrder( lineEdit_user, lineEdit_pwd );
    setTabOrder( lineEdit_pwd, btn_cancel );
    setTabOrder( btn_cancel, btn_ok );


	setFixedSize(QSize(400, 322));
	connect(btn_ok, SIGNAL(clicked()), this, SLOT(slt_btn_ok()));
	connect(btn_cancel, SIGNAL(clicked()), this, SLOT(slt_btn_cancel()));
	connect(radiobtn_direct, SIGNAL(clicked()), this, SLOT(slt_radiobtn_direct()));
	connect(radiobtn_http, SIGNAL(clicked()), this, SLOT(slt_radiobtn_http()));
	connect(radiobtn_ftp, SIGNAL(clicked()), this, SLOT(slt_radiobtn_ftp()));
	connect(radiobtn_socks4, SIGNAL(clicked()), this, SLOT(slt_radiobtn_socks4()));
	connect(radiobtn_socks5, SIGNAL(clicked()), this, SLOT(slt_radiobtn_socks5()));
	connect(checkBox_auth, SIGNAL(clicked()), this, SLOT(slt_chk_auth()));
	init_table();
}

/*
 *  Destroys the object and frees any allocated resources
 */
frmAddNewProxy::~frmAddNewProxy()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void frmAddNewProxy::languageChange()
{
    setCaption( tr( "Add New Proxy" ) );
    groupBox20->setTitle( tr( "Add New Proxy" ) );
    txt_name->setText( tr( "ProxyName" ) );
    txt_server->setText( tr( "Server" ) );
    txt_port->setText( tr( "Port" ) );
    gb_type->setTitle( tr( "Proxy Type" ) );
    radiobtn_socks5->setText( tr( "Sock5Proxy" ) );
    radiobtn_socks4->setText( tr( "Sock4Proxy" ) );
    radiobtn_http->setText( tr( "HttpProxy" ) );
    radiobtn_ftp->setText( tr( "FtpProxy" ) );
    radiobtn_direct->setText( tr( "Direct" ) );
    gb_auth->setTitle( QString::null );
    txt_user->setText( tr( "User" ) );
    txt_pwd->setText( tr( "Passwd" ) );
    checkBox_auth->setText( tr( "Authentication" ) );
    btn_cancel->setText( tr( "Cancel" ) );
    btn_ok->setText( tr( "OK" ) );
}


frmAddNewProxy::frmAddNewProxy(QString na, QString srv, QString us, 
						QString pw, int po, int ty, QWidget *parent, 
						const char *name, bool modal, WFlags fl) 
	: QDialog(parent, name, modal, fl)
{    if ( !name )
	setName( "AddNewProxy" );

    groupBox20 = new QGroupBox( this, "groupBox20" );
    groupBox20->setGeometry( QRect( 10, 10, 360, 270 ) );

    txt_name = new QLabel( groupBox20, "txt_name" );
    txt_name->setGeometry( QRect( 10, 30, 60, 20 ) );

    txt_server = new QLabel( groupBox20, "txt_server" );
    txt_server->setGeometry( QRect( 10, 60, 60, 20 ) );

    lineEdit_name = new QLineEdit( groupBox20, "lineEdit_name" );
    lineEdit_name->setGeometry( QRect( 70, 31, 270, 20 ) );

    lineEdit_server = new QLineEdit( groupBox20, "lineEdit_server" );
    lineEdit_server->setGeometry( QRect( 70, 60, 150, 20 ) );

    txt_port = new QLabel( groupBox20, "txt_port" );
    txt_port->setGeometry( QRect( 230, 60, 30, 20 ) );

    spinBox_port = new QSpinBox( groupBox20, "spinBox_port" );
    spinBox_port->setGeometry( QRect( 270, 59, 70, 21 ) );

    gb_type = new QGroupBox( groupBox20, "gb_type" );
    gb_type->setGeometry( QRect( 190, 90, 150, 170 ) );

    radiobtn_direct = new QRadioButton( gb_type, "radiobtn_direct" );
    radiobtn_direct->setGeometry( QRect( 10, 20, 94, 20 ) );
    
	radiobtn_http = new QRadioButton( gb_type, "radiobtn_http" );
    radiobtn_http->setGeometry( QRect( 10, 50, 120, 20 ) );

    radiobtn_ftp = new QRadioButton( gb_type, "radiobtn_ftp" );
    radiobtn_ftp->setGeometry( QRect( 10, 80, 120, 20 ) );
    
	radiobtn_socks4 = new QRadioButton( gb_type, "radiobtn_socks4" );
    radiobtn_socks4->setGeometry( QRect( 10, 110, 120, 20 ) );

    radiobtn_socks5 = new QRadioButton( gb_type, "radiobtn_socks5" );
    radiobtn_socks5->setGeometry( QRect( 10, 140, 120, 20 ) );


    gb_auth = new QGroupBox( groupBox20, "gb_auth" );
    gb_auth->setGeometry( QRect( 10, 100, 170, 160 ) );

    txt_user = new QLabel( gb_auth, "txt_user" );
    txt_user->setGeometry( QRect( 10, 20, 40, 20 ) );

    txt_pwd = new QLabel( gb_auth, "txt_pwd" );
    txt_pwd->setGeometry( QRect( 10, 50, 50, 20 ) );

    lineEdit_user = new QLineEdit( gb_auth, "lineEdit_user" );
    lineEdit_user->setGeometry( QRect( 60, 20, 100, 20 ) );

    lineEdit_pwd = new QLineEdit( gb_auth, "lineEdit_pwd" );
    lineEdit_pwd->setGeometry( QRect( 60, 50, 100, 20 ) );

    checkBox_auth = new QCheckBox( groupBox20, "checkBox_auth" );
    checkBox_auth->setGeometry( QRect( 20, 90, 110, 20 ) );

    btn_cancel = new QPushButton( this, "btn_cancel" );
    btn_cancel->setGeometry( QRect( 280, 290, 70, 23 ) );

    btn_ok = new QPushButton( this, "btn_ok" );
    btn_ok->setGeometry( QRect( 200, 290, 70, 23 ) );
    languageChange();
    resize( QSize(360, 322).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( lineEdit_name, lineEdit_server );
    setTabOrder( lineEdit_server, radiobtn_http );
    setTabOrder( radiobtn_http, radiobtn_ftp );
    setTabOrder( radiobtn_ftp, radiobtn_socks4 );
    setTabOrder( radiobtn_socks4, radiobtn_socks5 );
    setTabOrder( radiobtn_socks5, checkBox_auth );
    setTabOrder( checkBox_auth, lineEdit_user );
    setTabOrder( lineEdit_user, lineEdit_pwd );
    setTabOrder( lineEdit_pwd, btn_cancel );
    setTabOrder( btn_cancel, btn_ok );

	setFixedSize(QSize(400, 322));
	connect(btn_ok, SIGNAL(clicked()), this, SLOT(slt_btn_ok()));
	connect(btn_cancel, SIGNAL(clicked()), this, SLOT(slt_btn_cancel()));
	connect(radiobtn_direct, SIGNAL(clicked()), this, SLOT(slt_radiobtn_direct()));
	connect(radiobtn_http, SIGNAL(clicked()), this, SLOT(slt_radiobtn_http()));
	connect(radiobtn_ftp, SIGNAL(clicked()), this, SLOT(slt_radiobtn_ftp()));
	connect(radiobtn_socks4, SIGNAL(clicked()), this, SLOT(slt_radiobtn_socks4()));
	connect(radiobtn_socks5, SIGNAL(clicked()), this, SLOT(slt_radiobtn_socks5()));	
	connect(checkBox_auth, SIGNAL(clicked()), this, SLOT(slt_chk_auth()));

	init_table();

	name	= na;
	server	= srv;
	user	= us;
	pwd		= pw;
	port	= po;
	type	= ty;

	lineEdit_name->setText(name);
	lineEdit_server->setText(server);

	spinBox_port->setValue(port);

	if (type == 0)
		slt_radiobtn_direct();
	else if (type == 1)
		slt_radiobtn_http();
	else if (type == 2)
		slt_radiobtn_ftp();
	else if (type == 3)
		slt_radiobtn_socks4();
	else if (type == 4)
		slt_radiobtn_socks5();

	if (!user.isEmpty())
	{
		auth = true;
		lineEdit_user->setText(user);
		lineEdit_pwd->setText(pwd);

		checkBox_auth->setChecked(true);
		slt_chk_auth();
	}
	else
	{
		auth = false;
	}
}

void frmAddNewProxy :: init_table(void)
{
	slt_radiobtn_direct();
	lineEdit_user->setEnabled(false);
	lineEdit_pwd->setEnabled(false);
	txt_user->setEnabled(false);
	txt_pwd->setEnabled(false);

	lineEdit_pwd->setEchoMode(QLineEdit::Password);

	spinBox_port->setMaxValue(60000);
	spinBox_port->setValue(8080);

	auth = false;	
	type = 0;		
}

void frmAddNewProxy::slt_chk_auth()
{
	if (checkBox_auth->isChecked())
	{
		lineEdit_user->setEnabled(true);
		lineEdit_pwd->setEnabled(true);
		txt_user->setEnabled(true);
		txt_pwd->setEnabled(true);
		auth = true;
	}
	else
	{
		lineEdit_user->setEnabled(false);
		lineEdit_pwd->setEnabled(false);
		txt_user->setEnabled(false);
		txt_pwd->setEnabled(false);
		auth = false;
	}
}

void frmAddNewProxy::slt_btn_cancel()
{
	done(0);
}

void frmAddNewProxy::slt_btn_ok()
{
	name = lineEdit_name->text();
	server = lineEdit_server->text();
	port = spinBox_port->value();


	if (name.isEmpty() || server.isEmpty() || !port)
	{
		QMessageBox::warning(this, tr("Warning"), tr("please check your input..."));
		return;
	}


	if (checkBox_auth->isChecked())
	{
		user = lineEdit_user->text();
		pwd = lineEdit_pwd->text();

		if (user.isEmpty())
		{
			QMessageBox::warning(this, tr("Warning"), tr("user account is empty, please check it!"));
			return;
		}
		if (pwd.isEmpty())
		{
			QMessageBox::warning(this, tr("Warning"), tr("password is empty, please check it!"));
			return;
		}
	}

	done(1);
}

void frmAddNewProxy::slt_radiobtn_direct()
{
	radiobtn_direct->setChecked(true);

	radiobtn_http->setChecked(false);
	radiobtn_ftp->setChecked(false);
	radiobtn_socks4->setChecked(false);
	radiobtn_socks5->setChecked(false);
	type = 0;

}

void frmAddNewProxy::slt_radiobtn_http()
{
	radiobtn_http->setChecked(true);

	radiobtn_direct->setChecked(false);
	radiobtn_ftp->setChecked(false);
	radiobtn_socks4->setChecked(false);
	radiobtn_socks5->setChecked(false);
	type = 1;
}

void frmAddNewProxy::slt_radiobtn_ftp()
{
	radiobtn_ftp->setChecked(true);

	radiobtn_direct->setChecked(false);
	radiobtn_http->setChecked(false);
	radiobtn_socks4->setChecked(false);
	radiobtn_socks5->setChecked(false);

	type = 2;
}

void frmAddNewProxy::slt_radiobtn_socks4()
{
	radiobtn_socks4->setChecked(true);

	radiobtn_direct->setChecked(false);
	radiobtn_http->setChecked(false);
	radiobtn_ftp->setChecked(false);
	radiobtn_socks5->setChecked(false);

	checkBox_auth->setEnabled(false);
	lineEdit_user->setEnabled(false);
	lineEdit_pwd->setEnabled(false);

	type = 3;
}

void frmAddNewProxy::slt_radiobtn_socks5()
{
	radiobtn_socks5->setChecked(true);

	radiobtn_direct->setChecked(false);
	radiobtn_http->setChecked(false);
	radiobtn_ftp->setChecked(false);
	radiobtn_socks4->setChecked(false);

	type = 4;
}

