#ifndef _LOCK_H_
#define _LOCK_H_

#include "yy_macros.h"
#include <windows.h>
#include "yy_exception.h"

NS_YY_BEGIN


/*同步
线程锁
win32: 临界区, 互斥体, 读写锁
posix: 互斥体。。。


进程锁
win32: 互斥体
posix: 互斥体
IPC(命名管道，共享内存)会用到进程锁

*/


class ILock
{
public:
	virtual void lock()=0;
	virtual void unlock()=0;
};

class AutoLock
{
public:
	AutoLock(ILock* pLock)
	{
		if(NULL == pLock)
			THROWNEXCEPT("param is null.");

		m_lock=pLock;
		m_lock->lock();
	}

	~AutoLock()
	{
		m_lock->unlock();
	}
private:
	ILock* m_lock;
};


//用户模式下的线程同步:关键段
class CSLock : public ILock
{
public:
	CSLock()
	{
		InitializeCriticalSection(&cs_handle_);
	}
	~CSLock()
	{
		DeleteCriticalSection(&cs_handle_);
	}

	void lock()
	{
		EnterCriticalSection(&cs_handle_);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs_handle_);
	}

private:
	CRITICAL_SECTION cs_handle_;
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
};


NS_YY_END
#endif