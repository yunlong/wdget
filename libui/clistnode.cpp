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
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlistview.h>

#include "clistnode.h"


#include "wdget.h"


CListNode::CListNode( QWidget* parent, const char* name , WFlags fl  )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "ListNode" );
    CListNodeLayout = new QGridLayout( this, 1, 1, 11, 6, "ListNodeLayout"); 

	OrigList = new QListView(this);
	OrigList->setSelectionMode( QListView::Single );
	OrigList->clear();
    OrigList->header()->hide();
    OrigList->addColumn("");
	QPixmap pix_graphic(INSTALLDIR"images/graphic.png");
    CMyQListViewItem* rtLV = new CMyQListViewItem(OrigList);
    rtLV->setText(0, tr("Graphic/Log") );
    rtLV->setOpen(TRUE);
    rtLV->setPixmap(0, pix_graphic);
    rtLV->setExpandable(TRUE);
	
	CListNodeLayout->addWidget( OrigList, 0, 0 );
  
	languageChange();
    resize( QSize(100, 300).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CListNode::~CListNode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CListNode::languageChange()
{
    setCaption( QString::null );
}

