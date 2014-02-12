// cfg.h: interface for the CFG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFG_H__D6C2FF43_6DEF_431C_9611_C834B630AD58__INCLUDED_)
#define AFX_CFG_H__D6C2FF43_6DEF_431C_9611_C834B630AD58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
using namespace std;
#include <fstream>

#define MAX_CFG_LINE_LEN	2048
//≈‰÷√∂¡–¥¥¶¿Ì
class CFG  
{
public:
	float GetFloat( LPCSTR szSection, LPCSTR szName, LPCSTR szDefault = "0.0f" );
	long  GetLong( LPCSTR szSection, LPCSTR szName, LPCSTR szDefault = "0" );
	LPCSTR GetString( LPCSTR szSection, LPCSTR szName, LPCSTR szDefault = "" );
	DWORD GetHex( LPCSTR szSection, LPCSTR szName, LPCSTR szDefault = "0" );
	UINT  GetBinary( LPCSTR lpszSection, LPCSTR lpszEntry, LPBYTE pData, UINT nBytes );
	BOOL  GetBOOL( LPCSTR szSection, LPCSTR szName, LPCSTR szDefault = "false" );


	bool save(const char* str_file=NULL);
	void close();
	bool open(const char* str_file);
	CFG();
	virtual ~CFG();



protected:
	std::string		m_name;
	typedef std::map<std::string, std::string> MAP_OPTION;
	//	MAP_OPTION	m_map_option;
	typedef std::map<std::string, MAP_OPTION> MAP_GROUP;
	MAP_GROUP		m_map_group;
};

#endif // !defined(AFX_CFG_H__D6C2FF43_6DEF_431C_9611_C834B630AD58__INCLUDED_)
