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

#include <qapplication.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qevent.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qclipboard.h>
#include <stdlib.h>
#include <qlayout.h>

#include "frmMainWin.h"
#include "dnd.h"
#include "mapchart.h"

#include "netwm.h"

#include "wdget.h"

extern FrmMainWin * dmw;

dnd::dnd( QWidget *parent, const char *name,  WFlags f) 
			: QWidget(parent,name,f)
{
/*
	int speed = 0;
	mapLayout = new QGridLayout( this, 1, 1, 11, 6, "mapLayout" );
	OrigMap = new MapChart(this, 0, 0, 500);
    OrigMap->setStep(3);
    OrigMap->start(&speed);
	mapLayout->addWidget(OrigMap, 0, 0 );
*/
	setAcceptDrops(TRUE);
	SetOnTop();
}

dnd::~dnd()
{
	//Empty
}

void dnd::mousePressEvent( QMouseEvent *e )
{
    clickPos = e->pos();
}

void dnd::mouseMoveEvent( QMouseEvent *e )
{
    move( e->globalPos() - clickPos );
}

void dnd::dragEnterEvent( QDragEnterEvent *e )
{
    e->accept(QUriDrag::canDecode(e) || QTextDrag::canDecode(e));
}

void dnd::dropEvent( QDropEvent * e )
{
    QStrList list;
    QString str;

    if ( QUriDrag::decode( e, list ) )
    {
        for(const char* u = list.first(); u; u = list.next())
        {
			if(u)
				str = u;
        }
    }
    else
    {
        str = QString(e->encodedData("text/plain"));
    }
	
	if(str.isEmpty()) return;

	QApplication::clipboard()->setText(str);
	dmw->fileNewUrl();
}

void dnd::contextMenuEvent ( QContextMenuEvent * )
{

	QString path = INSTALLDIR"images/";

    QString snewtask = path + "add.png";
    QString soptions = path + "preferences.png";  
    QString sabout = path + "about.png";
    QString sexit = path + "exit.png";

    QPixmap pix_newtask(snewtask);
    QPixmap pix_options(soptions);
    QPixmap pix_about(sabout);
    QPixmap pix_exit(sexit);

	QPopupMenu* contextMenu = new QPopupMenu( this );
    Q_CHECK_PTR( contextMenu );
	QLabel *caption = new QLabel(tr("<font color=darkblue><u><b>wdget</b></u></font>"), this );
	caption->setAlignment( Qt::AlignCenter );
	contextMenu->insertSeparator();
	contextMenu->insertItem( caption );
	contextMenu->insertSeparator();
	contextMenu->insertItem(pix_newtask, tr("New task"), dmw, SLOT(fileNewUrl()) );
	contextMenu->insertItem(pix_options, tr("Setting..."), dmw, SLOT(filePref()) );
	contextMenu->insertItem(pix_about,  tr("About..."), dmw, SLOT(helpAbout()) );
	contextMenu->insertItem(pix_exit,  tr("Quit"), dmw, SLOT(fileExit()) );
	contextMenu->exec( QCursor::pos() );
	delete contextMenu;
}

void dnd::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (dmw->isShown())
		{
			dmw->hide();
		}
		else
		{
			dmw->show();
		}
	}
}

void dnd::SetOnTop()
{
	return __setState(winId(), NET::SkipTaskbar | NET::StaysOnTop);
}

void dnd::__setState(WId win, unsigned long state)
{
	NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), NET::WMState );
	info.setState( state, state );
}


