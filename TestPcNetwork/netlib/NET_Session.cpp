#include "stdafx.h"
#include "net_lib.h"


NET_Session::NET_Session(uint32 uid)
{
	m_uid	= uid;
	m_uid_opposite	= 0;
	m_time_set	= 0;
	m_net_delay	= 0;

	m_connect_check_time	= CONNECT_CHECK_TIME;

	m_ptr_net_socket	= NULL;

	m_index_send_check	= IC_BEGIN;
	m_index_recv_check	= IC_BEGIN;
	m_ric_seq			= 0;
	INIT_CS(&m_lock_ic_image);
}



NET_Session::~NET_Session()
{
	DELETE_CS(&m_lock_ic_image);

}


void NET_Session::Disconnect()
{
	if(m_ptr_net_socket==NULL)
		return;
	//connect
	NET_Packet*		pPacket	= new NET_Packet;
	pPacket->setCmd(NLIBP_DISCONNECT);				//马上通知对方断开连接
	this->send(pPacket);
}


void NET_Session::mark_index(NET_Packet *ptr_packet)
{
	if(ptr_packet==NULL)
		return;
	ptr_packet->setIC(m_index_send_check);
	m_map_packet_image[m_index_send_check]	= ptr_packet;
	m_index_send_check++;
	if(m_index_send_check%16==0)
	{
		if(m_index_send_check>IC_END)
		{
			m_index_send_check	= IC_BEGIN;
		}
	}
}

bool NET_Session::Connect()
{
	if(m_ptr_net_socket==NULL)
		return;
	//connect
	NET_Packet*		pPacket	= new NET_Packet;
	pPacket->setCmd(NLIBP_CONNECT);				//连接命令
	this->send(pPacket);
	return true;
}

void NET_Session::time_set(time_t time)
{
	m_time_set	= time;
}