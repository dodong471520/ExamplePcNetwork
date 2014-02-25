#ifndef _SOCKET_WRAP_H_
#define _SOCKET_WRAP_H_

#include "YYMacros.h"

#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	#include <winsock2.h>
#else
    #include <unistd.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h> 
#endif

#include "YYException.h"


NS_YY_BEGIN

inline void Init();
inline void Destroy();
inline int CreateSocket();
inline void CloseSocket(int socket);
inline int Connect(const char* ip, int port);



//////////////////////////////////////////////////////////////////////////////
void Init()
{
#ifdef WIN32
	WSADATA wsaData;
	if((WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
	{
		THROWNEXCEPT("WSAStartup() failed\n");
	}
#endif
}

void Destroy()
{
#ifdef WIN32
	WSACleanup();
#endif
}

int CreateSocket()
{
	//create socket
	int new_socket = socket(	AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef WIN32
	if(new_socket == INVALID_SOCKET)
	{
		THROWNEXCEPT("create socket error.");
	}
#else
	if(new_socket ==-1)
	{
		THROWNEXCEPT("create socket error.");
	}
#endif

	return new_socket;
}

void CloseSocket(int socket)
{
#ifdef WIN32
		closesocket(socket);
#else
		close(socket);
#endif
}


int Connect(const char* ip, int port)
{
	int client_socket=CreateSocket();

	//connect to server
	sockaddr_in server_addr;
	server_addr.sin_family=AF_INET;

#ifdef WIN32
	server_addr.sin_addr.s_addr = inet_addr(ip);
#else
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
#endif
	server_addr.sin_port=htons(port);
	if(connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) ==-1 )
	{
		CloseSocket(client_socket);
		return -1;
	}

	return client_socket;
}

NS_YY_END
#endif