#include "stdafx.h"
#include "net_lib.h"

NET_Socket::NET_Socket()
{
	INIT_CS(&m_lock_recv_packet);
	m_socket=INVALID_SOCKET;
}

NET_Socket::~NET_Socket()
{
	DELETE_CS(&m_lock_recv_packet);
}


bool NET_Socket::PushRecvPacket(NET_Packet* ptr_packet, time_t time)
{
	if(ptr_packet==NULL)
		return false;
	NET_Session* pSession = get_NET_Session(ptr_packet->getUID());
	if(pSession)
	{
		pSession->time_set(time);
	}

	switch(ptr_packet->getCmd())
	{
	case NLIBP_PING:
		{
			if(pSession==NULL)
			{
				return false;
			}
			ptr_packet->setCmd(NLIBP_PING_RET);
			uint32 t_oposite = ptr_packet->read32();
			pSession->m_net_delay	 = ptr_packet->read32();
			// pSession->m_net_delay	= t_delay;
			//Sys_Log("receive ping from session(%d) network delay = %d", ptr_packet->getUID(), pSession->m_net_delay);
			ptr_packet->write32(time);
			pSession->PushSendPacket(ptr_packet);
			return true;
		}
		break;
	case NLIBP_PING_RET:
		{
			if(pSession==NULL)
			{
				return false;
			}
			uint32 t_ref = ptr_packet->read32();
			ptr_packet->read32();
			uint32 t_oposite = ptr_packet->read32();
			pSession->m_net_delay	= time - t_ref;
			//Sys_Log("ping ok from session(%d) network delay = %d", ptr_packet->getUID(), pSession->get_net_delay());
		}
		break;
	case NLIBP_DISCONNECT:
		{
			CloseSession(ptr_packet->getUID());
//			return false;
		}
		break;
	case NLIBP_IC_QUERY:
		{//�յ��ط�������ط����ݰ�
			if(pSession!=NULL)
			{
				pSession->resend(ptr_packet->read16());
			}

			return false;
		}
		break;
	case NLIBP_IC_OK:
		{//�յ����У��ɹ���Ϣ,�ѱ��ݵ����ݰ�����ɾ��
			if(pSession==NULL)
			{
				return false;
			}
			pSession->update_ic_image(ptr_packet->read16());
			return false;
		}
		break;
	case NLIBP_IC_LOST:
		{//���ݰ��Ѿ���ʧ
		}
		break;
	}

	if(m_lst_recv_packet.size()>LIMIT_PACKET_BUF)
	{
		NET_Packet*	p = PopRecvPacket();//*m_lst_recv_packet.begin();
		if(p)
		{
			delete(p);
		}
		//m_lst_recv_packet.pop_front();
	}
	LOCK_CS(&m_lock_recv_packet);
	m_lst_recv_packet.push_back(ptr_packet);
	UNLOCK_CS(&m_lock_recv_packet);


	return true;

}