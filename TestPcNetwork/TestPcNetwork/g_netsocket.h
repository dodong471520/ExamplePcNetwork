#ifndef __CG_NET_SOCKET_H__
#define __CG_NET_SOCKET_H__

#include "g_platform.h"
#ifdef WIN32
#include <winsock.h>
#define GETERROR			WSAGetLastError()
#define CLOSESOKCET(s)		closesocket(s)
#define IOCTLSOCKT(s,c,a)	ioctlsocket(s,c,a)
#define CONN_INPROGRESS		WSAEWOULDBLOCK
typedef int socklen_t;
#else

#endif

int const PROTOCOL_UDP=1;
int const PROTOCOL_TCP=2;
class CG_NetSocket
{
public:
	CG_NetSocket();
	virtual ~CG_NetSocket();
	bool Attach(SOCKET socket);
	bool Close();
	bool Connect(char *szAddr,int port,unsigned long ip = 0);
	bool Listen();
	bool Initialize(int protocol);

	int Recv(char *buf,int len);
	int Send(char *buf,int len);
	int RecvFrom(char *buf,int len,SOCKADDR_IN *addr,int *addrlen);
	int SendTo(char *buf,int len,SOCKADDR_IN *addr);

	bool CanWrite();
	bool CanRead();
	bool HasExcept();

	bool SetNonBlocking();
	bool BindAddr(char *ip,int port);
	void Reset();

	bool SetSendBufferSize(int len);
	bool SetRecvBufferSize(int len);
	bool SetReuseAddr(bool reuse);

	bool GetLocalAddr (char *addr, short *port,unsigned long *ip = NULL);
	bool GetRemoteAddr(char *addr,short *port,unsigned long *ip = NULL);

	SOCKET Accept();
private:
	bool _NetStartUp(int VersionHigh,int VersionLow);
	bool _NetCleanUp();

	SOCKET		m_socket;
	static int	m_nCount;
};

#endif