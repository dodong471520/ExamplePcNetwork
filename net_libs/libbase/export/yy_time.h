#ifndef _TIME_H_
#define _TIME_H_

#include "yy_macros.h"
#include "FastDelegate.h"
#include <string>

NS_YY_BEGIN

//get micro seconds from Epoch
UI64 TimeStamp();

//get current formatted time
std::string StrTimeStamp();


class Timer
{
private:
	typedef fastdelegate::FastDelegate0<bool> OnTimeCallback;
	struct TimerInfo
	{
		OnTimeCallback func;		//定时回调函数
		UI64 time_stamp;			//时间戳
		UI32 time_interval;			//时间间隔,秒级别
	};
public:
	Timer();
	~Timer();
	static Timer* getInstance();

	//register a timer, time interval以秒为单位
	UI32 registerTimer(OnTimeCallback onTime, UI32 time_interval);
	void unRegisterTimer(UI32 timer_id);
	void run();
private:
	TimerInfo m_timers[10];
};



NS_YY_END

#endif