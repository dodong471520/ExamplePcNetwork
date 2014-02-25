#include "peer_manager.h"
#include "yy_exception.h"
NS_YY_BEGIN

PeerManager::PeerManager():m_serial(0)
{
	m_peer_array=new peer[MAX_PEER_NUM];
}

PeerManager::~PeerManager()
{
	if(NULL != m_peer_array)
		delete []m_peer_array;
}

UI32 PeerManager::newPeer()
{
	for(UI32 i=0; i<MAX_PEER_NUM; i++)
	{
		if(m_peer_array[i].socket!=INVALID_SOCKET)
			continue;

		m_peer_array[i].serial=m_serial++;
		m_peer_array[i].socket=INVALID_SOCKET;
		m_peer_array[i].read_buf.reset();
		m_peer_array[i].write_buf.reset();
		m_peer_array[i].write_buf_backup_.reset();
		m_peer_array[i].wsa_send=false;

		memset(m_peer_array[i].local_ip, 0, sizeof(m_peer_array[i].local_ip));
		m_peer_array[i].local_port=0;
		memset(m_peer_array[i].remote_ip, 0, sizeof(m_peer_array[i].remote_ip));
		m_peer_array[i].remote_port=0;

		return i;
	}

	return -1;
}

void PeerManager::deletePeer(UI32 index)
{
	peer* p=getPeer(index);
	if(NULL != p)
	{
		p->socket=INVALID_SOCKET;
	}
}

peer* PeerManager::getPeer(UI32 index)
{
	if(index >= MAX_PEER_NUM)
		THROWNEXCEPT("index:%u is out of range:%u", index, MAX_PEER_NUM);

	return &m_peer_array[index];
}
NS_YY_END