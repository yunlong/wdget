#ifndef __FTPPARSER_H__
#define __FTPPARSER_H__

/*
ftpparse tries to parse one line of LIST output.
The line is an array of len characters stored in buf.
It should not include the terminating CR LF; so buf[len] is typically CR.

If parse() can't find a filename, it returns 0.

If parse() can find a filename, it fills in fp and returns 1.
fp is a struct ftpparse, defined below.
The name is an array of fp.namelen characters stored in fp.name;
fp.name points somewhere within buf.
*/

namespace matrix {

class TFtpParser
{
private:
	void reset(void);
	int process_dos(char *line);
	int process_unix(char *line);

private:
	const char *file;
	const char *link;
	off_t size;
	time_t time;
	char type;

public:
	TFtpParser();
	~TFtpParser();

	const char *get_file(void);
	const char *get_link(void);
	off_t get_size(void);
	time_t get_time(void);
	char get_type(void);

	int parse(const char *line);

};

}

#endif 
