#include "stdafx.h"
#include "TCP.h"
#include <stdio.h>
#include <stdlib.h>

template<typename T>
TCP_Socket<T>::TCP_Socket()
{
	m_lastTime=0;
	m_socket=INVALID_SOCKET;
	FD_ZERO( &m_ReadFDs[SELECT_BAK] ) ;
	FD_ZERO( &m_WriteFDs[SELECT_BAK] ) ;
	FD_ZERO( &m_ExceptFDs[SELECT_BAK] ) ;
	FD_ZERO( &m_ReadFDs[SELECT_USE] ) ;
	FD_ZERO( &m_WriteFDs[SELECT_USE] ) ;
	FD_ZERO( &m_ExceptFDs[SELECT_USE] ) ;

	m_Timeout[SELECT_BAK].tv_sec = 0;
	m_Timeout[SELECT_BAK].tv_usec = 0;
}
template<typename T>
TCP_Socket<T>::~TCP_Socket()
{
	Shutdown();
}
template<typename T>
void TCP_Socket<T>::Shutdown()
{
	CleanupAllSession();
	if(m_socket!=INVALID_SOCKET)
	{
		CLOSESOCKET(m_socket);
		m_socket=INVALID_SOCKET;
	}
}
template<typename T>
bool TCP_Socket<T>::CreateServer(uint16 port)
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
	FD_SET(m_socket,&m_ReadFDs[SELECT_BAK]);
	return true;
}
template<typename T>
void TCP_Socket<T>::CreateClient(char const* szAddr,uint16 port)
{

}
template<typename T>
void TCP_Socket<T>::CloseSession(TCP_Session *session)
{
	std::map<uint32, TCP_Session*>::iterator it = m_map_session.find(session->m_socket);
	if(it!=m_map_session.end())
	{
		pSession = it->second;
		m_map_session.erase(it);
	}
	delete session;
}

template<typename T>
void TCP_Socket<T>::CleanupAllSession()
{
	std::map<SOCKET,T*>::iterator it = m_map_session.begin();
	while(it!=m_map_session.end())
	{
		T* pSession = *it;
		if(pSession)
		{
			pSession->Disconnect();
			delete(pSession);
		}
		m_map_session.erase(it);
		it = m_map_session.begin();
	}
}
template<typename T>
void TCP_Socket<T>::ProcessServer()
{
	m_Timeout[SELECT_USE].tv_sec  = m_Timeout[SELECT_BAK].tv_sec;
	m_Timeout[SELECT_USE].tv_usec = m_Timeout[SELECT_BAK].tv_usec;
	m_ReadFDs[SELECT_USE]   = m_ReadFDs[SELECT_BAK];
	m_WriteFDs[SELECT_USE]  = m_WriteFDs[SELECT_BAK];
	m_ExceptFDs[SELECT_USE] = m_ExceptFDs[SELECT_BAK];

	int ret = select(FD_SETSIZE,&m_ReadFDs[SELECT_USE],&m_WriteFDs[SELECT_USE],&m_ExceptFDs[SELECT_USE],&m_Timeout[SELECT_USE]);
	if(ret > 0)
	{
		//创建会话
		if(FD_ISSET(m_socket,&m_ReadFDs[SELECT_USE]))
			CreateSession();
		NET_Packet*	ptr_recv_packet	= new NET_Packet;
		std::map<SOCKET,T*>::iterator it;
		for(it=m_map_session.begin();it!=m_map_session.end();)
		{
			T* session=*it;
			it++;
			//发送数据包
			if(FD_ISSET(session->m_socket,&m_WriteFDs[SELECT_USE]))
			{
				NET_Packet *packet=session->PopSendPacket();
				if(packet)
					session->send(packet);
			}
			//收到数据包
			if(FD_ISSET(session->m_socket,&m_ReadFDs[SELECT_USE]))
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
						if(session->PushRecvPacket(pPacket, Sys_GetTime()))
						{
							//处理接收到的消息
							continue;
						}
						delete(pPacket);
					}
					if(ptr_recv_packet==NULL)
						ptr_recv_packet	= new NET_Packet;
				}
			}
		}
	}
	//心跳
	time_t t= Sys_GetTime();
	if(m_lastTime==0)
		m_lastTime=t;
	if(t - m_lastTime>=1000)
	{
		CheckConnect(t);
		m_lastTime = t;
	}
}
template<typename T>
T* TCP_Socket<T>::CreateSession()
{
	T* pRet	= new T;
	//accept a connect
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
	m_map_session.insert(make_pair((SOCKET)pRet->m_socket,pRet));
	pRet->time_set(Sys_GetTime());
	FD_SET(pRet->m_socket , &m_ReadFDs[SELECT_BAK]);
	FD_SET(pRet->m_socket , &m_WriteFDs[SELECT_BAK]);
	FD_SET(pRet->m_socket , &m_ExceptFDs[SELECT_BAK]);
	return pRet;
}
template<typename T>
void TCP_Socket<T>::CheckConnect(time_t time)
{
	if(m_socket<=0)
		return;
	std::map<SOCKET, T*>::iterator it = m_map_session.begin();
	while(it!=m_map_session.end())
	{
		if(it->second)
			it->second->Check_connect(time);
		it++;
	}
}