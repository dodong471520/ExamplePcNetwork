#ifndef _m_queueH_
#define _m_queueH_

#include "yy_memory.h"
#include "yy_exception.h"
#include <string.h>
#include <Windows.h>
#include <stdlib.h>


NS_YY_BEGIN

//线程安全， 浅拷贝， 线性消息队列，可优化成环形队列
template<class T>
class MsgQueue
{
public:
	MsgQueue(size_t max_size=1024);
	~MsgQueue();

	void append(const T* data, DWORD timeout=0);
	void remove(T* data, DWORD dwTimeout=0);
private:
	HANDLE m_handles[2];			//[0]: mutex,互斥体内核对象; [1]: semaphone，信号量内核对象
	size_t m_max_size;				//最多容纳元素的大小
	T* m_queue;						//存放元素的队列指针
};


template<class T>
MsgQueue<T>::MsgQueue(size_t max_size)
{
	m_max_size=max_size;
	m_queue=(T*)MPAlloc(m_max_size*sizeof(T));

	//创建互斥体
	m_handles[0]=CreateMutex(NULL, false, NULL);

	//创建信号量
	m_handles[1]=CreateSemaphore(NULL, 0, (long)m_max_size, NULL);
}

template<class T>
MsgQueue<T>::~MsgQueue()
{
	SAFE_DELETE_ARRAY(m_queue);
}

template<class T>
void MsgQueue<T>::append(const T* data, DWORD timeout)
{
	DWORD dw=WaitForSingleObject(m_handles[0], timeout);
	if(dw==WAIT_OBJECT_0)
	{
		//增加信号量的当前资源计数
		LONG prev_count;
		if(FALSE == ReleaseSemaphore(m_handles[1], 1, &prev_count))
		{
			ReleaseMutex(m_handles[0]);

			//如果有最大值限制
			THROWNEXCEPT("append to queue error, the queue is full");
		}

		memcpy(&m_queue[prev_count], data, sizeof(T));
		ReleaseMutex(m_handles[0]);
	}
	else
	{
		//timeout
		THROWNEXCEPT("append to queue timeout.");
	}
}

template<class T>
void MsgQueue<T>::remove(T* data, DWORD dwTimeout)
{
	DWORD dw=WaitForMultipleObjects(_countof(m_handles), m_handles, TRUE, dwTimeout);
	if(dw==WAIT_OBJECT_0)
	{
		memcpy(data, &m_queue[0], sizeof(T));

		//按字节自拷贝
		memmove(&m_queue[0], &m_queue[1], sizeof(T)*(m_max_size-1));
		ReleaseMutex(m_handles[0]);
	}
	else
	{
		//timeout
		THROWNEXCEPT("remove from queue timeout");
	}
}

NS_YY_END
#endif