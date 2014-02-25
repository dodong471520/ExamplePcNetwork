/************************************************************************/
/*
@author:  junliang
@time:    20130308
@brief:   任意类型的变量
*/
/************************************************************************/
#ifndef _VARIANT_H_
#define _VARIANT_H_

#include "YYMacros.h"
NS_YY_BEGIN


//数据类型
enum VARTYPE
{
	VARTYPE_UNKNOWN,
	VARTYPE_BOOL,
	VARTYPE_INT,
	VARTYPE_FLOAT,
	VARTYPE_STRING,
	//VARTYPE_WIDESTR,
	//VARTYPE_OBJECT,
	//VARTYPE_VOID,
	//VARTYPE_MAX,
};

#define MAX_CHAR_SIZE 100
struct Var
{
	VARTYPE type;
	union data
	{
		bool		bool_val;
		int			int_val;
		float		float_val;
		char		str_val[MAX_CHAR_SIZE];
	};

	Var():type(VARTYPE_UNKNOWN){}
};

NS_YY_END
#endif
