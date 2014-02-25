#include "PeerManager.h"
#include "YYException.h"

NS_YY_BEGIN

static UI64 s_serial=0;
PeerManager::PeerManager()
{
	peer_array_=new peer[MAX_PEER_NUM];
}

PeerManager::~PeerManager()
{}

UI32 PeerManager::getFreeIndex()
{
	for(UI32 i=0; i<MAX_PEER_NUM; i++)
	{
		if(peer_array_[i].socket!=-1)
			continue;

		peer_array_[i].serial=s_serial++;
		peer_array_[i].read_buf.reset();
		peer_array_[i].write_buf.reset();

		return i;
	}

	return -1;
}

peer* PeerManager::getPeer(UI32 index)
{
	if(index>=MAX_PEER_NUM)
		return NULL;

	return &peer_array_[index];
}

NS_YY_END