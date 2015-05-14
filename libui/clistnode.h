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

#ifndef CLISTNODE_H
#define CLISTNODE_H

#include <qvariant.h>
#include <qwidget.h>
#include <qlistview.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QListView;
class QListViewItem;

class CMyQListViewItem : public QListViewItem
{
public:
	int ThreadId;
	int m_flag;
	int idx;
public:
	CMyQListViewItem(QListView* parent): QListViewItem(parent)
	{
		idx			= -1;
		m_flag		= 0;
		ThreadId	= -1;
		setDragEnabled(true);
		setDropEnabled(true);
	}

	CMyQListViewItem(CMyQListViewItem* parent) : QListViewItem(parent)
	{
		idx			= -1;
		m_flag 		= 0;
		ThreadId	= -1;
		setDragEnabled(true);
		setDropEnabled(true);
	}	

	void SetChangeColor(int flag)
	{
		m_flag = flag;
	}

	void paintCell(QPainter* p, const QColorGroup & cg, int column, 
													int width, int align)
	{
		QColorGroup qg( cg );

		if (m_flag == 0)
			qg.setColor(QColorGroup::Text, Qt::blue);
		else if (m_flag == 1)
			qg.setColor(QColorGroup::Text, Qt::red);
		else if (m_flag == 2)
			qg.setColor(QColorGroup::Text, Qt::green);
		else
			qg.setColor(QColorGroup::Text, Qt::black);

		QListViewItem::paintCell( p, qg, column, width, align);
	}

};


class CListNode : public QWidget
{
    Q_OBJECT

public:
    CListNode( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CListNode();

	QListView* OrigList;
    QGridLayout* CListNodeLayout;

protected slots:
    virtual void languageChange();

};

#endif 
