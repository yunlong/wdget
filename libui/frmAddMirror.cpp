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
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "frmAddMirror.h"

frmAddMirror::frmAddMirror( QWidget* parent, const char* name, 
											bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AddMirror" );

    txt_mirror = new QLabel( this, "txt_mirror" );
    txt_mirror->setGeometry( QRect( 10, 20, 80, 20 ) );

    lineEdit_mirror = new QLineEdit( this, "lineEdit_mirror" );
    lineEdit_mirror->setGeometry( QRect( 80, 20, 260, 20 ) );

    btn_ok = new QPushButton( this, "btn_ok" );
    btn_ok->setGeometry( QRect( 190, 50, 70, 23 ) );

    btn_cancel = new QPushButton( this, "btn_cancel" );
    btn_cancel->setGeometry( QRect( 270, 50, 70, 23 ) );
    languageChange();
    resize( QSize(353, 83).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( lineEdit_mirror, btn_ok );
    setTabOrder( btn_ok, btn_cancel );
	///////////////////////////////////////
	setFixedSize(QSize(353, 83));

	connect(btn_ok, SIGNAL(clicked()), this, SLOT(slt_btn_ok()));
	connect(btn_cancel, SIGNAL(clicked()), this, SLOT(slt_btn_cancel()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
frmAddMirror::~frmAddMirror()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void frmAddMirror::languageChange()
{
    setCaption( tr( "AddNewMirror" ) );
    txt_mirror->setText( tr( "Mirror Url" ) );
    btn_ok->setText( tr( "OK" ) );
    btn_cancel->setText( tr( "Cancel" ) );
}


void frmAddMirror::slt_btn_ok()
{
	url = lineEdit_mirror->text();
	done(1);
}

void frmAddMirror::slt_btn_cancel()
{
	done(0);
}
