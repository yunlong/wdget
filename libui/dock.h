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

#ifndef __DOCK_H
#define __DOCK_H

#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qevent.h>

#include "qsystemtray.h"
#include "frmMainWin.h"

class FrmMainWin;
class QPopupMenu;

class TDock : public QSystemTray
{
	Q_OBJECT
private:
    FrmMainWin * MyParent;
	bool	bShowDnd;

public:
    TDock( FrmMainWin * parent);
    ~TDock();
	virtual void mousePressEvent(QMouseEvent* e);

public slots:
	void SetDndShow( void );
};

#endif
