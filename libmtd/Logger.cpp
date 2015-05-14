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

#define USE_LOGGER_H
#include "matrix.h"

#define BUFSIZE		4096

namespace matrix {

pthread_mutex_t LogLock = PTHREAD_MUTEX_INITIALIZER;
FILE *Logger::logFile = NULL;
int Logger::logLevel = 0;
time_t curTime;

void Logger::open(const char *file, int level)
{
	logLevel = level;

	if (logFile)
		closeLog();

	bool b = (file && *file);

	if(b) logFile = fopen(file, "a+");
	if(!logFile) 
	{
		logFile = stderr;
		if (b) LOG(1)("Error open log file: %s\n", file);
	}
}

void Logger::closeLog()
{
	if (logFile && logFile != stderr) 
	{
		fclose(logFile);
		logFile = NULL;
	}
}

void Logger::print(const char *fmt, ...)
{
	pthread_mutex_lock(&LogLock);

   	char ct[128];
	struct tm *tm = localtime(&curTime);
	strftime(ct, 127, "%Y/%m/%d %H:%M:%S", tm);
	
	char logbuf[BUFSIZE];
	va_list args;
	va_start(args, fmt);
	snprintf(logbuf, sizeof(logbuf), "%s| %s", ct, fmt);
	vfprintf(logFile, logbuf, args);
	fflush(logFile);
	va_end(args);

	pthread_mutex_unlock(&LogLock);

}

}
