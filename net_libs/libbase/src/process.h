#ifndef _PROCESS_HPP_
#define _PROCESS_HPP_

#include <windows.h>

namespace base
{
	void StartProcess(const char* exe_path);
	void StopProcess(const char* exe_path);
	DWORD FindProcess(const char* exe_path);
	void RestartProcess(const char* exe_path);
}


#endif