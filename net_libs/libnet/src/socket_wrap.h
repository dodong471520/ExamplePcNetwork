#ifndef _SOCKET_WRAP_H_
#define _SOCKET_WRAP_H_

#include "yy_macros.h"
#include <winsock2.h>
#include "yy_exception.h"
NS_YY_BEGIN

//非阻塞io
inline void NetInit();
inline void NetDestroy();
inline int CreateSocket();
inline void CloseSocket(SOCKET sd);

//创建监听socket，默认设置为非阻塞
inline void ListenSocket(SOCKET &listen_sd, const char* ip, UI32 port);


//阻塞连接，新socket为非阻塞, 返回值判断是否连接成功
inline SOCKET ConnectSocket(SOCKET listen_sd, const char* ip, int port);

//非阻塞accept, 返回值判断是否accept成功
inline bool AcceptSocket(SOCKET listen_sd, SOCKET& new_sd);



//////////////////////////////////////////////////////////////////////////////
void NetInit()
{
	WSADATA wsaData;
	if((WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
	{
		THROWNEXCEPT("WSAStartup() failed\n");
	}
}

void NetDestroy()
{
	WSACleanup();
}

int CreateSocket()
{
	//create socket
	SOCKET new_socket = ::socket(	AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(new_socket == INVALID_SOCKET)
	{
		THROWNEXCEPT("create socket error.errno:%d", WSAGetLastError());
	}

	return new_socket;
}

void CloseSocket(SOCKET sd)
{
	::closesocket(sd);
}

void ListenSocket(SOCKET &listen_sd, const char* ip, UI32 port)
{
	if(NULL == ip)
		THROWNEXCEPT("ip is null.");

	listen_sd = CreateSocket();

	sockaddr_in service;
	service.sin_family = AF_INET;

	//把字符串转换成32位的网络字节序
	service.sin_addr.s_addr = inet_addr(ip);

	//把16位端口号从主机字节序转换成网络字节序
	service.sin_port = htons(port);

	if(::bind( listen_sd, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR)
		THROWNEXCEPT("bind socket error.errno:%d", WSAGetLastError());

	u_long iMode = 1;  //non-blocking mode is enabled.
	ioctlsocket(listen_sd, FIONBIO, &iMode); //设置为非阻塞模式

	if( ::listen(listen_sd, 5) == SOCKET_ERROR )
		THROWNEXCEPT("listen socket erro.errno:%", WSAGetLastError());

	
	//关闭发送缓冲,必须在listen之前设置,这样wsasend的时候，wsabuf指向用户缓冲区，内存锁定。
	//int nZero=0;
	//assert(setsockopt(m_listen_sd, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero)) != SOCKET_ERROR);
	//assert( ::listen(m_listen_sd, 5) != SOCKET_ERROR );

	/*
	//关闭连接的时候，发送缓冲区和接受缓冲区中的所有数据都丢弃, time_wait错误的缘由
	LINGER lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;

	nRet = setsockopt(m_listen_sd, SOL_SOCKET, SO_LINGER,
	(char *)&lingerStruct, sizeof(lingerStruct) );
	if( nRet == SOCKET_ERROR )
	{
		THROWNEXCEPT("setsockopt(SO_LINGER) failed: %d\n", WSAGetLastError());
	}*/
}

//如果客户端和服务端在同一主机，则非阻塞connect通常立即建立连接；
//否则返回EINPROGRESS, 表示连接建立已经启动但是尚未完成，已经发起的tcp三路握手继续进行
//非阻塞connect移植性不好，而且一般都是进程刚启动的时候做的操作，所以用阻塞方式，可能会阻塞几十秒，也是可以的
SOCKET ConnectSocket(SOCKET listen_sd, const char* ip, int port)
{
	SOCKET new_sd=CreateSocket();

	//connect to server
	sockaddr_in server_addr;
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port=htons(port);
	if(::connect(new_sd, (sockaddr*)&server_addr, sizeof(server_addr)) ==-1 )
	{
		CloseSocket(new_sd);
		return INVALID_SOCKET;
	}

	//设置为非阻塞模式
	u_long iMode = 1;  //non-blocking mode is enabled.
	if(ioctlsocket(new_sd, FIONBIO, &iMode) == SOCKET_ERROR)
		THROWNEXCEPT("set no block mode erro. errno:%d", WSAGetLastError());

	return new_sd;
}

//即使用了select判断，也有可能在select成功后，客户端断开连接，导致accept阻塞，所以要用非阻塞io
bool AcceptSocket(SOCKET listen_sd, SOCKET& new_sd)
{
	sockaddr_in clientAddr;
	int nClientLen = sizeof(sockaddr_in);

	//非阻塞accept
	new_sd=::accept(listen_sd, (sockaddr*)&clientAddr, &nClientLen);
	if(new_sd == INVALID_SOCKET)
	{
		int err_num=WSAGetLastError();
		if(err_num==WSAEWOULDBLOCK)
			return false;

		THROWNEXCEPT("accept socket erro. errno:%d", err_num);
	}

	//有新连接进来，设置成非阻塞的
	u_long iMode = 1;  //non-blocking mode is enabled.
	if(ioctlsocket(new_sd, FIONBIO, &iMode) == SOCKET_ERROR)
		THROWNEXCEPT("set no block mode erro. errno:%d", WSAGetLastError());

	return true;
}

NS_YY_END
#endif