#include "process.h"
#include <string>
//#include <tlhelp32.h>
#include "yy_file.h"
#include "yy_exception.h"

#define MAXEXELEN 512

namespace base
{
	void StartProcess(const char* exe_path)
	{
		if(!IsFileExist(exe_path))
			THROWNEXCEPT("%s is not exit.", exe_path);

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		char buf[MAXEXELEN]={0};
		strcpy_s(buf, sizeof(buf), exe_path);

		//获取文件名
		const char* path_sep_pos = strrchr(exe_path, '\\');
		int len=path_sep_pos-exe_path;
		std::string curdir(exe_path, len);
		if( !CreateProcess( NULL,	// No module name (use command line)
			buf,					// Command line,可读可写
			NULL,					// Process handle not inheritable
			NULL,					// Thread handle not inheritable
			FALSE,					// Set handle inheritance to FALSE
			CREATE_NEW_CONSOLE,		// 创建子进程基于新窗口
			NULL,					// Use parent's environment block
			curdir.c_str(),			// 指定子进程的工作目录
			&si,
			&pi )					// Pointer to PROCESS_INFORMATION structure
			)
			THROWNEXCEPT("create process:%s error.", exe_path);

		//关闭子进程的线程句柄，减少引用计数，这样子进程的主线程退出时，引用计数为0，系统可以释放主线程对象
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	void StopProcess(DWORD process_id)
	{
		HANDLE process_handle=OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id );
		if( process_handle == NULL )
			THROWNEXCEPT("open process error.");

		//异步的，所以需要waitforsingleobject
		int ret=TerminateProcess(process_handle, 0);
		if(ret==0)
			THROWNEXCEPT("terminate process:%d error num:%d", process_id, GetLastError());

		//等待进程真的退出
		DWORD dw=WaitForSingleObject(process_handle, 10000);
		switch(dw)
		{
		case WAIT_OBJECT_0:
			//ok
			break;
		case WAIT_TIMEOUT:
			THROWNEXCEPT("StopProcess timeout");
			break;
		case WAIT_FAILED:
			THROWNEXCEPT("StopProcess invalid handle");
			break;
		}

		CloseHandle(process_handle);
	}

	DWORD FindProcess(const char* exe_path)
	{
		if(!IsFileExist(exe_path))
			THROWNEXCEPT("%s is not exit.", exe_path);

		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32;

		// Take a snapshot of all processes in the system.
		hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
		if( hProcessSnap == INVALID_HANDLE_VALUE )
		{
			THROWNEXCEPT( "CreateToolhelp32Snapshot (of processes)" );
		}

		// Set the size of the structure before using it.
		pe32.dwSize = sizeof( PROCESSENTRY32 );

		// Retrieve information about the first process,
		// and exit if unsuccessful
		if( !Process32First( hProcessSnap, &pe32 ) )
		{
			THROWNEXCEPT( "Process32First error" ); // Show cause of failure
			CloseHandle( hProcessSnap );    // Must clean up
		}

		do
		{

			HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
			MODULEENTRY32 me32;

			// Take a snapshot of all modules in the specified process.
			hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pe32.th32ProcessID);
			if( hModuleSnap == INVALID_HANDLE_VALUE )
			{
				continue;
				//THROWNEXCEPT( "CreateToolhelp32Snapshot (of modules)" );
			}

			// Set the size of the structure before using it.
			me32.dwSize = sizeof( MODULEENTRY32 );

			// Retrieve information about the first module,
			// and exit if unsuccessful
			if( !Module32First( hModuleSnap, &me32 ) )
			{
				CloseHandle( hModuleSnap );    // Must clean up
				THROWNEXCEPT( "Module32First" ); // Show cause of failure
			}

			// Now walk the module list of the process,
			// and display information about each module
			do
			{
				if(strcmp(me32.szExePath, exe_path)==0)
					return pe32.th32ProcessID;

			} while( Module32Next( hModuleSnap, &me32 ) );

			CloseHandle( hModuleSnap );


		} while( Process32Next( hProcessSnap, &pe32 ) );

		return 0;
	}

	void StopProcess(const char* exe_path)
	{
		if(!IsFileExist(exe_path))
			THROWNEXCEPT("%s is not exit.", exe_path);

		DWORD process_id=FindProcess(exe_path);
		if(process_id!=0)
			StopProcess(process_id);
	}
	void RestartProcess(const char* exe_path)
	{
		if(!IsFileExist(exe_path))
			THROWNEXCEPT("%s is not exit.", exe_path);

		//如果存在，则kill
		DWORD process_id=FindProcess(exe_path);
		if(process_id!=0)
			StopProcess(exe_path);

		StartProcess(exe_path);
	}
}