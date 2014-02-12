#include "stdafx.h"
#include "net_lib.h"

NET_Packet::NET_Packet(int data_size)
{
	m_data	= new char[data_size];
	m_max_data_size	= data_size;
	m_cursor = m_data + PACKET_CONTENT_POS;
	m_datalen		= 0;
	setCmd(NLIBP_NULL);
	memset(m_data, 0, sizeof(NET_PACKET_HEADER));
}

NET_Packet::~NET_Packet()
{
	if(m_data)
	{
		delete[] m_data;
	}
}

int NET_Packet::send_to(TCP_Session* pSession)
{
	end();
	return ::send(pSession->get_SOCKET(), m_data, m_cursor - m_data, 0);
}
void NET_Packet::begin()		
{
	m_cursor = m_data + PACKET_CONTENT_POS;
}
void NET_Packet::end()	
{
	if(getLen() < m_cursor-m_data)
		((NET_PACKET_HEADER *) m_data)->len 
			= ntohs(m_cursor-m_data);
}

int NET_Packet::recv(SOCKET socket)
{
	int len = ::recv(socket, 
		m_data + m_datalen, 
		m_max_data_size - m_datalen, 
		0
		);
	if (len > 0)
		begin();
	m_datalen += len;
	return len;
}

void NET_Packet::reset()
{
	m_cursor = m_data + PACKET_CONTENT_POS;
	m_datalen		= 0;
	setCmd(NLIBP_NULL);
	memset(m_data, 0, sizeof(NET_PACKET_HEADER));
}

bool NET_Packet::is_cling()
{
	if(m_datalen>=sizeof(NET_PACKET_HEADER) && m_datalen>=getLen())
		return true;
	else 
		return false;
}


NET_Packet* NET_Packet::get_cling_packet()
{
	if(m_datalen<=getLen())
		return NULL;
	int size_cling = m_datalen - getLen();
	NET_Packet*	pRet	= new NET_Packet;
	memcpy(pRet->m_data, (void*)(m_data + getLen()), size_cling);
	pRet->m_datalen	= size_cling;
	return pRet;

}