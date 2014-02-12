// LOG.cpp: implementation of the LOG class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LOG.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LOG::LOG()
{
	m_bInit = true;
	SetOpt(true,true,true,true,true);
	m_idx = 0;

	m_map_log_level[""];
	set_log_level("net_lib_log", true, 0x00ff0000);
}

LOG::~LOG()
{

}



bool LOG::BeginLog(const char *name)
{
	char buf[128],tmpTime[32],tmpDate[32];
	_strtime(tmpTime);
	_strdate(tmpDate);

	sprintf(buf,"Log File Create At [%s %s]\r\n",tmpDate,tmpTime);
	m_file.open(name,ios::out);
	if(!m_file.good()) return false;
	m_bInit = true;
	m_file.write(buf,strlen(buf));
	m_file.flush();
	return true;
}

void LOG::SetOpt(bool file, bool print, bool debugstr,bool time,bool idx)
{
	m_bFile = file;
	m_bPrint = print;
	m_bDebugString = debugstr;
	m_bTime = time;
	m_bIdx = idx;
}

void LOG::ADD_Log(const char* strLevel, const char *format,...)
{
	MAP_LOG_LEVEL::iterator it;
	if(strLevel)
		it	= m_map_log_level.find(strLevel);
	else
		it	= m_map_log_level.find("");;
	if(it==m_map_log_level.end())
		return;
	if(!it->second.bEnable)
		return;

	if (!m_bInit) return;
	m_idx++;

	int pos = 0;
	if(m_bIdx)
	{
		pos += sprintf(m_buf+pos,"[%5d]",m_idx);
	}
	if(m_bTime)
	{
		char tmpTime[32];
		_strtime(tmpTime);
		pos += sprintf(m_buf+pos,"[%s]",tmpTime);
	}

	va_list args;
	va_start(args, format);
	pos += VSNPRINTF(m_buf+pos,DEFAULT_LOG_BUF_SIZE-50,format, args);
	m_buf[pos]	 = '\r';
	m_buf[pos+1] = '\n';
	m_buf[pos+2] = '\0';
#ifdef WIN32
	if(m_bDebugString) OutputDebugString(m_buf);
#endif
	if(m_bPrint) printf("%s",m_buf);
	if(m_bFile)
	{
		m_file.write(m_buf,pos+2);
		m_file.flush();
	}
	va_end(args);
	on_add_log(m_buf, &it->second);
}


void LOG::set_log_level(const char *strLevel, bool bEnable, DWORD color)
{
	//LOG_LEVEL	log_level;
	//log_level.bEnable			= bEnable;
	//log_level.text_color		= color;
	LOG_LEVEL*	pll = &m_map_log_level[strLevel];
	pll->bEnable	= bEnable;
	if(color!=0xffffffff)
		pll->text_color	= color;
}

void LOG::on_add_log(const char *strLog, LOG_LEVEL *pLogLevel)
{

}
