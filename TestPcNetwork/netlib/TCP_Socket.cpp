#include "stdafx.h"
#include "TCP.h"
#include <stdio.h>
#include <stdlib.h>

TCP_Socket::TCP_Socket()
{
	m_socket=INVALID_SOCKET;
	INIT_CS(&m_lock_list_session);
}
TCP_Socket::~TCP_Socket()
{
	Shutdown();
	DELETE_CS(&m_lock_list_session);
}

void TCP_Socket::Shutdown()
{
	CleanupSessionList();
	if(m_socket!=INVALID_SOCKET)
	{
		CLOSESOCKET(m_socket);
		m_socket=INVALID_SOCKET;
	}
}
bool TCP_Socket::Create(uint16 port)
{
	if(m_socket!=INVALID_SOCKET)
		Shutdown();
	m_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(m_socket==INVALID_SOCKET)
	{
		Sys_Log("net_lib_log","create tcp socket failed.");
		return false;
	}
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;
	if(bind(m_socket,(SOCKADDR*)&addr,sizeof(addr))==SOCKET_ERROR)
	{
		Sys_Log("net_lib_log","tcp socket bind failed.");
		CLOSESOCKET(m_socket);
		m_socket=INVALID_SOCKET;
		return false;
	}
	if(listen(m_socket,SOMAXCONN)==SOCKET_ERROR)
	{
		Sys_Log("net_lib_log","NetSocket:listen error");
		CLOSESOCKET(m_socket);
		m_socket=INVALID_SOCKET;
		return false;
	}
	return true;
}

void TCP_Socket::CleanupSessionList()
{
	LOCK_CS(&m_lock_list_session);
	std::vector<TCP_Session*>::iterator it = m_list_session.begin();
	while(it!=m_list_session.end())
	{
		TCP_Session* pSession = *it;
		if(pSession)
		{
			pSession->Disconnect();
			delete(pSession);
		}
		m_list_session.erase(it);
		it = m_list_session.begin();
	}
	UNLOCK_CS(&m_lock_list_session);
}
int TCP_Socket::GetSessionCount()
{
	LOCK_CS(&m_lock_list_session);
	int ret=m_list_session.size();
	UNLOCK_CS(&m_lock_list_session);
	return ret;
}

void TCP_Socket::Process()
{
	timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	FD_ZERO(&m_readfds);
	FD_ZERO(&m_errfds);
	FD_ZERO(&m_writefds);
	FD_SET(m_socket,&m_readfds);
	LOCK_CS(&m_lock_list_session);
	for(int i=0;i<m_list_session.size();++i)
	{
		FD_SET(m_list_session[i]->m_socket,&m_readfds);
		FD_SET(m_list_session[i]->m_socket,&m_writefds);
		FD_SET(m_list_session[i]->m_socket,&m_errfds);
	}
	UNLOCK_CS(&m_lock_list_session);
	int ret = select(FD_SETSIZE,&m_readfds,&m_writefds,&m_errfds,&timeout);
	if(ret > 0)
	{
		if(FD_ISSET(m_socket,&m_readfds))
			CreateSession(0,0);
		NET_Packet*	ptr_recv_packet	= new NET_Packet;
		for(int i=0;i<GetSessionCount();++i)
		{
			LOCK_CS(&m_lock_list_session);
			TCP_Session* session=m_list_session[i];
			UNLOCK_CS(&m_lock_list_session);
			//if(FD_ISSET(m_list_session[i]->m_socket,&m_errfds))
			//	CloseSession(m_list_session[i]);
			if(FD_ISSET(session->m_socket,&m_writefds))
			{
				//send request packet
				NET_Packet* packet=session->PopSendPacket();
				if(packet)
					session->send(packet);
			}
			if(FD_ISSET(session->m_socket,&m_readfds))
			{
				int recv_size = ptr_recv_packet->recv(session->get_SOCKET());
				if(recv_size>0)
				{
					NET_Packet*	pPacket;
					while(ptr_recv_packet && ptr_recv_packet->is_cling()) 
					{
						pPacket	= ptr_recv_packet;
						ptr_recv_packet	= pPacket->get_cling_packet();
						if(pPacket->getCmd()==NLIBP_CONNECT)
							session->time_set(Sys_GetTime());
						if(session->get_TCP_Socket()->PushRecvPacket(pPacket, Sys_GetTime()))
							continue;
						delete(pPacket);
					}
					if(ptr_recv_packet==NULL)
						ptr_recv_packet	= new NET_Packet;
				}
			}
		}
	}
}


bool		TCP_Socket::PushRecvPacket(NET_Packet* ptr_packet, time_t time)
{
	if(ptr_packet==NULL)
		return false;
	return NET_Socket::PushRecvPacket(ptr_packet, time);
}

TCP_Session* TCP_Socket::CreateSession(uint32 ip, uint16 port, const char* szAddr)
{
	TCP_Session* pRet	= NULL;
	if(ip==0 && port==0)
	{//accept a connect
		pRet	= new TCP_Session(this);
		if(pRet==NULL)
			return NULL;
		int len = sizeof(pRet->get_sockaddr_in());
		pRet->m_socket = accept(m_socket, (SOCKADDR *)&(pRet->m_addr_in), (socklen_t *)&len);
		if (pRet->m_socket == INVALID_SOCKET || pRet->m_socket == NULL)
		{
			delete(pRet);
			Sys_Log("net_lib_log", "accept error");
			return NULL;
		}
		LOCK_CS(&m_lock_list_session);
		m_list_session.push_back(pRet);
		UNLOCK_CS(&m_lock_list_session);
		pRet->NET_Session::Connect();
	}
	else
	{//make connect
		pRet	= new TCP_Session(this);
		if(pRet==NULL)
			return NULL;
		sockaddr_in*	p_addr =	pRet->get_sockaddr_in();
		memset(p_addr, 0, sizeof(sockaddr_in));
		p_addr->sin_family = AF_INET;
		p_addr->sin_port = htons(port);
		if(szAddr)
			p_addr->sin_addr.s_addr = inet_addr(szAddr);
		else
			p_addr->sin_addr.s_addr = htonl(ip);
		if(p_addr->sin_addr.s_addr==INADDR_NONE)
		{
			hostent *host=NULL;
			if(!szAddr) 
			{
				delete(pRet);
				return NULL;
			}
			host=gethostbyname(szAddr);
			if(!host) 
			{
				delete(pRet);
				return NULL;
			}
			memcpy(&p_addr->sin_addr,host->h_addr_list[0],host->h_length);
		}
		if(!pRet->Connect())
			return NULL;
		LOCK_CS(&m_lock_list_session);
		m_list_session.push_back(pRet);
		UNLOCK_CS(&m_lock_list_session);
	}
	pRet->time_set(Sys_GetTime());
	return pRet;
}