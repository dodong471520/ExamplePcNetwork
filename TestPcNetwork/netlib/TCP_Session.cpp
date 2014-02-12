#include "stdafx.h"
#include "TCP.h"


bool TCP_Session::Connect()
{
	if(connect(m_socket,(SOCKADDR *)get_sockaddr_in(),sizeof(m_addr_in))
		==SOCKET_ERROR)
	{
		int err = GETERROR;
		if (err != CONN_INPRROGRESS)
		{
			Sys_Log("net_lib_log", "socket connect error = %d",err); 
			return false;
		}
		else
		{
			return false;
		}
	}
	NET_Session::Connect();
	return true;
}

int TCP_Session::send(NET_Packet* pPacket, bool bIC)
{
	if(pPacket==NULL)
		return 0;
	if(bIC)
	{
		this->mark_index(pPacket);
		return  pPacket->send_to(this);
	}
	else
	{
		int ret = pPacket->send_to(this);
		if(pPacket->getIC()==IC_NULL)
			delete(pPacket);
		return ret;
	}
}


TCP_Session::TCP_Session(TCP_Socket* ptr_tcp_socket)
{
	m_socket	= INVALID_SOCKET;
	m_ptr_net_socket	= ptr_tcp_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_socket ==INVALID_SOCKET) 
		Sys_Log("net_lib_log", "create tcp socket failed." );
	INIT_CS(&m_lock_send_packet);
}

TCP_Session::~TCP_Session()
{
	CLOSESOCKET(m_socket);
	m_socket	= INVALID_SOCKET;
	DELETE_CS(&m_lock_send_packet);
}

NET_Packet*	TCP_Session::PopSendPacket()
{
	NET_Packet* pRet;
	LOCK_CS(&m_lock_send_packet);
	if(m_lst_send_packet.begin()!=m_lst_send_packet.end())
	{
		pRet	= *m_lst_send_packet.begin();
		m_lst_send_packet.pop_front();
	}
	else
		pRet	= NULL;
	UNLOCK_CS(&m_lock_send_packet);
	return pRet;
}