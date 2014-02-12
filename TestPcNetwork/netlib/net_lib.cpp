#include "stdafx.h"
#include "net_lib.h"
#include "net.h"
#include "tcp.h"
//#include "udp.h"

#include "log.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LOG*	g_ptr_net_lib_log	= NULL;


void Sys_Log(const char* strLevel, const char *format,...)
{
	va_list args;
	va_start(args, format);
	char buf[256];
	VSNPRINTF(buf,255,format,args);
	if(g_ptr_net_lib_log)
		g_ptr_net_lib_log->ADD_Log(strLevel, buf);
	else
		printf("%s\r\n",buf);
	/* write to file */
	va_end(args);

}


unsigned long Sys_GetTime()
{
#ifdef WIN32
	return GetTickCount();
#else 
	/* linux */
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000+tv.tv_usec/1000);
#endif
}