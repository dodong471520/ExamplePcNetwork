#include "g_netsocket.h"
#ifdef WIN32
#pragma comment(lib,"Ws2_32.lib")
#endif

int CG_NetSocket::m_nCount = 0;
bool CG_NetSocket::Initialize( int protocol )
{
	if(m_nCount++==0)
		if(!_NetStartUp(1,1))return false;
	if(protocol==PROTOCOL_UDP)
		m_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(protocol==PROTOCOL_TCP)
		m_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(m_socket==INVALID_SOCKET)
		return false;
	SetNonBlocking();
	return true;
}

bool CG_NetSocket::_NetStartUp( int VersionHigh,int VersionLow )
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested=MAKEWORD(VersionHigh,VersionLow);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
	{
		Sys_Log("WSAStartup Error");
		WSACleanup();
		return false;
	}
	if(LOBYTE(wsaData.wVersion)!=VersionLow||HIBYTE(wsaData.wVersion)!=VersionHigh)
	{
		Sys_Log("WSAStartup Version Error");
		WSACleanup();
		return false;
	}
	Sys_Log("WSAStartup OK");
#endif
}

bool CG_NetSocket::SetNonBlocking()
{
	u_long arg=1;
	if(IOCTLSOCKT(m_socket,FIONBIO,&arg)==SOCKET_ERROR)
		return false;
	else
		return true;
}

CG_NetSocket::CG_NetSocket()
{
	Reset();
}

void CG_NetSocket::Reset()
{
	m_socket=INVALID_SOCKET;
}

CG_NetSocket::~CG_NetSocket()
{
	Close();
}

bool CG_NetSocket::Close()
{
	if(m_socket==INVALID_SOCKET) return false;
	CLOSESOKCET(m_socket);
	Reset();
	if(--m_nCount==0)
		_NetCleanUp();
	return true;
}

bool CG_NetSocket::_NetCleanUp()
{
#ifdef WIN32
	WSACleanup();
#endif
	return true;
}

bool CG_NetSocket::BindAddr( char *ip,int port )
{
	SOCKADDR_IN addrlocal;
	addrlocal.sin_family=AF_INET;
	addrlocal.sin_port=htons(port);
	if(ip)
		addrlocal.sin_addr.s_addr=inet_addr(ip);
	else
		addrlocal.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(m_socket,(SOCKADDR*)&addrlocal,sizeof(addrlocal))==SOCKET_ERROR)
	{
		Sys_Log("bind socket error");
		return false;
	}
	return true;
}

bool CG_NetSocket::Listen()
{
	if(listen(m_socket,SOMAXCONN)==SOCKET_ERROR)
	{
		Sys_Log("NetSocket:listen error");
		return false;
	}
	return true;
}

bool CG_NetSocket::Connect( char *szAddr,int port,unsigned long ip /*= 0*/ )
{
	SOCKADDR_IN addrRemote;
	hostent *host=NULL;
	memset(&addrRemote,0,sizeof(addrRemote));
	addrRemote.sin_family=AF_INET;
	addrRemote.sin_port=htons(port);
	if(szAddr)
		addrRemote.sin_addr.s_addr=inet_addr(szAddr);
	else
		addrRemote.sin_addr.s_addr=ip;
	if(addrRemote.sin_addr.s_addr==INADDR_NONE)
	{
		if(!szAddr) return false;
		host=gethostbyname(szAddr);
		if(!host)return false;
		memcpy(&addrRemote.sin_addr,host->h_addr_list[0],host->h_length);
	}
	if(connect(m_socket,(SOCKADDR*)&addrRemote,sizeof(addrRemote))==SOCKET_ERROR)
	{
		int err=GETERROR;
		if(err!=CONN_INPROGRESS)
		{
			Sys_Log("socket connect error=%d",err);
			return false;
		}
	}
	return true;
}

bool CG_NetSocket::CanRead()
{
	fd_set readfds;
	timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	FD_ZERO(&readfds);
	FD_SET(m_socket,&readfds);
	int ret=select(FD_SETSIZE,&readfds,NULL,NULL,&timeout);
	if(ret>0&&FD_ISSET(m_socket,&readfds))
		return true;
	else
		return false;
}

SOCKET CG_NetSocket::Accept()
{
	SOCKADDR_IN addr;
	int len=sizeof(addr);
	SOCKET tmp;
	tmp=accept(m_socket,(SOCKADDR*)&addr,(socklen_t*)&len);
	if(tmp==INVALID_SOCKET||tmp==NULL)
	{
		Sys_Log("accept error");
		return NULL;
	}
	m_nCount++;
	return tmp;
}

bool CG_NetSocket::Attach( SOCKET socket )
{
	m_socket=socket;
	SetNonBlocking();
	return true;
}

bool CG_NetSocket::GetLocalAddr( char *addr, short *port,unsigned long *ip /*= NULL*/ )
{
	SOCKADDR_IN addrLocal;
	socklen_t len=sizeof(addrLocal);
	if(getsockname(m_socket,(SOCKADDR*)&addrLocal,&len)==SOCKET_ERROR)
		return false;
	char *tmp=inet_ntoa(addrLocal.sin_addr);
	if(!tmp)
		return false;
	if(addr)
		strcpy(addr,tmp);
	if(port)
		*port=ntohs(addrLocal.sin_port);
	if(ip)
		*ip=addrLocal.sin_addr.s_addr;
	return true;
}

bool CG_NetSocket::GetRemoteAddr( char *addr,short *port,unsigned long *ip /*= NULL*/ )
{
	sockaddr_in addrRemote;
	int len = sizeof(addrRemote);
	if(getpeername(m_socket,(sockaddr *)&addrRemote,(socklen_t *)&len)==SOCKET_ERROR)
		return false;

	char *tmp = inet_ntoa(addrRemote.sin_addr);
	if(!tmp) 
		return false;
	if(addr)
		strcpy(addr,tmp);
	if(port)
		*port = ntohs(addrRemote.sin_port);
	if(ip)
		*ip = addrRemote.sin_addr.s_addr; 
	return true;
}

int CG_NetSocket::Send( char *buf,int len )
{
	if(!CanWrite())return 0;
	int ret;
	ret=send(m_socket,buf,len,0);
	if(ret==SOCKET_ERROR)
	{
		int err=GETERROR;
		if(err==WSAEWOULDBLOCK) return 0;
		return -1;
	}
	return ret;
}

bool CG_NetSocket::CanWrite()
{
	fd_set writefds;
	timeval timeout;

	timeout.tv_sec=0;
	timeout.tv_usec=0;
	FD_ZERO(&writefds);
	FD_SET(m_socket,&writefds);
	int ret = select(FD_SETSIZE,NULL,&writefds,NULL,&timeout);
	if(ret > 0 && FD_ISSET(m_socket,&writefds))
		return true;
	else 
		return false;
}

int CG_NetSocket::Recv( char *buf,int len )
{
	if (CanRead()==false) 
		return 0;

	int ret;
	/* in linux be careful of SIGPIPE */
	ret = recv(m_socket,buf,len,0);

	if (ret==0)
	{
		/* remote closed */
		return -1;
	}

	if (ret==SOCKET_ERROR)
	{
		int err=GETERROR;
		if (err!=WSAEWOULDBLOCK)
		{
			return -1;
		}
	}
	return ret;
}

