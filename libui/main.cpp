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


#include <sys/types.h>
#include <sys/stat.h>

#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qdir.h>
#include <qstyle.h>
#include <qstylefactory.h>

#include "frmMainWin.h"
#include "listviews.h"
#include "wdget.h"

QTranslator ts( 0 );

QApplication* GlobalApp = NULL;

extern FrmMainWin * dmw;

bool IsRunning(int argc, char** argv)
{       
    bool bRun = false;
        
    char buf[64];
    char path[64];
    char str_pid[64];

    memset(buf, 0, sizeof(buf));
    memset(path, 0, sizeof(path));
    memset(str_pid, 0, sizeof(str_pid));

        
    int res = access("/tmp/.dget.lock", F_OK);
        
    FILE* fp = NULL;

    if (res == 0)
    {   
        fp = fopen("/tmp/.dget.lock", "r");

        if (!fp)
        {
            goto out;
        }
        
        fgets(buf, sizeof(buf), fp);
        fclose(fp);

        int len = strlen(buf);

        if (buf[len - 1] == '\n')
            --len;

        strcpy(path, "/proc/");
        strncpy(path + strlen("/proc/"), buf, len);
        printf("path = %s\n", path);

        if (access(path, F_OK) == 0)
        {
            bRun = true;
        }
    }

out:
    if (bRun == false)
    {
        pid_t pid = getpid();
        sprintf(str_pid, "%d\n", pid);
    }
    else if (bRun == true)
    {
        strncpy(str_pid, buf, strlen(buf));
    }


    fp = fopen("/tmp/.dget.lock", "w");

    if(!fp)
    {
        return false;
    }


    fwrite(str_pid, 1, strlen(str_pid), fp);

    for (int i = 0; i < argc; ++i)
    {
        fwrite(argv[i], 1, strlen(argv[i]), fp);
        fputs("\n", fp);
    }

    fclose(fp);
    chmod("/tmp/.wdget.lock", 0666);

    return (!bRun);
}

int main( int argc, char ** argv )
{
	if(IsRunning(argc, argv))
	{
		int sw, sh;
 
		GlobalApp = new QApplication( argc, argv );	

		QString base = QDir(INSTALLDIR"lang").absPath(); 
		QString LangFile = QString( "dget_" ) + QTextCodec::locale();
		ts.load(LangFile, base);
		GlobalApp->installTranslator( &ts );	
	
		QStyle * s = QStyleFactory::create( "CDE" );
    	if ( s )
        	QApplication::setStyle( s );	
		
    	dmw = new FrmMainWin();
    	dmw->setCaption( "wdget for x11" );

		GlobalApp->setMainWidget(dmw);
    	
  		QDesktopWidget * Screen = QApplication::desktop();
		sw = Screen->width();     
    	sh = Screen->height();
   	
    	dmw->move((sw - dmw->width()) / 2 , (sh - dmw->height()) / 2);
    	dmw->show();
    	return GlobalApp->exec();
	}
}
