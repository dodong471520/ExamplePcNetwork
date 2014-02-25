#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <time.h>

#include <string>
#include <list>
#include <vector>
#include <bitset>
#include <map>
using namespace std;

#ifdef _WIN32

#include <windows.h>
#include <winsock.h>



#define GETERROR			WSAGetLastError()
#define CLOSESOCKET(s)		closesocket(s)
#define IOCTLSOCKET(s,c,a)  ioctlsocket(s,c,a)
#define CONN_INPRROGRESS	WSAEWOULDBLOCK
typedef int socklen_t;

#define _countof(array) (sizeof(array)/sizeof(array[0]))
#define VSNPRINTF(a,b,c,d) _vsnprintf(a,b,c,d)
/* thread operate*/
#define THREAD_FUNC(func)			DWORD WINAPI func(LPVOID lpParam)
#define PTR_THREAD_FUNC				LPTHREAD_START_ROUTINE
#define CREATE_THREAD(func,arg)		CreateThread(NULL,NULL,(PTR_THREAD_FUNC)func,(void*)arg,NULL,NULL)
#define CREATE_THREAD_RET(ret)		((ret)==0)
#define TRY_CS(p)					TryEnterCriticalSection(p)
#define LOCK_CS(p)					EnterCriticalSection(p)	
#define UNLOCK_CS(p)				LeaveCriticalSection(p)
#define INIT_CS(p)					InitializeCriticalSection(p)
#define DELETE_CS(p)				DeleteCriticalSection(p)					
#define TYPE_CS						CRITICAL_SECTION 


#else
#define LINUX

#include <sys/types.h>

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


#define CLOSESOCKET(s)  close(s)
#define CONN_INPRROGRESS    EINPROGRESS
#define GETERROR errno
#define IOCTLSOCKET(s,c,a)    ioctl(s,c,a)
#define INVALID_SOCKET       (-1)
#define SOCKET_ERROR       (-1)

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/errno.h>
#include <arpa/inet.h>

#include <stddef.h>

#define VSNPRINTF(a,b,c,d) vsnprintf(a,b,c,d)
/* thread operate */
#include <pthread.h>
#include <semaphore.h>
extern pthread_t _pthreadid;
#define CREATE_THREAD(func,arg)		pthread_create(&_pthreadid,NULL,func,(void*)arg)
#define CREATE_THREAD_RET(ret)		((ret)!=0)
#define TRY_CS(p)
#define LOCK_CS(p)					sem_wait(p)	
#define UNLOCK_CS(p)				sem_post(p)
#define INIT_CS(p)					sem_init(p,0,1)
#define DELETE_CS(p)				sem_destroy(p)					
#define TYPE_CS						sem_t		


#endif 
#define MAX_PACKET_SIZE	2000

bool net_startup(int VersionHigh,int VersionLow)
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(VersionHigh,VersionLow);
	err=WSAStartup(wVersionRequested, &wsaData);

	/* startup failed */
	if (err!=0)									
	{
		Sys_Log("WSAStartup Error");
		WSACleanup();
		return false;
	}

	/* version error */
	if (LOBYTE(wsaData.wVersion)!= VersionLow ||
		HIBYTE(wsaData.wVersion)!= VersionHigh ) 
	{
		Sys_Log("WSAStartup Version Error");
		WSACleanup();
		return false;
	}
	Sys_Log("WSAStartup OK");
#endif
	return true;
}

int net_recv(SOCKET s,char *buff,int len)
{
	int ret=recv(s,buff,len,0);
	if(ret==0)return -1;
	if(ret==SOCKET_ERROR)
	{
		int err=GETERROR;
		if(err!=WSAEWOULDBLOCK)
			return -1;
	}
	return ret;
}