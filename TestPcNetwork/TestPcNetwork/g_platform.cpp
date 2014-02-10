#include "g_platform.h"

void Sys_Log( char *format,... )
{
	va_list args;
	va_start(args,format);
	char buf[256];
	vsnprintf(buf,255,format,args);
	printf("%s\r\n",buf);
	va_end(args);
}
