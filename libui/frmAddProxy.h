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

#ifndef FRMADDNEWPROXY_H
#define FRMADDNEWPROXY_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QPushButton;

class frmAddNewProxy : public QDialog
{
    Q_OBJECT
public:
    frmAddNewProxy( QWidget* parent = 0, const char* name = 0, 
									bool modal = FALSE, WFlags fl = 0 );
    
	frmAddNewProxy(QString na, QString srv, QString us, 
						QString pw, int po, int ty, 
						QWidget *parent = 0, const char *name = 0, 
						bool modal = FALSE, WFlags fl = 0);
    ~frmAddNewProxy();

    QGroupBox* groupBox20;
    QLabel* txt_name;
    QLabel* txt_server;
    QLineEdit* lineEdit_name;
    QLineEdit* lineEdit_server;
    QLabel* txt_port;
    QSpinBox* spinBox_port;
    QGroupBox* gb_type;
    QRadioButton* radiobtn_socks5;
    QRadioButton* radiobtn_socks4;
    QRadioButton* radiobtn_http;
    QRadioButton* radiobtn_ftp;
    QRadioButton* radiobtn_direct;
    QGroupBox* gb_auth;
    QLabel* txt_user;
    QLabel* txt_pwd;
    QLineEdit* lineEdit_user;
    QLineEdit* lineEdit_pwd;
    QCheckBox* checkBox_auth;
    QPushButton* btn_cancel;
    QPushButton* btn_ok;

protected slots:
    virtual void languageChange();

///////////////////////class impl ////////////////////
private:
	void init_table();
public:

	QString name;
	QString server;
	QString user;
	QString pwd;
	int type;
	int port;
	bool auth;

public slots:
	void slt_btn_cancel();
	void slt_btn_ok();
	void slt_radiobtn_direct();
	void slt_radiobtn_http();
	void slt_radiobtn_ftp();
	void slt_radiobtn_socks4();
	void slt_radiobtn_socks5();
	void slt_chk_auth();
};

#endif
