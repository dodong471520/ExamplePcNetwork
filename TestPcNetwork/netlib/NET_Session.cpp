#include "stdafx.h"
#include "net_lib.h"


NET_Session::NET_Session()
{
	m_time_set	= 0;
	m_net_delay	= 0;

	m_connect_check_time	= CONNECT_CHECK_TIME;

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
	//connect
	NET_Packet*		pPacket	= new NET_Packet;
	pPacket->setCmd(NLIBP_DISCONNECT);				//����֪ͨ�Է��Ͽ�����
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

void NET_Session::time_set(time_t time)
{
	m_time_set	= time;
}

void NET_Session::Ping()
{
	//���ԻỰ������ʱ
	NET_Packet* pPacket = new NET_Packet;
	pPacket->setCmd(NLIBP_PING);
	pPacket->begin();
	pPacket->write32(Sys_GetTime());
	pPacket->write32(m_net_delay);
	pPacket->end();
	PushSendPacket(pPacket);
}


void NET_Session::Check_connect(time_t tm)
{
	//����ϴ�ͨ�ŵ�������������ʱ�䣬�����������ֵ��������ܱ�����һ�����ݰ����ȴ������ת��
	//	time_t time = Sys_GetTime();
	if(tm< m_time_set)
		m_time_set	= tm;
	else if(tm - m_time_set>m_connect_check_time)
	{
		NET_Packet* pPacket = new NET_Packet;
		pPacket->setCmd(NLIBP_CHECK_CONNECT);
		pPacket->begin();
		pPacket->write32(tm - m_time_set);
		pPacket->end();
		PushSendPacket(pPacket);
	}
}


void NET_Session::resend(uint16 ic)
{
	LOCK_CS(&m_lock_ic_image);
	std::map<uint16, NET_Packet*>::iterator	it = m_map_packet_image.find(ic);
	NET_Packet* pPacket = NULL;
	if(it!=m_map_packet_image.end())
	{
		pPacket	= it->second;
	}
	UNLOCK_CS(&m_lock_ic_image);
	if(pPacket)
	{
		this->send(pPacket);
		return;
	}

	pPacket	= new NET_Packet;
	pPacket->setIC(ic);
	pPacket->setCmd(NLIBP_IC_LOST);
	send(pPacket);
}


void NET_Session::Query_IC()
{
	//�����ط�����
	time_t tm = Sys_GetTime();
	//��ֹƵ���ظ��ط��������
	if(tm-m_ric_seq>30 || tm<m_ric_seq)
	{
		m_ric_seq	= tm;
		NET_Packet*	pPacket	= new NET_Packet;
		pPacket->setCmd(NLIBP_IC_QUERY);
		pPacket->write16(m_index_recv_check);
		this->send(pPacket);
	}
}

bool NET_Session::check_index(NET_Packet *ptr_packet)
{
	if(ptr_packet==NULL)
		return true;
	if(ptr_packet->getIC()==IC_NULL)
	{
		return true;
	}
	else if(m_index_recv_check==ptr_packet->getIC())
	{
		m_index_recv_check++;
		if(m_index_recv_check%16==0)
		{
			NET_Packet*	pPacket	= new NET_Packet;
			pPacket->setCmd(NLIBP_IC_OK);
			pPacket->write16(m_index_recv_check);
			this->send(pPacket);
			if(m_index_recv_check>IC_END)
			{
				m_index_recv_check	= IC_BEGIN;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

void NET_Session::update_ic_image(uint16 last_ic)
{
	std::map<uint16, NET_Packet*>::iterator	it = m_map_packet_image.begin();
	//	NET_Packet* pPacket = NULL;
	while(it!=m_map_packet_image.end())
	{
		NET_Packet* pPacket	= it->second;
		int ic_d	= last_ic-pPacket->getIC();
		if(ic_d>0 && ic_d<16*100)
		{
			delete(pPacket);
			it	= m_map_packet_image.erase(it);
		}
		else
			it++;
	}
}
