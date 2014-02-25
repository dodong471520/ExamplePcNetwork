#ifndef _THREAD_H_
#define _THREAD_H_

#include "yy_macros.h"
#include "FastDelegate.h"
#include <windows.h>

#define MAX_THRED_NUM		10
NS_YY_BEGIN


typedef fastdelegate::FastDelegate0<void> ThreadFunCallback;

/*/
MonitorTask monitor_task;
base::Thread thread(MakeDelegate(&monitor_task, &MonitorTask::run));
thread.open(1);
//*/

class Thread
{
public:
	Thread();
	Thread(ThreadFunCallback cb);
	~Thread();

public:
	void bind(ThreadFunCallback cb);
	void open(UI32 num);
	void close();

public:
	bool stop();
	void run();

private:
	HANDLE m_thread_handle[MAX_THRED_NUM];		//线程句柄
	UI32 m_thread_num;							//线程数目
	bool m_stop_flag;							//是否停止
	ThreadFunCallback m_delegate;				//函数指针
};


NS_YY_END

#endif