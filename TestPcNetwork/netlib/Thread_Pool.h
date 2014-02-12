// Thread_Pool.h: interface for the Thread_Pool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_POOL_H__D7C1BC9F_BD5E_4590_9B07_5DE5DEEA5364__INCLUDED_)
#define AFX_THREAD_POOL_H__D7C1BC9F_BD5E_4590_9B07_5DE5DEEA5364__INCLUDED_

//#include "net_lib.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define THREAD_POOL_CMD_NULL		0
#define THREAD_POOL_CMD_SHUTDOWN	1

#define THREAD_END(pTPD) if(pTPD){if(pTPD->get_Thread_Pool()){pTPD->get_Thread_Pool()->CloseThread(pTPD->get_name(),false);}delete(pTPD);}
//class Thread_Pool;

//�̲߳����͹���ӿ�
struct NET_LIB_API THREAD_PARAM_DATA
{
	friend Thread_Pool;
public:
	void*				ptr_arg;		//�̲߳����������
	uint16				command;		//�߳�����

	THREAD_PARAM_DATA(Thread_Pool* pPool);
	~THREAD_PARAM_DATA();

	Thread_Pool*	get_Thread_Pool()	{return ptr_pool;}
	const char*		get_name()			{return thread_name.c_str();}

	void set_name(const char* strName)	{thread_name	= strName;}

	//	bool Try_Lock();
	void Lock();
	void Unlock();
	//	bool Try_Lock_Runing();
	void Lock_Runing();
	void Unlock_Runing();
protected:
	std::string			thread_name;	//�߳�����
	TYPE_CS				lock_run;		//�߳������������̳߳�ʼ��ʱҪ����������ʱҪ����
	TYPE_CS				lock_cmd;		//������������
	Thread_Pool*		ptr_pool;		//�̳߳�ָ��
};

//�̳߳��࣬�����߳�
class NET_LIB_API Thread_Pool  
{
public:
	int get_Thread_count();
	void Cleanup();

	void Unlock();
	void Lock();

	void		CloseThread(const char* name, bool wait=true);						//�ú���һֱ�ȴ��߳��ͷ��������������ܻ�����������
	const char* RunThread(const char* name, PTR_THREAD_FUNC func, void* pArg);		//����������һ���µ��߳�,�����߳�����


	Thread_Pool();
	virtual ~Thread_Pool();


protected:
	std::map<std::string, THREAD_PARAM_DATA*>	m_map_threads;
	TYPE_CS		m_lock_pool;

private:
	uint32 m_unique_num;
};

#endif // !defined(AFX_THREAD_POOL_H__D7C1BC9F_BD5E_4590_9B07_5DE5DEEA5364__INCLUDED_)
