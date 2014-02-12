// Thread_Pool.cpp: implementation of the Thread_Pool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "net_lib.h"
#include "Thread_Pool.h"

#include <stdio.h>
#include <stdlib.h>

//bool THREAD_PARAM_DATA::init(const char* strName)
//{
//	if(ptr_pool==NULL)
//		return false;
//	thread_name	= strName;
//
//
//	return true;
//}

THREAD_PARAM_DATA::THREAD_PARAM_DATA(Thread_Pool* pPool)
{
	ptr_pool	= pPool;
	INIT_CS(&lock_cmd);
	INIT_CS(&lock_run);
}

THREAD_PARAM_DATA::~THREAD_PARAM_DATA()
{
	DELETE_CS(&lock_cmd);
	DELETE_CS(&lock_run);
}

void THREAD_PARAM_DATA::Lock()						
{
	LOCK_CS(&lock_cmd);
}

void THREAD_PARAM_DATA::Unlock()					
{
	UNLOCK_CS(&lock_cmd);
}
//bool THREAD_PARAM_DATA::Try_Lock()
//{
//	return TRY_CS(&lock_cmd);
//}

void THREAD_PARAM_DATA::Lock_Runing()				
{
	LOCK_CS(&lock_run);
}

void THREAD_PARAM_DATA::Unlock_Runing()				
{
	UNLOCK_CS(&lock_run);
}

//bool THREAD_PARAM_DATA::Try_Lock_Runing()
//{
//	return TRY_CS(&lock_run);
//}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Thread_Pool::Thread_Pool()
{
	INIT_CS(&m_lock_pool);
	m_unique_num	= 0;
}

Thread_Pool::~Thread_Pool()
{
	DELETE_CS(&m_lock_pool);
}

const char* Thread_Pool::RunThread(const char* name, PTR_THREAD_FUNC func, void* pArg)
{
	if(name == NULL)
	{
		char str[50];
		sprintf(str, "thread_%d", m_unique_num);
		m_unique_num++;
		name = str;
	}
	Lock();
	std::map<std::string, THREAD_PARAM_DATA*>::iterator it = 	m_map_threads.find(name);
	THREAD_PARAM_DATA* pTPD;
	if(it==m_map_threads.end())
	{
		pTPD	= new THREAD_PARAM_DATA(this);
		pTPD->ptr_arg	= pArg;
		pTPD->command	= THREAD_POOL_CMD_NULL;
		pTPD->set_name(name);
		m_map_threads[name]	= pTPD;
		Unlock();
		CREATE_THREAD(func, pTPD);
	}
	else
	{
		pTPD	= it->second;
		pTPD->Lock();
		pTPD->command	= THREAD_POOL_CMD_SHUTDOWN;
		pTPD->Unlock();
		pTPD->Lock_Runing();
		pTPD->command	= THREAD_POOL_CMD_NULL;
		pTPD->ptr_arg	= pArg;
		pTPD->Unlock_Runing();
		CREATE_THREAD(func, it->second);
	}

	return pTPD->get_name();
}


void Thread_Pool::CloseThread(const char *name, bool wait)
{
	Lock();
	std::map<std::string, THREAD_PARAM_DATA*>::iterator it = 	m_map_threads.find(name);
	if(it==m_map_threads.end())
	{
		Unlock();
		return ;
	}
	else
	{
		THREAD_PARAM_DATA* pTPD	= it->second;
		Unlock();
		if(pTPD==NULL)
			return;
		pTPD->Lock();
		pTPD->command	= THREAD_POOL_CMD_SHUTDOWN;
		pTPD->Unlock();
		if(wait)
		{
			pTPD->Lock_Runing();
			pTPD->Unlock_Runing();
		}
		Lock();
		it = 	m_map_threads.find(name);
		m_map_threads.erase(it);
		Unlock();
		//解除管理
		pTPD->ptr_pool	= NULL;
		return;
	}
}

void Thread_Pool::Cleanup()
{
	Lock();
	std::map<std::string, THREAD_PARAM_DATA*>::iterator it = 	m_map_threads.begin();
	while(it!=m_map_threads.end())
	{//set shutdown command
		THREAD_PARAM_DATA* pTPD	= it->second;
		//		Unlock();
		if(pTPD!=NULL)
		{
			pTPD->Lock();
			pTPD->command	= THREAD_POOL_CMD_SHUTDOWN;
			pTPD->Unlock();
		}
		it++;
	}
	//wait for thread 
	it	= m_map_threads.begin();
	while(it != m_map_threads.end())
	{
		THREAD_PARAM_DATA* pTPD	= it->second;
		m_map_threads.erase(it);
		Unlock();
		if(pTPD==NULL)
			return;

		pTPD->Lock_Runing();
		pTPD->Unlock_Runing();
		//解除管理
		pTPD->ptr_pool	= NULL;

		Lock();
		it = m_map_threads.begin();

	}
	m_map_threads.clear();
	Unlock();

	m_unique_num	= 0;
}

void Thread_Pool::Lock()
{
	LOCK_CS(&m_lock_pool);
}

void Thread_Pool::Unlock()
{
	UNLOCK_CS(&m_lock_pool);
}


int Thread_Pool::get_Thread_count()
{
	return m_map_threads.size();
}
