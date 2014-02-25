#ifndef _UTILITY_H_20130204__
#define _UTILITY_H_20130204__

#include "yy_macros.h"
#include <string>

NS_YY_BEGIN

std::string		IntToString( int Val );
std::string		Int64ToString( SI64 val );
std::string		FloatToString( float val );

int				StringToInt( const char * pVal );
UI64			StringToInt64( const char * pVal );
double			StringToFloat( const char* pVal );

int				WideStrToInt( const wchar_t * pVal );
UI64			WideStrToInt64( const wchar_t * pVal );
float			WideStrToFloat( const wchar_t * pVal );

std::wstring	IntToWideStr( int val );
std::wstring	Int64ToWideStr( SI64 val );
std::wstring	FloatToWideStr( float val );

std::wstring	AnsiToWstr(const char* pVal);
std::wstring	Utf8ToWstr(const char* pVal);
std::string		WstrToAnsi(const wchar_t* pVal);
std::string		WstrToUtf8(const wchar_t* pVal);

std::string		AnsiToUtf8(const char *pVal);
std::string		Utf8ToAnsi(const char *pVal);

//按照不同操作系统，把字符串中的slash转换成各自操作系统支持的slash
std::string		SlashTrim(const char* pVal);

std::string&	StringReplace(std::string& str, const std::string& old_value, const std::string& new_value);


//1． Little endian：将低序字节存储在起始地址
//2． Big endian：将高序字节存储在起始地址
bool			CheckBigendien();


NS_YY_END


#endif
