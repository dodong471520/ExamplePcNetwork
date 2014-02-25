#ifndef __G_PLATFORM_H__
#define __G_PLATFORM_H__
#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <time.h>

#ifdef WIN32

#include <windows.h>
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif
#ifndef VSNPRINTF
#define VSNPRINTF(a,b,c,d) _vsnprintf(a,b,c,d)
#endif
/* thread operate*/
#define CREATE_THREAD(func,arg)		CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)func,(void*)arg,NULL,NULL)
#define CREATE_THREAD_RET(ret)		((ret)==0)
#define LOCK_CS(p)					EnterCriticalSection(p)	
#define UNLOCK_CS(p)				LeaveCriticalSection(p)
#define INIT_CS(p)					InitializeCriticalSection(p)
#define DELETE_CS(p)				DeleteCriticalSection(p)					
#define TYPE_CS						CRITICAL_SECTION 
#else
#define LINUX
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;
#define TRUE  1
#define FALSE 0

#define VSNPRINTF(a,b,c,d) vsnprintf(a,b,c,d)
/* thread operate */
#include <pthread.h>
#include <semaphore.h>
extern pthread_t _pthreadid;
#define CREATE_THREAD(func,arg)		pthread_create(&_pthreadid,NULL,func,(void*)arg)
#define CREATE_THREAD_RET(ret)		((ret)!=0)
#define LOCK_CS(p)					sem_wait(p)	
#define UNLOCK_CS(p)				sem_post(p)
#define INIT_CS(p)					sem_init(p,0,1)
#define DELETE_CS(p)				sem_destroy(p)					
#define TYPE_CS						sem_t		
#endif

void Sys_Log(char *format,...);
void Sys_Sleep(int ms);
unsigned long Sys_GetTime();
unsigned long Sys_GetSec();
void Sys_CreateConsole(int width, int height);

#endif
