// net_lib.h: interface for the net_lib class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NET_LIB_H__F914925A_07DE_4EBB_B14C_2E787759E3D5__INCLUDED_)
#define AFX_NET_LIB_H__F914925A_07DE_4EBB_B14C_2E787759E3D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#pragma warning (disable:4786)
#pragma warning (disable:4251)

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


#define CONNECT_CHECK_TIME		5000
#define CONNECT_TIME_OUT		10000
#define LIMIT_PACKET_BUF		10000
//lib protocols 
#define NLIBP_NULL				0x0000
#define NLIBP_ALIVE				0x0001
#define NLIBP_CHECK_CONNECT		0x0002
#define NLIBP_DISCONNECT		0x0003
#define NLIBP_CONNECT			0x0010
#define NLIBP_PING				0x0100
#define NLIBP_PING_RET			0x0101

#define NLIBP_IC				0x0200				//序号校验
#define NLIBP_IC_QUERY			0x0201				//序号校验要求重发数据包		重发的序号
#define NLIBP_IC_LOST			0x0202				//序号校验要求重发数据包失败	重发的序号
#define NLIBP_IC_OK				0x0203				//序号校验成功					最后一个数据包的序号

#define VER_PACKET		0x1001
#define MAX_PACKET_SIZE	2000

//序号校验定义
#define		IC_NULL		0
#define		IC_BEGIN	1
#define		IC_END		60000


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


const int NET_CONNECT_TIMEOUT		= 30*1000;
const int NET_KEEP_ALIVE_INTERVAL	= 10*1000;
#ifdef _DEBUG
const int NET_TIMEOUT			= 900*1000;
#else
const int NET_TIMEOUT			= 90*1000;
#endif


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef list<string> 	StrList;
typedef list<void *>	PtrList;
typedef list<uint32>	UinList;




class NET_Packet;
class NET_Session;
class NET_Socket;

class TCP_Session;
class TCP_Socket;

class UDP_Session;
class UDP_Socket;

class Thread_Pool;

class LOG;

#define		NET_LIB_API


void Sys_Log(const char* strLevel, const char *format,...);
void Sys_Sleep(int ms);
unsigned long Sys_GetTime();
unsigned long Sys_GetSec();
void Sys_CreateConsole(int width, int height);


bool Net_Startup(int VersionHigh,int VersionLow);
bool Net_Cleanup();

bool NET_CanRead(SOCKET socket);
bool NET_CanWrite(SOCKET socket);
bool NET_HasExcept(SOCKET socket);

bool NET_WaitForRead(SOCKET socket, long time_out_sec=60);
bool NET_WaitForWrite(SOCKET socket, long time_out_sec=60);

NET_Packet*	NET_CreatePacket(int size = MAX_PACKET_SIZE);

unsigned long NET_GetLocalIP();
bool NET_GetLocalAddr(SOCKET socket, char *addr, short *port, unsigned long *ip);
bool NET_GetRemoteAddr(SOCKET socket, char *addr, short *port,unsigned long *ip);
bool NET_SetReuseAddr(SOCKET socket, bool reuse);
bool NET_SetSendBufferSize(SOCKET socket, int len);
bool NET_SetRecvBufferSize(SOCKET socket, int len);

//thread
//DWORD thread_UDP_transfers( LPVOID lpParam );
//DWORD thread_TCP_listener(LPVOID lpParam);
//THREAD_FUNC(thread_UDP_transfers);		//要求传入正确的UDP_Socket指针作为参数
THREAD_FUNC(thread_UDP_send);			//要求传入正确的UDP_Socket指针作为参数
THREAD_FUNC(thread_UDP_recv);			//要求传入正确的UDP_Socket指针作为参数
THREAD_FUNC(thread_TCP_listen);			//要求传入正确的TCP_Socket指针作为参数
THREAD_FUNC(thread_TCP_send);			//要求传入正确的TCP_Session指针作为参数
THREAD_FUNC(thread_TCP_recv);			//要求传入正确的UDP_Session指针作为参数
THREAD_FUNC(thread_check_connect);		//要求传入正确的NET_Session指针作为参数
THREAD_FUNC(thread_close_session);
THREAD_FUNC(thread_close_socket);

#pragma pack(1)
struct NET_PACKET_HEADER {
	uint16 ver;
	uint16 ic;
	uint16 reserved;
	uint32 sid;
	uint16 len;
	uint32 uid;
};
#pragma pack()


//#include "log.h"
extern LOG*	g_ptr_net_lib_log;

#endif // !defined(AFX_NET_LIB_H__F914925A_07DE_4EBB_B14C_2E787759E3D5__INCLUDED_)
