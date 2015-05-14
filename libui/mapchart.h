#ifndef MAPCHART_H
#define MAPCHART_H

#include "qframe.h"
#include <qptrlist.h>
#include <qpoint.h>

class MapChart : public QFrame
{ 
public:	

	MapChart(QWidget * parent, const char * name = 0 ,WFlags f = 0,int interval = 1000 );
	virtual void timerEvent( QTimerEvent*);
	void refresh();
	//根据*sense绘制线条
	//默认参数0为继续绘制(用于pause()后恢复计数)
	void start(int* sense=0);
	void pause();
	void stop();
	//设置水平线间距,不可在计数中使用
	bool setBlocksize(int blocksize);
	//设置关键点间距
	bool setStep(int step);
protected:
	virtual void resizeEvent( QResizeEvent *e );
	void paintEvent( QPaintEvent *e );
	
	QPtrList<int> drawTimeList;
	QPtrList<QPoint> pointList;

	bool resizeSignal;
	bool gTimerKey;
	int* sense;

};

#endif //MAPCHART_H
