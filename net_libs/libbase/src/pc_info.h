/************************************************************************/
/*
@file:    pc_info.h
@author:  junliang
@time:    5-Jun-2012
*/
/************************************************************************/
#ifndef PC_INFO_HPP_
#define PC_INFO_HPP_

#include <windows.h>


/************************************************************************/
/*
@class:    PcInfo
@brief:    获取主机信息
*/
/************************************************************************/
class PcInfo
{
public:

	PcInfo()
	{
		GetSystemTimes( &preidleTime_, &prekernelTime_, &preuserTime_ );
	}

	__int64 CompareFileTime ( FILETIME time1, FILETIME time2 )
	{
		__int64 a = (unsigned __int64)time1.dwHighDateTime << 32 | time1.dwLowDateTime ;
		__int64 b = (unsigned __int64)time2.dwHighDateTime << 32 | time2.dwLowDateTime ;

		return   (b - a);
	}

	int cpuInUse()
	{
		BOOL res ;

		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;

		res = GetSystemTimes( &idleTime, &kernelTime, &userTime );

		__int64 idle = CompareFileTime( preidleTime_,idleTime);
		__int64 kernel = CompareFileTime( prekernelTime_, kernelTime);
		__int64 user = CompareFileTime(preuserTime_, userTime);

		__int64 cpu =(kernel +user - idle) *100/(kernel+user);
		preidleTime_ = idleTime;
		prekernelTime_ = kernelTime;
		preuserTime_ = userTime ;

		return (int)cpu;
	}

	int memInUse()
	{
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof (statex);
		GlobalMemoryStatusEx (&statex);
		return statex.dwMemoryLoad;
	}
private:
	FILETIME preidleTime_;
	FILETIME prekernelTime_;
	FILETIME preuserTime_;


};
#endif