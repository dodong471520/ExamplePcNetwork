#include "stdafx.h"
#include "TCP.h"

template<typename T>
bool TCP_Session<T>::Connect()
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
	return true;
}
template<typename T>
int TCP_Session<T>::send(NET_Packet* pPacket, bool bIC)
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

template<typename T>
TCP_Session<T>::TCP_Session(TCP_Socket<T> *tcpsocket)
{
	m_ptr_tcp_socket=tcpsocket;
	m_socket	= INVALID_SOCKET;
	m_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_socket ==INVALID_SOCKET) 
		Sys_Log("net_lib_log", "create tcp socket failed." );
	INIT_CS(&m_lock_send_packet);
}
template<typename T>
TCP_Session<T>::~TCP_Session()
{
	CLOSESOCKET(m_socket);
	m_socket	= INVALID_SOCKET;
	DELETE_CS(&m_lock_send_packet);
}
template<typename T>
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

template<typename T>
bool TCP_Session<T>::PushRecvPacket(NET_Packet* ptr_packet, time_t time)
{
	if(ptr_packet==NULL)
		return false;
	time_set(time);
	if(!check_index(ptr_packet))
	｛
		Query_IC();
		return false;
	｝
	switch(ptr_packet->getCmd())
	{
	case NLIBP_PING:
		{
			ptr_packet->read32();
			m_net_delay	 = ptr_packet->read32();
			ptr_packet->setCmd(NLIBP_PING_RET);
			ptr_packet->begin();
			ptr_packet->write32(time);
			ptr_packet->end();
			PushSendPacket(ptr_packet);
			return false;
		}
		break;
	case NLIBP_PING_RET:
		{
			uint32 t_ref = ptr_packet->read32();
			ptr_packet->read32();
			uint32 t_oposite = ptr_packet->read32();
			m_net_delay	= time - t_ref;
			return false;
		}
		break;
	case NLIBP_CHECK_CONNECT:
		{
			time_t tm	= ptr_packet->read32();
			if(tm>20000)
			{//掉线
				m_ptr_tcp_socket->CloseSession(this);
			}
			else
				Ping();
		}
		break;
	case NLIBP_DISCONNECT:
		{
			m_ptr_tcp_socket->CloseSession(this);
			return false;
		}
		break;
	case NLIBP_IC_QUERY:
		{//收到重发请求后重发数据包
			resend(ptr_packet->getIC());
			return false;
		}
		break;
	case NLIBP_IC_OK:
		{//收到序号校验成功信息,把备份的数据包镜像删除
			update_ic_image(ptr_packet->getIC());
			return false;
		}
		break;
	case NLIBP_IC_LOST:
		{//数据包已经丢失
		}
		break;
	}
	return true;
}
