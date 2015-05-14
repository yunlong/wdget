
/* 
library for parsing FTP LIST responses
Currently covered:
EPLF.
UNIX ls, with or without gid.
Microsoft FTP Service.
Windows NT FTP Server.
VMS.
WFTPD (DOS).
NetPresenz (Mac).
NetWare.
Definitely not covered: 
Long VMS filenames, with information split across two lines.
NCSA Telnet FTP server. Has LIST = NLST (and bad NLST for directories).
*/

#define USE_UTILS_H
#define USE_FTPPARSER_H

#include "matrix.h"


using namespace std;

namespace matrix {

TFtpParser::TFtpParser()
{
	file = NULL;
	link = NULL;
	size = 0;
	time = 0;
	type = '-';
};

TFtpParser::~TFtpParser()
{
	delete[] file;
	delete[] link;
};

void TFtpParser::reset(void)
{
	delete[] file; file = NULL;
	delete[] link; link = NULL;
	size = 0;
	time = 0;
	type = '-';
};

const char* TFtpParser::get_file(void)
{
	return file;
};

const char* TFtpParser::get_link(void)
{
	return link;
};

off_t TFtpParser::get_size(void)
{
	return size;
};

time_t TFtpParser::get_time(void)
{
	return time;
};

char TFtpParser::get_type(void)
{
	return type;
};

/*
 * 05-05-03  06:13PM        4169728       03-The Rose.mp3
 * 06-14-00  09:42PM        2394          1.htm
 * 06-14-00  09:42PM        <DIR>         DESKTOP
 */
int TFtpParser::process_dos(char *line)
{
	struct tm tmp;

	// 05-05-03
	tmp.tm_mon = atoi(line);
	tmp.tm_mon -= 1;
	while(ISDIGIT(*line)) line ++;
	if(*line != '-') return -1;
	line ++;
	tmp.tm_mday = atoi(line);
	while(ISDIGIT(*line)) line ++;
	if(*line != '-') return -1;
	line ++;
	tmp.tm_year = atoi(line);
	while(ISDIGIT(*line)) line ++;
	while(ISBLANK(*line)) line ++;
	if(tmp.tm_year < 70){
		tmp.tm_year += 2000 - 1900;
	}

	// 06:13PM
	tmp.tm_hour = atoi(line);
	while(ISDIGIT(*line)) line ++;
	if(*line != ':') return -1;
	line ++;
	tmp.tm_min = atoi(line);
	while(ISDIGIT(*line)) line ++;
	if(strncasecmp(line, "AM", 2) == 0){
		if(tmp.tm_hour == 12) tmp.tm_hour = 0;
	}else if(strncasecmp(line, "PM", 2) == 0){
		if(tmp.tm_hour != 12) tmp.tm_hour += 12;
	}else{
		return -1;
	}
	line += 2;
	while(ISBLANK(*line)) line ++;

	// 313123 or <DIR>
	size = 0;
	if(ISDIGIT(*line)){
		type = 'f';
		while(ISDIGIT(*line)){
			size = size * 10 + (*line - '0');
			line ++;
		}
	}else if(strncasecmp(line, "<DIR>", 5) == 0){
		type = 'd';
		line += 5;
	}else{
		return -1;
	}
	while(ISBLANK(*line)) line ++;
	if(*line == '\0') return -1;
	
	file = StrDup(line);
	tmp.tm_sec = 0;
	tmp.tm_wday = 0;
	tmp.tm_yday = 0;
	tmp.tm_isdst = 0;

	time = mktime(&tmp);
	if(time == -1) return -1;

	return 0;
}; // end of process_dos
	
/*
 * -rw-r--r--    1 0        0         9455616 Apr 05  2004 DOS71CD.ISO
 * -rwxrw-rw-    1 root     root        23421 Aug  7  1999 setup.exe
 * drwxrw-rw-    1 root     root            0 Aug 22 15:34 DIR
 * lrwxrwxrwx    1 1000     27             12 May 16 13:26 link -> UnxUtils.zip
 * */
int TFtpParser::process_unix(char *line)
{
	struct tm tmp;
	type = *line;

	static char* MonthStr[] = 
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	  NULL};

