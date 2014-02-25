#ifndef _WIN_LOCK_H_
#define _WIN_LOCK_H_

#include "yy_macros.h"
#include <Windows.h>

NS_YY_BEGIN

//用户模式下的线程同步:关键段
class CSLock
{
public:
	CSLock()
	{
		InitializeCriticalSection(&m_cs);
	}
	~CSLock()
	{
		DeleteCriticalSection(&m_cs);
	}

	void lock()
	{
		EnterCriticalSection(&m_cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&m_cs);
	}

private:
	CRITICAL_SECTION m_cs;
};

//内核对象的线程同步:事件
class CEvent
{
public:
	CEvent(bool initial_state=false, bool manual_reset=false)
	{
		//创建事件
		event_handle_=::CreateEvent(NULL,	//安全参数
			manual_reset,					//是否自动重置
			initial_state,					//是否初始化为未触发状态
			NULL);							//名字
	}

	~CEvent()
	{
		CloseHandle(event_handle_);
	}

	//触发事件
	int setEvent()
	{
		return ::SetEvent(event_handle_);
	}

	//先触发再重置事件
	int pulseEvent() 
	{ 
		return ::PulseEvent(event_handle_); 
	}

	//重置事件
	int resetEvent() 
	{ 
		return ::ResetEvent(event_handle_); 
	}

	int wait(DWORD dwTimeOut = INFINITE) 
	{
		return ::WaitForSingleObject(event_handle_, dwTimeOut);
	}

	//判断当前事件是否被触发
	bool isSetEvent()
	{
		return (wait(0) == WAIT_OBJECT_0);
	}

private:
	HANDLE event_handle_;
};


//内核对象的线程同步:信号量

//内核对象的线程,进程同步:互斥体
class CMutex : public ILock
{
public:
    CMutex(char* name)
	{
		createMyMutex(name);
	}

    ~CMutex()
	{
		WaitForSingleObject(m_mutex, 1000);
	}
  
    void createMyMutex(char* name)
	{
		m_mutex = CreateMutexA(NULL, FALSE, name);
		if(m_mutex == NULL)  
		{  
			//create error
			THROWNEXCEPT("create mutex error.");
		}  
	}
    void lock()
	{
		DWORD dw=WaitForSingleObject(m_mutex, 5000);
		if(dw!=WAIT_OBJECT_0)
		{
			//timeout
			THROWNEXCEPT("can not lock.");
		}

	}
    void unlock()
	{
		ReleaseMutex(m_mutex);
	}
  
private:
    HANDLE m_mutex;
};*/

NS_YY_END
#endif
