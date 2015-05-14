#include "mapchart.h"
#include <qpixmap.h>
#include <qpainter.h>


//所有的mapchart共同使用的参数
int BLOCKSIZE;
int STEP;
int MAXRECORD;
int maxvalue = 2 * 1024;	//(speed=2M)
MapChart::MapChart( QWidget * parent, const char * name , WFlags f,int interval)
							: QFrame(parent, name,f)
{
	startTimer(interval);
	pointList.setAutoDelete(true);
	drawTimeList.clear();
	gTimerKey = false;
	//这个参数没有测试过
	BLOCKSIZE = 20;
	STEP = 2;
	MAXRECORD = 1000;
}

void MapChart::resizeEvent( QResizeEvent *)
{
	resizeSignal = true;
}

void MapChart::paintEvent( QPaintEvent *)
{
	int w = this->width();
	int h = this->height();
	int val=maxvalue;

	QPixmap pm( w, h );
	QPainter p(&pm);
	p.translate(0, h);
	p.scale(1.0, -1.0);
	p.setPen(NoPen);
	p.fillRect(0, 0, w, h, QBrush(Qt::black));

	p.setPen(QPen(QColor::QColor(0,120,0), 1));

	int i=1;
		//画横线
	for(; i * BLOCKSIZE < h;i++)
	{
		p.drawLine(0, h - i * BLOCKSIZE, w, h - i * BLOCKSIZE);
	}

	//p.setPen(QPen(QColor::QColor(255,150,150), 1));
	//p.setPen(QPen(QColor::QColor(0,255,0), 1));
	p.setPen(QPen(QColor::QColor(255,0,0), 1));

	pointList.clear();


	for(int i = 0; i < this->width() / STEP + 1;i++)
		pointList.append( new QPoint(0,0) );

	for( uint k = 0; k < drawTimeList.count(); k++)
	{
		pointList.removeFirst();
		if((int)drawTimeList.at(k)==0)
		{
			pointList.append( new QPoint(0, 2*maxvalue/h) );
		}
		else
		{
			pointList.append( 
					new QPoint(0, (int)drawTimeList.at(k) > maxvalue ?
								 maxvalue : (int)drawTimeList.at(k) ) 
			);
		}
	}

	for( i = 0; i < (int)pointList.count() - 1; i++)
	{

//		p.drawLine( i*STEP, pointList.at(i)->y() * h/maxvalue, (i+1)*STEP, pointList.at(i+1)->y() * h/maxvalue ); 
		p.drawRect( i*STEP, pointList.at(i)->y() * h/maxvalue, STEP,
			pointList.at(i)->y() * h/maxvalue
		 ); 

		p.drawRect( i*STEP, pointList.at(i+1)->y() * h/maxvalue, STEP,
			pointList.at(i+1)->y() * h/maxvalue
		 ); 

	}

	//标刻度,保证不超过10个刻度,并且均匀分布
	p.scale(1, -1);
	p.translate(0, -h);
	QFont qf = p.font();
	qf.setPointSize(8);
	p.setFont(qf);
	p.setPen(QColor(150, 255, 150));
	int t = (int)(h / (float)BLOCKSIZE) - 1;
	if(t == 0) t = 1;
	t = t > 8 ? 8 : t;
	switch(t)
	{
	case 2:
	case 3:
		t = 2;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		t = 4;
		break;
	}
	
	//	if(t == 8 && val == 20 * 1024) t = 10;
	//	if(t == 10 && val < 20 * 1024) t = 8;
	int tBLOCKSIZE = (h - BLOCKSIZE) / t;
	int posy = 4 + BLOCKSIZE / 2;

	if(val == 2 * 1024)
	{
		//maxspeed=2M

		p.drawText(10, posy, "2M");
		posy += tBLOCKSIZE;
		val -= 2 * 1024 / t;
		for (i = 1; i < t; i++)
		{
			val >= 1024 ? p.drawText(10, posy, QString::number(val / 1024.0) + "M"): 
				 					p.drawText(10, posy, QString::number(val) + "K");
									
			val -= 2 * 1024 / t;
			posy += tBLOCKSIZE;
		}
		val = 512 / t;
		p.drawText(10, posy, QString::number(val) + "K");

	}
	bitBlt(this, 0 ,0, &pm);                  
}

void MapChart::timerEvent(QTimerEvent* )
{
	if( gTimerKey )
		refresh();
}

void MapChart::refresh( )
{
	if( (int)drawTimeList.count() > MAXRECORD )
		drawTimeList.removeFirst();
	drawTimeList.append( (const int*)(*sense));
	paintEvent(NULL);
}

//计数开始
void MapChart::start(int* sense)
{
	if(sense != 0)
		this->sense = sense;
	gTimerKey = true;
}

//暂停
void MapChart::pause()
{
	gTimerKey = false;
}

//停止并清除历史纪录
void MapChart::stop()
{
	gTimerKey = false;
	drawTimeList.clear();
}

//设置水平线间距
bool MapChart::setBlocksize(int blocksize)
{
	if(gTimerKey) return false;
	BLOCKSIZE = blocksize;
	return true;
}
//设置关键点间距
bool MapChart::setStep(int step)
{
	if(gTimerKey)return false;
	STEP = step;
	return true;
}
