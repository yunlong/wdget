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

#include <qcursor.h>
#include "dock.h"
#include "frmMainWin.h"

#include "wdget.h"

extern FrmMainWin * dmw;

TDock :: TDock(FrmMainWin * parent)
{
    MyParent = parent;

	QString path = INSTALLDIR"images/drag.png";
	QPixmap pix_dnd(path);
    setPixmap(pix_dnd);
    
	bShowDnd = false;
}


TDock :: ~TDock()
{
}

void TDock::mousePressEvent(QMouseEvent * e)
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
	else if (e->button() == Qt::RightButton)
	{
		
		QString path = INSTALLDIR"images/";

		QString snewtask = path + "add.png";
    	QString soptions = path + "preferences.png";
    	 QString sdnd = path + "dnd.png";
    	QString sabout = path + "about.png";
    	QString sexit = path + "exit.png";

    	QPixmap pix_newtask(snewtask);
    	QPixmap pix_options(soptions);
    	 QPixmap pix_dnd(sdnd);
    	QPixmap pix_about(sabout);
    	QPixmap pix_exit(sexit);

	    QPopupMenu * contextMenu = new QPopupMenu( this );
	    Q_CHECK_PTR( contextMenu );

	    QLabel * caption = new QLabel(tr("<font color=darkblue><u><b>wdget</b></u></font>"), this );
	    caption->setAlignment( Qt::AlignCenter );
		contextMenu->insertSeparator();
	    contextMenu->insertItem( caption );
	    contextMenu->insertSeparator();
	    contextMenu->insertItem( pix_newtask, tr("NewTask"), dmw, SLOT(fileNewUrl()) );
		contextMenu->insertItem( pix_options, tr("Preference"), dmw, SLOT(filePref()) );
		contextMenu->insertItem( pix_dnd, tr("Show DND"), this, SLOT(SetDndShow()) );
	    contextMenu->insertItem( pix_about, tr("About ..."), dmw, SLOT(helpAbout()) );
	    contextMenu->insertItem( pix_exit, tr("Quit"), dmw, SLOT(fileExit()) );

		if (bShowDnd)
		{
			int _id = contextMenu->idAt(3);
			contextMenu->setItemChecked(_id, true);
		}
	    contextMenu->exec( QCursor::pos() );

	}

}

void TDock :: SetDndShow( void )
{
	bShowDnd = !bShowDnd;
	return dmw->ShowDnd(bShowDnd);
}

