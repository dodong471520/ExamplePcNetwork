#include "yy_utility.h"
#include "yy_exception.h"
#include "yy_memory.h"

#include <stdlib.h>
#include <algorithm>
#include <Windows.h>
#include <wchar.h>


NS_YY_BEGIN

std::string IntToString(int val)
{
	// integer MAX : 4294967295L
	char buf[16] = {'\0'};
	_itoa_s(val, buf, sizeof(buf)-1, 10L);
	buf[15] = 0;
	return std::string(buf);
}

std::string Int64ToString(SI64 val)
{
	// integer_64 MAX : 18446744073709551615L
	char buf[32] = {'\0'};
	_i64toa_s(val, buf, sizeof(buf)-1, 10L);
	buf[31] = 0;
	return std::string(buf);
}

std::string FloatToString(float val)
{
	char buf[32] = {'\0'};
	sprintf_s( buf, sizeof(buf), "%f", val);
	buf[31] = 0;
	return std::string(buf);
}


int StringToInt(const char * pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return (::atoi(pVal));
}

UI64	StringToInt64(const char * pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return ( ::_atoi64(pVal));
}

double	StringToFloat(const char* pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return ( ::atof(pVal) );
}

int		WideStrToInt(const wchar_t * pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return (::_wtoi(pVal));
}

UI64	WideStrToInt64(const wchar_t * pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return (::_wtoi64(pVal));
}

float	WideStrToFloat(const wchar_t * pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return ((float)::_wtof(pVal));
}

std::wstring	IntToWideStr( int val )
{
	wchar_t buf[32] = {L"\0"};
	_itow_s(val,buf,32,10 );
	return std::wstring(buf);
}

std::wstring	Int64ToWideStr(SI64 val)
{
	wchar_t buf[32] = {L"\0"};
	_i64tow_s(val,buf,32,10 );
	return std::wstring(buf);
}

std::wstring	FloatToWideStr(float val)
{
	wchar_t buf[32] = {L"\0"};
	wsprintfW(buf, L"%f", val);
	return std::wstring(buf);	
}

