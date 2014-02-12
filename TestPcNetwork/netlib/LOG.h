// LOG.h: interface for the LOG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOG_H__64F0C070_1FFF_4FC4_877B_675E08C94ED8__INCLUDED_)
#define AFX_LOG_H__64F0C070_1FFF_4FC4_877B_675E08C94ED8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//using namespace std;
#include <fstream>

const int DEFAULT_LOG_BUF_SIZE = 1024;
struct	LOG_LEVEL
{
	bool		bEnable;
	DWORD		text_color;
public:
	LOG_LEVEL()
	{
		bEnable	= true;
		text_color	= 0;
	}
};
typedef	std::map<std::string, LOG_LEVEL>	MAP_LOG_LEVEL;
class LOG  
{
public:
	void set_log_level(const char* strLevel, bool bEnable=TRUE, DWORD color=0xffffffff);
	LOG();
	virtual ~LOG();

	void ADD_Log(const char* strLevel, const char *format,...);
	void SetOpt(bool file,bool print,bool debugstr,bool time,bool idx);
	bool BeginLog(const char *name);

private:
	std::fstream m_file;
	bool m_bFile;
	bool m_bPrint;
	bool m_bDebugString;
	bool m_bTime;
	bool m_bIdx;
	bool m_bInit;

	int   m_idx;
	char  m_buf[DEFAULT_LOG_BUF_SIZE];
	MAP_LOG_LEVEL	m_map_log_level;
protected:
	virtual void on_add_log(const char* strLog, LOG_LEVEL* pLogLevel=NULL);
};

#endif // !defined(AFX_LOG_H__64F0C070_1FFF_4FC4_877B_675E08C94ED8__INCLUDED_)
