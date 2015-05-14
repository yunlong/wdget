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
#include <qtable.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qlistview.h>

#include "pic.h"
#include "Led.h"

/*
 *  Constructs a CPic as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */

CPic::CPic( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
	
    if ( !name )
	setName( "CPic" );
	
	CPicLayout = new QGridLayout( this, 1, 1, 11, 6, "PicLayout" );
			
	OrigListbox = new QListView(this);
	OrigListbox->setSelectionMode( QListView::Extended );
	OrigListbox->setAllColumnsShowFocus(true);
    OrigListbox->header()->setClickEnabled( FALSE );
	OrigListbox->header()->setStretchEnabled( FALSE );
	OrigListbox->addColumn( tr( "time" ) );
    OrigListbox->addColumn( tr( "information" ) );
    OrigListbox->setColumnWidth(0, 175);
    OrigListbox->setColumnWidth(1, 350);	
	OrigListbox->setColumnWidthMode(0, QListView::Manual );
	OrigListbox->setColumnWidthMode(1, QListView::Maximum );
	OrigListbox->setSortColumn(-1);

	CPicLayout->addWidget(OrigListbox, 0, 0 );
	
	languageChange();

    resize( QSize(545, 473).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

}

/*
 *  Destroys the object and frees any allocated resources
 */
CPic::~CPic()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */



void CPic::languageChange()
{
    setCaption( tr( "Pic" ) );
}
