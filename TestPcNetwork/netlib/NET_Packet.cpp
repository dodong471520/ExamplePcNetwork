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


void NET_Packet::write8(uint8 b)
{
	if (m_cursor <= m_data + m_max_data_size - sizeof(b)) 
	{
		*(uint8 *) m_cursor = b;
		m_cursor += sizeof(b);
	}
}

void NET_Packet::write16(uint16 w)
{
	if (m_cursor <= m_data + m_max_data_size - sizeof(w)) 
	{
		*(uint16 *) m_cursor = htons(w);
		m_cursor += sizeof(w);
	}
}

void NET_Packet::write32(uint32 dw)
{
	if (m_cursor <= m_data + m_max_data_size - sizeof(dw)) 
	{
		*(uint32 *) m_cursor = htonl(dw);
		m_cursor += sizeof(dw);
	}
}

void NET_Packet::writeString(const char *str)
{
	uint16 len = strlen(str) + 1;
	if (m_cursor <= m_data + m_max_data_size - sizeof(len) - len) 
	{
		write16(len);
		strcpy(m_cursor, str);
		m_cursor += len;
	}
}


void NET_Packet::writeData(void* pData, uint16 len)
{
	if (m_cursor <= m_data + m_max_data_size - sizeof(len) - len) 
	{
		write16(len);
		memcpy(m_cursor, pData, len);
		m_cursor += len;
	}
}


uint8 NET_Packet::read8()
{
	uint8 b = 0;
	if (m_cursor <= m_data + m_datalen  - sizeof(uint8)) 
	{
		b = *m_cursor;
		m_cursor += sizeof(b);
	}
	return b;
}

uint16 NET_Packet::read16()
{
	uint16 w = 0;
	if (m_cursor <= m_data + m_datalen - sizeof(w)) 
	{
		w = ntohs(*(uint16 *) m_cursor);
		m_cursor += sizeof(w);
	}
	return w;
}

uint32 NET_Packet::read32()
{
	uint32 dw = 0;
	if (m_cursor <= m_data + m_datalen - sizeof(dw)) 
	{
		dw = ntohl(*(uint32 *) m_cursor);
		m_cursor += sizeof(uint32);
	}
	return dw;
}

const char *NET_Packet::readString()
{
	const char *str = NULL;
	uint16 len = read16();
	if (m_cursor <= m_data + m_datalen - len && !m_cursor[len - 1]) 
	{
		str = m_cursor;
		m_cursor += len;
	}
	return str;
}

void* NET_Packet::readData()
{
	uint16 len = read16();
	void* str = NULL;
	if (m_cursor <= m_data + m_datalen - len) 
	{
		str = m_cursor;
		m_cursor += len;
	}
	return str;
}

_PACKET_DATA NET_Packet::readBinary()
{
	_PACKET_DATA	ret;
	ret.data_len = read16();
	if (m_cursor <= m_data + m_datalen - ret.data_len) 
	{
		ret.ptr_data = m_cursor;
		m_cursor += ret.data_len;
	}
	return ret;

}
