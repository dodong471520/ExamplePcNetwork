#ifndef _YY_TASK_H_
#define _YY_TASK_H_


#include "YYMacros.h"
#include "YYQueue.h"
#include "FastDelegate.h"
using namespace fastdelegate;

NS_YY_BEGIN

//带队列的任务，根据是否开辟线程，分为主动任务和被动任务，参考ace的task
template<class T>
class Task
{
public:
	Task(UI32 queue_size=1024);
	~Task();

	void pushMsg(const void* msg, UI32 timeout=1000);
	void popMsg(void* msg, UI32 timeout=5000);

	virtual void runInThread();

private:
	MsgQueue<T> m_queue;
};



template<class T>
Task<T>::Task(UI32 queue_size), m_queue(queue_size)
{}

template<class T>
Task::~Task(){}


template<class T>
void Task<T>::pushMsg(const void* msg, UI32 timeout)
{
	if(NULL == msg)
		THROWNEXCEPT("msg is null.");

	queue_.append(msg, timeout);
}

template<class T>
void Task<T>::popMsg(void* msg, UI32 timeout)
{
	if(NULL == msg)
		THROWNEXCEPT("msg is null.");

	queue_.remove(msg, timeout);
}


template<class T>
void Task<T>::runInThread()
{
}


NS_YY_END
#endif