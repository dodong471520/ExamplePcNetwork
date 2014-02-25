#ifndef		_DUMP_H_
#define		_DUMP_H_

#include "yy_macros.h"
#include "yy_utility.h"

#include <shlwapi.h>
#include <tchar.h>
#include <string>
#include <stdlib.h>


using namespace std;
#pragma comment(lib, "dbghelp.lib")

TCHAR	m_acDumpName[MAX_PATH*2] = { _T("\0") };


#include <Windows.h>
#include <imagehlp.h>


NS_YY_BEGIN

/*
	在main中调用Dump::getInstance();
*/

class Dump
{
public:
	static Dump& getInstance();
	~Dump();

	// 默认为 执行档名字.dmp
	void	SetDumpFileName(const TCHAR* dumpName);

protected:
	static	void	CreateMiniDump( EXCEPTION_POINTERS* pep );

	static	LONG __stdcall snUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

	static	BOOL CALLBACK MiniDumpCallback( 
		PVOID pParam,
		const PMINIDUMP_CALLBACK_INPUT pInput, 
		PMINIDUMP_CALLBACK_OUTPUT	   pOutput );

private:
	Dump();
	Dump(const Dump& rhs){}
private:
	LPTOP_LEVEL_EXCEPTION_FILTER	m_previousFilter;
};

TCHAR*		snFindLastOfChar( const TCHAR* pSrc, const TCHAR* keyChar )
{
	assert( NULL != pSrc && NULL != keyChar );
	size_t	nKeyLen = _tcslen(keyChar);
	const	TCHAR*	pFindPos = NULL;
	const	TCHAR*	pLabor	 = pSrc;
	while ( ( pLabor = _tcsstr(pLabor, keyChar) ) != NULL ) {
		pLabor += nKeyLen;
		pFindPos = pLabor;
	}

	return	const_cast<TCHAR*>( pFindPos );
}

bool IsDataSectionNeeded( const TCHAR* pModuleName ) 
{
	if( NULL == pModuleName ) {
		return false;
	}

	TCHAR szFileName[MAX_PATH] = { _T("\0") };

	_tsplitpath_s( pModuleName, NULL, 0, NULL, 0, szFileName, ARRAYSIZE(szFileName), NULL, 0 );

	return ( 0 == _tcsicmp( szFileName, _T("ntdll") )  ? true : false );
}

Dump&		Dump::getInstance( void )
{
	static	Dump	app;
	return app;
}

Dump::~Dump(void)
{
	if ( NULL != m_previousFilter )
	{
		SetUnhandledExceptionFilter( m_previousFilter );
		m_previousFilter = NULL;
	}
}

LONG __stdcall Dump::snUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	Dump::CreateMiniDump( pExceptionInfo );

	exit( pExceptionInfo->ExceptionRecord->ExceptionCode  ); 

	return EXCEPTION_EXECUTE_HANDLER;
}

BOOL CALLBACK Dump::MiniDumpCallback(	PVOID                            pParam, 
									  const PMINIDUMP_CALLBACK_INPUT   pInput, 
									  PMINIDUMP_CALLBACK_OUTPUT        pOutput 
									  ) 
{
	if( 0 == pInput|| 0 == pOutput) return FALSE; 
	switch( pInput->CallbackType ) 
	{
	case ModuleCallback: 

		//如果要包含堆内容
		if( pOutput->ModuleWriteFlags & ModuleWriteDataSeg )
		{
			const wchar_t* val= pInput->Module.FullPath;
			std::string tmp = WstrToAnsi(val);
			if( !IsDataSectionNeeded( tmp.c_str() ) ) 
			{
				pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); 
			}
		}
		// fall through
	case IncludeModuleCallback: 
	case IncludeThreadCallback: 
	case ThreadCallback: 
	case ThreadExCallback: 
		return TRUE; 
	default:;
	}
	return FALSE; 
}

void Dump::CreateMiniDump( EXCEPTION_POINTERS* pep ) 
{
	/*
	HANDLE hFile = CreateFile( m_acDumpName,
		GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	*/

	HANDLE hFile = CreateFile( m_acDumpName,
		GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS,  MiniDumpNormal, NULL );

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei; 
		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pep; 
		mdei.ClientPointers     = FALSE; 
		MINIDUMP_CALLBACK_INFORMATION mci; 
		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)(Dump::MiniDumpCallback);
		mci.CallbackParam       = 0;

		//保存所有内存内容
		//MINIDUMP_TYPE mdt = MiniDumpNormal;
		MINIDUMP_TYPE mdt = MiniDumpWithFullMemory;

		BOOL bOK = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci );

		CloseHandle( hFile );
	}
}

Dump::Dump(void)
{
	::GetModuleFileName( 0, m_acDumpName, ARRAYSIZE(m_acDumpName) );

	bool	bModified = false;
	TCHAR* pszDot = snFindLastOfChar( m_acDumpName, _T(".") );
	if ( pszDot )
	{
		if ( _tcslen(pszDot) >= 3 ) 
		{
			bModified = true;
			_tcscpy_s( pszDot, sizeof(_T("dmp"))/sizeof(TCHAR) +1, _T("dmp") );
		}
	}

	if ( !bModified )
	{
		_tcscat_s( m_acDumpName, _T(".dmp") );
	}

	m_previousFilter = ::SetUnhandledExceptionFilter( Dump::snUnhandledExceptionFilter );
}

void	Dump::SetDumpFileName( const TCHAR* dumpName )
{
	if ( NULL != dumpName )
	{
		_tcscpy_s( m_acDumpName, ARRAYSIZE(m_acDumpName), dumpName );
	}
}


NS_YY_END

#endif