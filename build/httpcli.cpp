#define USE_SOCKET_H
#define USE_UTILS_H
#define USE_URI_H
#define USE_HTTPMSG_H
#define USE_HTTPCLI_H
#define USE_FTPCLI_H
#define USE_THREAD_H
#define USE_TASKMGR_H
#define USE_TASK_H
#define USE_INIFILE_H

#include "matrix.h"

using namespace matrix;

int test_http(void)
{
    timeval tv;
    gettimeofday( &tv, NULL );
    UINT start  = (tv.tv_sec*1000 + tv.tv_usec/1000);

	THttpCli * hcp = new THttpCli();

//	string sURL = "http://www.amazon.cn/mn/browseApp?showType=1&nodeid=51080&sortType=market&pageNow=1";
	string sURL = "http://www.amazon.cn/mn/detailApp/478-7900914-8125608?ref=BR&uid=478-7900914-8125608&prodid=bkbk852094";
	hcp->fetchHtmlPage( sURL );

//	string sURL = "http://cdnetworks-kr-2.dl.sourceforge.net/project/itextsharp/itextsharp/iTextSharp-5.0.2/itextsharp-5.0.2.zip";
//	hcp->fetchHtmlPage( sURL, "itextsharp-5.0.2.zip" );

//string sURL =	"http://www.kernel.org/pub/linux/kernel/v2.6/linux-2.6.34.1.tar.bz2";
//	client->fetchPage( sURL.c_str(), "linux-2.6.34.1.tar.bz2" );
	cout << hcp->getHtmlContent() << endl;
	
	delete hcp;

    gettimeofday( &tv, NULL );
    UINT end  = (tv.tv_sec*1000 + tv.tv_usec/1000);
	cout << (end - start) / (double)1000 << endl;
	return 0;
}


int test_ftp(void)
{
//	if(argc < 2)
//		return -1;

    timeval tv;
    gettimeofday( &tv, NULL );
    UINT start  = (tv.tv_sec*1000 + tv.tv_usec/1000);

	TFtpCli * fcp = new TFtpCli();

	//string sURL =	"ftp://ftp.kernel.org/pub/linux/kernel/v1.0/linux-1.0.tar.gz";
	string sURL = "ftp://ftp.kernel.org/pub/linux/kernel/v2.0/linux-2.0.10.tar.gz";
	fcp->fetchRemoteFile( sURL.c_str() );
	delete fcp;

    gettimeofday( &tv, NULL );
    UINT end  = (tv.tv_sec*1000 + tv.tv_usec/1000);
	cout << (end - start) / (double)1000 << endl;
	return 0;
}

int test_task(void)
{
//	string sURL = "http://cdnetworks-kr-2.dl.sourceforge.net/project/itextsharp/itextsharp/iTextSharp-5.0.2/itextsharp-5.0.2.zip";
//	string sURL = "http://www.amazon.cn/mn/detailApp/478-7900914-8125608?ref=BR&uid=478-7900914-8125608&prodid=bkbk852094";
	string sURL = "http://www.kernel.org/pub/linux/kernel/v2.0/linux-2.0.8.tar.gz";
//	string sURL = "ftp://ftp.kernel.org/pub/linux/kernel/v2.0/linux-2.0.10.tar.gz";
//	string sURL = "mms://v1.im.tv/music/12/09/16/c184269bc5fe4feeaa48d2f7b4329227.mp3";
//	string sURL = "mms://vod.tom.com/music/guhaibin/peijiao.wmv";

	TTask * tsk = new TTask(sURL);
	tsk->fetchHttpRemoteFileInfo();
//	tsk->fetchFtpRemoteFileInfo();
//	tsk->fetchMmsRemoteFileInfo();
//	tsk->fetchRtspRemoteFileInfo();

	int pos = tsk->uri->path().find_last_of('/');
	string  dir, file;
	if(pos != string::npos)
	{    
		dir = tsk->uri->path().substr(1, pos);
		file = tsk->uri->path().substr(pos + 1);

		if ( !file.empty() )
		{

		}

		if(!dir.empty())
		{
		}
	}    

//	cout << tsk->uri->path() << '\t' << dir << '\t' << file << endl;

	tsk->setOutDir("/tmp");
	tsk->setOutFilePath("/tmp/" + file);
	tsk->setLogFile("/tmp/" + file + ".log");

	if(tsk->FileSize == -1)
	{
		tsk->createThreadQue(1);
	}
	else
	{
		tsk->createThreadQue(4);
	}

	tsk->initTaskThreadQue();

	FILE * fp = NULL;
	string outfile = "/tmp/" + file + ".dg";
	if ((fp = fopen(outfile.c_str(), "wb")) == NULL)
		return -1;
	if(fseeko(fp, tsk->FileSize, SEEK_SET) != -1) 
		fclose(fp);

	if(tsk->createLogFile() == -1)
	{
		ShowMsg("Error opening file  for writing logfile");
		return -1 ;
	}                                       

    tsk->setdlStartTime();
    tsk->setTaskStatus(TSK_RUNNING);
    tsk->execDownloads();  
	delete tsk;
}

void test_config(void)
{
    IniFile cfg("/usr/local/wdget/config/wdget.ini");

	string host = "172.16.23.52";
	string type = "http";
	string user = "yunlong.lee";
	string pass = "123456";
	bool auth = false;

    cfg.write("proxy", "type", type);
    cfg.write("proxy", "user", user);
    cfg.write("proxy", "pass", pass);
    cfg.write("proxy", "host", host);
    cfg.write("proxy", "port", 3128);
    cfg.write("proxy", "auth", auth);
	cfg.show_config();

}

int main(int argc, char **argv)
{
//	test_config();
	test_http();
//	test_ftp();
//	test_task();
	return 0;
}
