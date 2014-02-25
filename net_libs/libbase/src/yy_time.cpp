#include "yy_time.h"
#include "yy_exception.h"
#include "yy_log.h"
#include <time.h>

NS_YY_BEGIN


static const int kMicroSecondsPerSecond = 1000 * 1000;

#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	#include <Windows.h>
	struct timezone
	{
		int tz_minuteswest;
		int tz_dsttime;
	};

	const UI64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;

	int gettimeofday(struct timeval *tv, struct timezone*)
	{
		FILETIME ft;
		UI64 tmpres = 0;
		memset(&ft,0, sizeof(ft));
		GetSystemTimeAsFileTime(&ft);

		tmpres = ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS; 
		tv->tv_sec = (__int32)(tmpres*0.000001);
		tv->tv_usec =(tmpres%1000000);


		return 0;
	}
#endif

UI64 TimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	UI64 seconds = tv.tv_sec;
	return seconds * kMicroSecondsPerSecond + tv.tv_usec;
}

std::string StrTimeStamp()
{
	char buf[100] = {0};
	UI64 current_time=TimeStamp();
	time_t seconds = static_cast<time_t>(current_time / kMicroSecondsPerSecond);
	int microseconds = static_cast<int>(current_time % kMicroSecondsPerSecond);
	struct tm tm_time;
	localtime_s(&tm_time, &seconds);

	sprintf_s(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
	  tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
	  tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
	  microseconds);
	return std::string(buf, strlen(buf));
}



Timer::Timer()
{
	memset(m_timers, 0, sizeof(m_timers));
}

Timer::~Timer()
{}

Timer* Timer::getInstance()
{
	static Timer s_timer;
	return &s_timer;
}

UI32 Timer::registerTimer(OnTimeCallback onTime, UI32 time_interval)
{
	if(!onTime || 0 == time_interval)
		THROWNEXCEPT("parameter error.");

	int index=0;
	TimerInfo* timer_info=NULL;

	for(int i=0; i<ARRAY_LEN(m_timers); i++)
	{
		if(!m_timers[i].func)
		{
			timer_info=&m_timers[i];
			index=i;
		}
	}

	timer_info->func=onTime;
	timer_info->time_stamp=TimeStamp();
	timer_info->time_interval=time_interval;
	return index;
}

void Timer::unRegisterTimer(UI32 timer_id)
{
	memset(&m_timers[timer_id], 0, sizeof(TimerInfo));
}

void Timer::run()
{
	static UI64 last_time_stamp=TimeStamp();
	UI64 time_stamp=TimeStamp();

	//定时器精度为1秒
	if(1000*1000 > time_stamp-last_time_stamp)
	{
		return;
	}

	for(int i=0; i<ARRAY_LEN(m_timers); i++)
	{
		if(!m_timers[i].func || 0 == m_timers[i].time_stamp)
			continue;

		//默认参数为0，则每帧跑定时器
		/*if(m_timers[i].time_interval==0)	
		{
			if(!m_timers[i].func())
				unRegisterTimer(i);

			continue;
		}*/

		if(m_timers[i].time_interval*1000*1000 <= time_stamp-m_timers[i].time_stamp)
		{
			if(!m_timers[i].func())
			{
				unRegisterTimer(i);
				continue;
			}

			m_timers[i].time_stamp=time_stamp;
		}
	}
}

NS_YY_END