	// permission
	line += 10;
	while(ISBLANK(*line)) line ++;
	// 
	while(*line != '\0' && !ISBLANK(*line)) line ++;
	while(ISBLANK(*line)) line ++;
	// user
	while(*line != '\0' && !ISBLANK(*line)) line ++;
	while(ISBLANK(*line)) line ++;
	// group
	while(*line != '\0' && !ISBLANK(*line)) line ++;
	while(ISBLANK(*line)) line ++;
	// size
	if(!ISDIGIT(*line)) return -1;
	size = 0;
	while(ISDIGIT(*line)){
		size = size * 10 + (*line - '0');
		line ++;
	}
	while(ISBLANK(*line)) line ++;
	// Aug  7  1999
	// Aug 22 15:34
	int i;
	char **mon = MonthStr;
	for(i = 0; mon[i] != NULL; i ++){
		if(strncasecmp(line, mon[i], 3) == 0){
			tmp.tm_mon = i;
			line += 3;
			break;
		}
	}
	if(i >= 12) return -1;
	while(ISBLANK(*line)) line ++;
	// 7 1999
	// 22 15:34
	tmp.tm_mday = atoi(line);
	while(ISDIGIT(*line)) line ++;
	while(ISBLANK(*line)) line ++;
	// 1999 or 15:34
	i = atoi(line);
	while(ISDIGIT(*line)) line ++;
	if(*line == ':'){
		tmp.tm_hour = i;
		line ++;
		tmp.tm_min = atoi(line);
		while(ISDIGIT(*line)) line ++;
		// set the right year
		time_t curr = ::time(NULL);
		struct tm curr_tm;
		gmtime_r(&curr, &curr_tm);
		tmp.tm_year = curr_tm.tm_year;
		time = mktime(&tmp);
		if(time > curr){
			tmp.tm_year = curr_tm.tm_year - 1;
			time = mktime(&tmp);
		}
	}else if(ISBLANK(*line)){
		tmp.tm_year = i - 1900;
		tmp.tm_hour = 0;
		tmp.tm_min = 0;
		time = -1;
	}else{
		return -1;
	}
	while(ISBLANK(*line)) line ++;
	
	if(type == 'l'){
		char *ptr;
		ptr = strstr(line, " -> ");
		if(ptr == NULL){
			return -1;
		}else{
			*ptr = '\0';
			file = StrDup(line);
			link = StrDup(ptr + 4);
		}
	}else{
		file = StrDup(line);
	}

	if(type == '-'){
		type = 'f';
	}

	tmp.tm_sec = 0;
	tmp.tm_wday = 0;
	tmp.tm_yday = 0;
	tmp.tm_isdst = 0;
	if(time == -1){
		time = mktime(&tmp);
		if(time == -1) return -1;
	}

	return 0;
} // end of process_unix

int TFtpParser::parse(const char *line)
{
	assert(line != NULL);

	char *ptr;
	int len;
	int ret;
	
	reset();
	if(strncasecmp(line, "total", 5) == 0){
		return -1;
	}

	len = strlen(line);
	if(len < 10) return -1;
	ptr = StrDup(line);
	if(ptr[len - 1] == '\n'){
		ptr[len - 1] = '\0';
	}else if(ptr[len - 1] == '\r'){
		ptr[len - 1] = '\0';
	}
	if(ptr[len - 2] == '\r'){
		ptr[len - 2] = '\0';
	}

	if(ISDIGIT(*ptr)){
		ret = process_dos(ptr);
	}else{
		ret = process_unix(ptr);
	}

	delete[] ptr;

	return ret;
} // end of parse

}
