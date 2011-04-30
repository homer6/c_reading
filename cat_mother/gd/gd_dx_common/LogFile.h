#ifndef _LOGFILE_H
#define _LOGFILE_H


#include <stdio.h>


/**
 * Simple log file output.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LogFile
{
public:
	explicit LogFile( const char* name );
	~LogFile();

	void	printf( const char* fmt, ... );

private:
	FILE* m_file;

	LogFile( const LogFile& );
	LogFile& operator=( const LogFile& );
};


#endif // _LOGFILE_H
