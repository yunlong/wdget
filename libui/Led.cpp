#include <qpixmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpainter.h>
#include <qrect.h>
#include <qtimer.h>
#include "Led.h"

#include "wdget.h"

TLed::TLed(QWidget *parent,const char *name,WFlags f)
	:QFrame(parent,name,f),cellCount(0)
{
	setLineWidth(2);
	setMidLineWidth(2);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);

	loadLedImages();
}

TLed::~TLed(void)
{
	vvCell.clear();
	vvLedPixmap.clear();
}

void TLed::initLed(off_t file_total_size, off_t per_size)
{
	totalSize = file_total_size;
	blockSize = per_size;

	if (file_total_size == 0 || blockSize == 0)
		return;

	//Initialize vvCell
	cellCount = totalSize / blockSize;
	if(totalSize % blockSize != 0)
		cellCount++;

	if (vvCell.size() > 0)
		vvCell.clear();

	Cell tmp;
	tmp.status = WAIT;
	vvCell.reserve(cellCount);
	for(uint i = 0; i < cellCount; i++)
	{
		vvCell.append(tmp);
	}

	adjustSize();
}

void TLed::drawLed(off_t offset, off_t length)
{
	if(offset > totalSize)
	{
		qWarning("illegal offset value,set offset to max");
		offset = totalSize;
		length = 0;
	}
	else if( (offset + length) > totalSize )
	{
		qWarning("illegal length value,set length to max");
		length = totalSize - offset;
	}

	off_t i = offset / blockSize;
	off_t j = (offset + length) / blockSize;

	for(off_t k = i; k < j; k++)
	{
		vvCell[k].status = DONE;
		update( QRect(vvCell[k].x, vvCell[k].y, cellWidth, cellHeight) );
	}
	
	if( (offset + length) % blockSize != 0 )
	{
		if( (offset + length) < totalSize)
			vvCell[j].status = CURRENT;
		else
			vvCell[j].status = DONE;

		update( QRect(vvCell[j].x, vvCell[j].y, cellWidth, cellHeight) );
	}
	else if( j < cellCount && vvCell[j].status != DONE)
	{
		vvCell[j].status = CURRENT;
		update( QRect(vvCell[j].x, vvCell[j].y, cellWidth, cellHeight) );
	}
}

void TLed::clearled(void)
{
	for(uint i = 0; i < cellCount; i++)
	{
		vvCell[i].status = WAIT;
	}

	update();
}

void TLed::loadLedImages()
{
	QStringList ledImages;
	ledImages.append(INSTALLDIR"images/grey.bmp");
	ledImages.append(INSTALLDIR"images/green.bmp");
	ledImages.append(INSTALLDIR"images/blue.bmp");

	QPixmap pix;
	QStringList::iterator it = ledImages.begin();

	useImage = true;
	int w = 0, h = 0;

	for(; it != ledImages.end(); ++it)
	{
		if(pix.load(*it) == false)
		{
			qWarning("Load image %s error!", (*it).local8Bit().data());
			useImage = false;
			break;
		}
		else
		{
			if(w == 0)
			{
				w = pix.width();
				h = pix.height();
			}
			else if(w != pix.width() || h != pix.height())
			{
				qWarning("Led images' format are not correspond to each other!");
				vvLedPixmap.clear();
				useImage = false;
				break;
			}
			vvLedPixmap.append(pix);
		}
	}

	if(useImage)
	{
		cellWidth   = w;
		cellHeight  = h;
		cellSpacing = 0;
	}
	else
	{
		cellWidth = cellHeight = 8;
		cellSpacing = 1;
	}
}

void TLed::paintEvent(QPaintEvent* pe)
{
	QRect re = pe->rect();
	QPainter paint(this);

	if(useImage == TRUE)
	{ 
		//Use images
		QValueVector<Cell>::iterator it = vvCell.begin();
		for( ; it != vvCell.end(); ++it)
		{
			QRect r( (*it).x, (*it).y, cellWidth, cellHeight);
			if( r.intersects(re) )
				paint.drawPixmap( (*it).x, (*it).y, vvLedPixmap[(int)((*it).status)] );
		}
	}
	else
	{ //Draw color rectangles
		paint.setPen( QPen(white, 1) );
		QValueVector<Cell>::iterator it = vvCell.begin();
		
		for( ; it != vvCell.end(); ++it)
		{
			QRect r( (*it).x, (*it).y, cellWidth, cellHeight);
			if(r.intersects(re) == false)
				continue;

			switch((*it).status)
			{
				case WAIT:
					paint.setBrush(gray);
					break;
				case CURRENT:
					paint.setBrush(red);
					break;
				case DONE:
					paint.setBrush(blue);
					break;
				default:
					paint.setBrush(gray);
					qWarning("Unrecognized status!");
			}

			paint.drawRect( (*it).x, (*it).y, cellWidth, cellHeight);
		}
	}
}

void TLed::resizeEvent(QResizeEvent* re)
{
	if(cellCount == 0)
		return;
	adjustSize(re->size().width(), re->size().height());
}

void TLed::adjustSize(int w,int h)
{
	if(w == 0)
	{
		w = width();
		h = height();
	}

	int gw = cellWidth + cellSpacing * 2;
	int gh = cellHeight + cellSpacing * 2;

	uint col = w / gw;

	uint c = 0,r = 0;
	for(uint i = 0; i < cellCount; i++, c++)
	{
		if(c >= col)
		{
			c = 0;
			r++;
		}

		Cell &tmp = vvCell[i];
		tmp.x = gw * c + cellSpacing;
		tmp.y = gh * r + cellSpacing;
	}

	int bottom = vvCell[cellCount - 1].y + cellHeight + cellSpacing;
	if(bottom > h)
		resize(w, bottom);

	update();
}

//////////////////////////////////////////////////////

TLedImpl::TLedImpl(QWidget *parent,const char *name,WFlags f)
	:QScrollView(parent,name,f)
{
	setHScrollBarMode(QScrollView::AlwaysOff);
	setMinimumSize(100, 20);

	d = new TLed;
	addChild(d);
}

TLedImpl::~TLedImpl()
{
	if(d)
		delete d;
	d=0;
}

void TLedImpl::resizeEvent(QResizeEvent *)
{
	//QScrollView的状态更新不够及时，包括滚动条显示与否、visibleWidth等参数，
	//所以有必要自己判断是否显示滚动条并更新contents的大小。
	QTimer::singleShot(0, this, SLOT( slotCheckScrollBarStatus() ) );
}

void TLedImpl::slotCheckScrollBarStatus()
{
 	QScrollBar *vbar = verticalScrollBar();
	int w = width() - 4, h = height() - 4;
	int w1 = w;
	d->resize(w, h);

	if( d->height() <= height() && vbar->isVisible() )
	{
		vbar->hide();
	}
	else if( d->height() > height() && vbar->isVisible() == false)
	{
		vbar->show();
	}

	if( vbar->isVisible() )
	{
		w1 -= vbar->width();
	}

	if(w != w1)
		d->resize(w1, h);
}

