#include "yy_log.h"
#include "yy_time.h"
#include "yy_memory.h"
#include "yy_utility.h"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <windows.h>



NS_YY_BEGIN

#define MAX_LOG_LINE 100000

Log::Log():m_log_level(Info), m_err_log_file(NULL), m_all_log_file(NULL), m_log_line(0)
{
	m_thread.bind(fastdelegate::MakeDelegate(this, &Log::runInThread));
	//m_thread.open(1);
}

Log::~Log()
{
	if(m_err_log_file)
		fclose(m_err_log_file);

	if(m_all_log_file)
		fclose(m_all_log_file);
}

Log* Log::getInstance()
{
	static Log s_log;
	return &s_log;
}

void Log::setLevel(LogLevel level)
{
	m_log_level = level;
}

void Log::setColor(Color color)
{
	static WORD WinColorFG[Color_count] =
	{
		0,                                                  // BLACK
		FOREGROUND_RED,                                     // RED
		FOREGROUND_GREEN,                                   // GREEN
		FOREGROUND_RED | FOREGROUND_GREEN,                  // BROWN
		FOREGROUND_BLUE,                                    // BLUE
		FOREGROUND_RED |                    FOREGROUND_BLUE,// MAGENTA
		FOREGROUND_GREEN | FOREGROUND_BLUE,                 // CYAN
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,// WHITE
		// YELLOW
		FOREGROUND_RED | FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
		// RED_BOLD
		FOREGROUND_RED |                                      FOREGROUND_INTENSITY,
		// GREEN_BOLD
		FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
		FOREGROUND_BLUE | FOREGROUND_INTENSITY,             // BLUE_BOLD
		// MAGENTA_BOLD
		FOREGROUND_RED |                    FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		// CYAN_BOLD
		FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		// WHITE_BOLD
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
	};

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, WinColorFG[color]);
}

void Log::resetColor()
{
	HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
}


void Log::logging(LogLevel level, const char * fmt, ...)
{
	AutoLock lock(&m_lock);

	//先检查日志等级
	if (level > m_log_level)
	{
		return;
	}

	char buf[MAX_LOG_SIZE-100];
	memset(buf, 0, sizeof(buf));

	va_list ap;
	va_start(ap, fmt);

	//vsprintf (buf,fmt, ap);
	//truncate,如果超出长度，则截取
	vsnprintf_s(buf, sizeof(buf)-1, _TRUNCATE, fmt, ap);
	va_end(ap);

	logData(level, buf);

	//LogMsg log_msg;
	//log_msg.level=level;
	//strcpy_s(log_msg.data, sizeof(log_msg.data), buf);
	//m_queue.append(log_msg);
}

void Log::runInThread()
{
	LogMsg log_msg;

	try
	{
		m_queue.remove(&log_msg);
	}catch(Exception& err)
	{
		//timeout
		return;
	}

	logData(log_msg.level, log_msg.data);
}

//生成log字符串
void Log::logData(LogLevel level, const char * buf)
{
	static char s_log_level[Level_count][10]=
	{
		"Error",
		"Warning",
		"Info",
		"Trace",
		"Debug"
	};

	char format_log[MAX_LOG_SIZE];
	std::string current_time=StrTimeStamp();
	sprintf_s(format_log, sizeof(format_log), "[%s] [threadID:%d][%s] %s\n", s_log_level[level],GetCurrentThreadId(),current_time.c_str(), buf);

	//如果是错误信息，则高亮显示
	if(level==Error)
	{
		setColor(GREEN);
		logToConsole(format_log);
		resetColor();

		logToErrFile(format_log);
	}
	else if(level==Warn)
	{
		setColor(CYAN);
		logToConsole(format_log);
		resetColor();

		logToErrFile(format_log);
	}
	else
		logToConsole(format_log);

	logToFile(format_log);
	m_log_line++;
	if(m_log_line>=MAX_LOG_LINE)
		m_log_line=0;
}

//log到终端
void Log::logToConsole(const char* log)
{
	//用快速的printf而不是cout
	//std::cout <<log<< std::endl;
	printf("%s", log);
}

//log到文件
void Log::logToFile(const char* log)
{
	if(m_log_line==0 || m_all_log_file==NULL)
	{
		if(m_all_log_file)
			fclose(m_all_log_file);
		m_all_log_file=NULL;

		//日期变了,重设文件名
		char new_log_file_name[50] = {0};
		if(!IsDirExist("log"))
			MkDir("log");

		std::string log_time=StringReplace(StrTimeStamp(), ":", "_");
		sprintf_s(new_log_file_name, sizeof(new_log_file_name), "log\\%s.log", log_time.c_str());
		fopen_s(&m_all_log_file, new_log_file_name, "a");
		if(NULL==m_all_log_file)
			THROWNEXCEPT("create file:%s error.", new_log_file_name);
	}

	size_t count=strlen(log);
	size_t write_count=fwrite(log, sizeof(char), count, m_all_log_file);
	if(write_count!=count)
		THROWNEXCEPT("log to file error.");
}

void Log::logToErrFile(const char* log)
{
	if(m_log_line==0 || m_err_log_file==NULL)
	{
		if(m_err_log_file)
			fclose(m_err_log_file);
		m_err_log_file=NULL;

		//日期变了,重设文件名
		char new_log_file_name[50] = {0};
		if(!IsDirExist("log"))
			MkDir("log");

		std::string log_time=StringReplace(StrTimeStamp(), ":", "_");
		sprintf_s(new_log_file_name, sizeof(new_log_file_name), "log\\error_%s.log", log_time.c_str());
		fopen_s(&m_err_log_file, new_log_file_name, "a");
		if(NULL==m_err_log_file)
			THROWNEXCEPT("create file:%s error.", new_log_file_name);
	}

	size_t count=strlen(log);
	size_t write_count=fwrite(log, sizeof(char), count, m_err_log_file);
	if(write_count!=count)
		THROWNEXCEPT("log to file error.");
}



NS_YY_END