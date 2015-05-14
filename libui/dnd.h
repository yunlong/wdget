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

#ifndef	__DND_H__
#define	__DND_H__

#include <qwidget.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h>

#include <qevent.h>
#include <qdragobject.h>
#include <qwidget.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qframe.h>
#include <stdlib.h>

#include "mapchart.h"

class QGridLayout;
class QWidget;
class QImage;
class QPixmap;
class QFile;
class QEvent;
class QDragObject;


class dnd : public QWidget
{
	Q_OBJECT
public:
    dnd(QWidget *parent=0, const char *name=0, WFlags f = 0);
    ~dnd();

    void SetOnTop();
	void __setState(WId win, unsigned long state);
/*
public:	
	QGridLayout * mapLayout;
	MapChart * OrigMap;
*/
public slots:

protected:
    void mousePressEvent( QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent* e);
    void mouseMoveEvent( QMouseEvent *);

protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
    void contextMenuEvent ( QContextMenuEvent * );

private:
    QPoint clickPos;
};

#endif	//	__DND_H__