std::wstring StringToWstr(const char *pVal, unsigned int codepage)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	int len;
	wchar_t *strUnicode;

	/*
	int MultiByteToWideChar(
	UINT CodePage,         // code page
	DWORD dwFlags,         // character-type options
	LPCSTR lpMultiByteStr, // string to map
	int cbMultiByte,       // number of bytes in string
	LPWSTR lpWideCharStr,  // wide-character buffer
	int cchWideChar        // size of buffer
	);

	*/
	//get unicode length
	len = MultiByteToWideChar(codepage, 0,pVal, -1, NULL,0);
	if (len <1)
	{
		/*
		ERROR_INSUFFICIENT_BUFFER. A supplied buffer size was not large enough, or it was incorrectly set to NULL. 
		ERROR_INVALID_FLAGS. The values supplied for flags were not valid. 
		ERROR_INVALID_PARAMETER. Any of the parameter values was invalid. 
		ERROR_NO_UNICODE_TRANSLATION. Invalid Unicode was found in a string. 
		*/
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	//gb2312 to unicode
	strUnicode = (wchar_t*)MPAlloc(sizeof(wchar_t) * len);
	len = MultiByteToWideChar(codepage, 0, pVal, -1, strUnicode, len);
	if (len<1)
	{
		delete []strUnicode;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	std::wstring wstr(strUnicode, len);
	delete []strUnicode;
	return wstr;
}

std::string WstrToString(const wchar_t* pVal, unsigned int codepage)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	int len;
	char *str;

	/*
	int WideCharToMultiByte(
	UINT CodePage,            // code page
	DWORD dwFlags,            // performance and mapping flags
	LPCWSTR lpWideCharStr,    // wide-character string
	int cchWideChar,          // number of chars in string.
	LPSTR lpMultiByteStr,     // buffer for new string
	int cbMultiByte,          // size of buffer
	LPCSTR lpDefaultChar,     // default for unmappable chars
	LPBOOL lpUsedDefaultChar  // set when default char used
	);
	*/
	//utf8 length
	len = WideCharToMultiByte(codepage, 0, pVal, -1, NULL, 0, NULL, NULL);
	if (len<1)
	{
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	str = (char*)MPAlloc(sizeof(char) * len);

	//unicode convert to utf8
	len = WideCharToMultiByte (codepage, 0, pVal, -1, str, len, NULL,NULL);
	if (len<1)
	{
		delete []str;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	std::string strRet(str, len);
	delete []str;

	return strRet;
}


std::wstring	AnsiToWstr(const char* pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return StringToWstr(pVal, CP_ACP);
}

std::wstring	Utf8ToWstr(const char* pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return StringToWstr(pVal, CP_UTF8);
}

std::string		WstrToAnsi(const wchar_t* pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return WstrToString(pVal, CP_ACP);
}

std::string		WstrToUtf8(const wchar_t* pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");

	return WstrToString(pVal, CP_UTF8);
}

std::string AnsiToUtf8(const char *pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");


	int len;
	wchar_t *strUnicode=NULL;
	char *str=NULL;

	/*
	int MultiByteToWideChar(
	UINT CodePage,         // code page
	DWORD dwFlags,         // character-type options
	LPCSTR lpMultiByteStr, // string to map
	int cbMultiByte,       // number of bytes in string
	LPWSTR lpWideCharStr,  // wide-character buffer
	int cchWideChar        // size of buffer
	);

	*/
	//get unicode length
	len = MultiByteToWideChar(CP_ACP, 0,pVal, -1, NULL,0);
	if (len <1)
		THROWNEXCEPT("errno:%d",  GetLastError());

	//gb2312 to unicode
	strUnicode = (wchar_t*)MPAlloc(sizeof(wchar_t) * len);
	len = MultiByteToWideChar(CP_ACP, 0, pVal, -1, strUnicode, len);
	if (len<1)
	{
		delete []strUnicode;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	/*
	int WideCharToMultiByte(
	UINT CodePage,            // code page
	DWORD dwFlags,            // performance and mapping flags
	LPCWSTR lpWideCharStr,    // wide-character string
	int cchWideChar,          // number of chars in string.
	LPSTR lpMultiByteStr,     // buffer for new string
	int cbMultiByte,          // size of buffer
	LPCSTR lpDefaultChar,     // default for unmappable chars
	LPBOOL lpUsedDefaultChar  // set when default char used
	);
	*/
	//utf8 length
	len = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	if (len<1)
	{
		delete []strUnicode;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	str = (char*)MPAlloc(sizeof(char) * len);

	//unicode convert to utf8
	len = WideCharToMultiByte (CP_UTF8, 0, strUnicode, -1, str, len, NULL,NULL);
	delete strUnicode;
	if (len<1)
	{
		delete []str;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	std::string pOut=str;
	delete []str;

	return pOut;

}

std::string Utf8ToAnsi(const char *pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");


	int len;
	wchar_t *strUnicode;
	char *str;

	len = MultiByteToWideChar(CP_UTF8, 0,pVal, -1, NULL,0);
	if (len <1)
		THROWNEXCEPT("errno:%d",  GetLastError());

	strUnicode = (wchar_t*)MPAlloc(sizeof(wchar_t) * len);

	//utf8 to unicode
	len = MultiByteToWideChar(CP_UTF8, 0, pVal, -1, strUnicode, len);
	if (len<1)
	{
		delete []strUnicode;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	//gb2312 length
	len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	if (len<1)
	{
		delete []strUnicode;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	str =(char*)MPAlloc(sizeof(char) * len);

	//unicode convert to utf8
	len = WideCharToMultiByte (CP_ACP, 0, strUnicode, -1, str, len, NULL,NULL);
	delete []strUnicode;
	if (len<1)
	{
		delete []str;
		THROWNEXCEPT("errno:%d",  GetLastError());
	}

	std::string pOut = str;

	delete []str;

	return pOut;
}


std::string SlashTrim(const char* pVal)
{
	if(NULL == pVal)
		THROWNEXCEPT("param is null.");


	char buf[2048]={0};
	size_t size=strlen(pVal);
	if(size>=sizeof(buf))
		THROWNEXCEPT("buffer is too long.");

	for(size_t i=0; i<size; i++)
	{
		char tmp=*(pVal+i);
		if('\\'==tmp)
			buf[i]='/';
		else
			buf[i]=tmp;
	}

	buf[strlen(pVal)]='\0';
	return buf;
}

std::string&	StringReplace(std::string& str, const std::string& old_value, const std::string& new_value)
{
	while(true)
	{
		std::string::size_type   pos = str.find(old_value);
		if(std::string::npos == pos)
		{
			break;
		}

		str.replace(pos,old_value.length(),new_value);
	}
	return   str;
}


bool CheckBigendien ()
{
	byte	swaptest[2] = {1,0};
	
	//如果强制转换后的值为 0x0001，则字节序为小尾顺序，这种字节序通常是高地址的值在右边，低地址的值在左边；
	//如果强制转换后的值为 0x0100，则字节序为大尾顺序，这种字节序通常是高地址的值在左边，低地址的值在右边；
	if ( *(short *)swaptest == 1)
		return false;
	else
		return true;
}

NS_YY_END