/************************************************************************/
/*
@author:  junliang
@brief:   macros
@time:    20130401
*/
/************************************************************************/
#ifndef _MACROS_H_
#define _MACROS_H__

//support os
#define YY_PLATFORM_WIN32				1
#define YY_PLATFORM_LINUX				2

//determine current os
#if defined (WIN32)
	//#define _CRT_SECURE_NO_WARNINGS			//cancel fopen_s, sprintf_s warn
	#define YY_TARGET_PLATFORM				YY_PLATFORM_WIN32
#else
	#define YY_TARGET_PLATFORM				YY_PLATFORM_LINUX
#endif


#define	UI64_MAX						0xFFFFFFFFFFFFFFFF
#define	UI32_MAX						0xFFFFFFFF
#define UI16_MAX						0xFFFF
#define UI08_MAX						0xFF

//use do ... while(0), 防止外面调用的时候if()SAFE_DELETE; else... 方式
#define SAFE_DELETE(p)					do{if(p){::delete(p); (p)=0;}}while(0)
#define SAFE_DELETE_ARRAY(p)			do{if(p){::delete[](p); p=0;}}while(0)
#define SAFE_FREE(p)					do{if(p) { free(p); (p) = 0; } }while(0)
#define SAFE_RELEASE(p)					do{if(p) { (p)->release(); } }while(0)
#define SAFE_RELEASE_NULL(p)			do{if(p) { (p)->release(); (p) = 0; } }while(0)
#define SAFE_RETAIN(p)					do{if(p) { (p)->retain(); } }while(0)

#define ARRAY_LEN(array)				sizeof(array)/sizeof(array[0])


typedef unsigned char					UI08;
typedef unsigned short					UI16;
typedef unsigned int					UI32;
typedef char							SI08;
typedef short							SI16;
typedef int								SI32;
typedef long long						SI64;
typedef unsigned long long				UI64;

// printf: SI64--%lld, UI64--%llu

#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	#define SLASH '/'
	//typedef unsigned __int64			UI64;
	//typedef signed __int64				SI64;
#endif

#if (YY_TARGET_PLATFORM==YY_PLATFORM_LINUX)
	#define SLASH '\\'
	//#include <stdint.h>
	//typedef uint64_t					UI64;
	//typedef int64_t						SI64;
#endif


// namespace yy {}
#ifdef __cplusplus
	#define NS_YY_BEGIN					namespace yy {
	#define NS_YY_END					}
	#define USING_NS_YY					using namespace yy;
#else
	#define NS_YY_BEGIN 
	#define NS_YY_END 
	#define USING_NS_YY 
#endif 


/* Define NULL pointer value */
#ifndef NULL
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif

#endif