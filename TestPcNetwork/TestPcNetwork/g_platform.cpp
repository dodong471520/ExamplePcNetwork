#include "g_platform.h"

void Sys_Log( char *format,... )
{
	va_list args;
	va_start(args,format);
	char buf[256];
	VSNPRINTF(buf,255,format,args);
	printf("%s\r\n",buf);
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
