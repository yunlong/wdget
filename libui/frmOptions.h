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

#ifndef FRMOPTIONS_H
#define FRMOPTIONS_H

#include <qvariant.h>
#include <qdialog.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qtabwidget.h>
#include <vector>

#include "frmOptions.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QTabWidget;
class QWidget;
class QGroupBox;
class QLineEdit;
class QTable;
class QCheckBox;
class QLabel;
class QSpinBox;
class KColorButton;

class frmOptions : public QDialog
{
    Q_OBJECT

public:
    frmOptions( QWidget* parent = 0, const char* name = 0, 
								bool modal = FALSE, WFlags fl = 0 );
    ~frmOptions();

    QPushButton* btn_ok;
    QPushButton* btn_cancel;
	
    QTabWidget* tabWidget;

    QWidget* tab1;	
    QGroupBox* gb1;
    QLineEdit* lineEdit_download_dir;
    QPushButton* btn_choose_dir;
	
    QWidget* tab2;
    QTable* table_proxy;
    QPushButton* btn_new;
    QPushButton* btn_edit;
    QPushButton* btn_del;
    QPushButton* btn_verify;
    QPushButton* btn_import;
    QPushButton* btn_export;
	
    QWidget* tab3;
    QCheckBox* checkBox_use_default;
	
	QGroupBox* gb2;
    QLabel* txt_max_task_num;
    QSpinBox* spinBox_task_num;	
	
	QLabel* txt_max_thread_num;
    QSpinBox* spinBox_thread_num;
	
	QLabel* txt_retry_times;
    QSpinBox* spinBox_retry_times;
	
    QLabel* txt_retry_interval;
    QSpinBox* spinBox_retry_interval;
	QLabel* txt_retry_sec;

	QLabel*txt_max_redir;
    QSpinBox* spinBox_max_redir ;

    QSpinBox* spinBox_timeout_wait;
    QLabel* txt_timeout_wait;
    QLabel* txt_timeout_sec;

	QGroupBox* gb3;
    QLabel* txt_min_block_size;
    QSpinBox* spinBox_min_block_size;
    QLabel* txt_socket_buffer;
    QSpinBox* spinBox_socket_buffer;
    QCheckBox* checkBox_sync;
    QSpinBox* spinBox_sync_size;
    QLabel* txt_min_KB;
    QLabel* txt_socket_KB;
    QLabel* txt_sync_KB;
	
protected:
    QString filename;
    QFileInfo fileinfo;

protected slots:
    virtual void languageChange();
	void slotStyleChanged( int i );
	void slotLangChanged( int );
/////////////////////////////////////////
private:
	int max_task_num;
	int max_thread_num;
	int max_redir_num;
	
	int min_blk_size;
	
	int retry_times;
	int retry_interval;
	int timeout_wait;
	
	int recv_buf_size;
	int sync_cache_size;

	int read_from_engine_file(int flag);
	void init_proxy_table();
	void refresh_proxy_table();
	void init_param();
	void proxy_apply();
	void engine_param_apply();
	void set_enabled(bool b);
	bool check_select();

	void write_proxy_inf(const char * path);
	void write_engine_inf(void);

public slots:

	void slt_btn_choose_dir();

	void slt_btn_add_proxy();
	void slt_btn_del_proxy();
	void slt_btn_edit_proxy();

	void slt_btn_import_proxy();
	void slt_btn_export_proxy();

	void slt_checkBox_sync();
	void slt_checkBox_use_default();

	void slt_btn_ok();
	void slt_btn_cancel();

};

#endif
