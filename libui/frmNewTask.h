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

#ifndef FRMNEWTASK_H
#define FRMNEWTASK_H

#include <qvariant.h>
#include <qdialog.h>
#include <qfiledialog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qevent.h>

#include <string>
#include <vector>
#include <iostream>

#include "matrix.h"
using namespace matrix;

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QTabWidget;
class QWidget;
class QLabel;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QSpinBox;
class QTable;
class QButtonGroup;

class frmNewTask : public QDialog
{
    Q_OBJECT

public:
    frmNewTask(QString url, QString reffer, NewTaskParam& ntp, 
				bool bload = false,	QWidget* parent = 0, 
				const char* name = 0, bool modal = FALSE, 
				WFlags fl = 0);

    ~frmNewTask();

    QPushButton* btn_ok;
    QPushButton* btn_cancel;
    QTabWidget* tabWidget;
    QWidget* tab;
	
    QLabel* txt_reffer;
    QLabel* txt_url;
    QLabel* txt_saveto;
    QLabel* txt_kind;
    QLabel* txt_rename;
    QLabel* txt_blocks;
	
    QCheckBox* checkBox_auth;
    QGroupBox* gb_task;
    QRadioButton* radioButton_auto;
    QRadioButton* radioButton_manual;
    QRadioButton* radioButton_plan;
    QLineEdit* lineEdit_reffer;
    QLineEdit* lineEdit_saveto;
    QLineEdit* lineEdit_rename;
    QPushButton* btn_choose;
    QGroupBox* gb_explain;
    QTextEdit* textEdit;
    QLineEdit* lineEdit_url;
    QComboBox* comboBox_kind;
    QSpinBox* spinBox;
    QLabel* txt_user;
    QLineEdit* lineEdit_user;
    QLabel* txt_pwd;
    QLineEdit* lineEdit_pwd;
    QWidget* TabPage;
    QCheckBox* checkBox_use_mirror;
    QLabel* txt_explain;
    QPushButton* btn_auto_scan;
    QPushButton* btn_del_mirror;
    QPushButton* btn_add_mirror;
    QPushButton* btn_clear_all;
    QPushButton* btn_verify_mirror;
    QTable* table_mirror;
    
    QButtonGroup* buttonGroup1;
	QLabel*		http_kind; 
	QLabel*		ftp_kind; 
	QLabel*		mms_kind;
	QLabel*		rtsp_kind;
	QComboBox*		comboBox_HTTP; 
    QComboBox*		comboBox_FTP; 
    QComboBox*		comboBox_MMS; 
	QComboBox*		comboBox_RTSP; 

protected slots:
    virtual void languageChange();
	
public:

    NewTaskParam result() { return m_ntp; }

	void refreshProxyInfo(void);
	string getProxyName( QString sUrl);	

public slots:
    void slt_btn_choose();
    void slt_btn_ok();
    void slt_btn_cancel();
    void slt_radiobtn_auto();
    void slt_radiobtn_manual();
    void slt_radiobtn_plan();
    void slt_checkbox_auth();
    void slt_checkbox_use_mirror();
    void slt_clear_all();
    void slt_btn_autoscan();
    void slt_btn_addmirror();
    void slt_btn_delmirror();
    void slt_btn_verify();
    void slt_current_changed(QWidget*);

	void slt_url_changed(const QString&);
	void slt_set_caption(QString str);
protected:
    void mousePressEvent( QMouseEvent *);
    void mouseMoveEvent( QMouseEvent *);

protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
	
private:
    QString getFile(QString url);
    void Init();
    void init_mirror_table();
	void add_mirror_url_item(QString url, bool b = false);

	void get_current_html_page();
	void restore(NewTaskParam& ntp);

    QPoint clickPos;

private:
    NewTaskParam m_ntp;
};

#endif
