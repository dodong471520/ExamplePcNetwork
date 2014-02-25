/************************************************************************/
/*
@author:  junliang
@brief:   select io interface
*/
/************************************************************************/
#ifndef _SELECT_IO_SERVICE_H_
#define _SELECT_IO_SERVICE_H_

#include "SocketWrap.h"
#include "PeerManager.h"
#include <vector>
#include "IOService.h"
NS_YY_BEGIN

//select io, 单线程
class SelectIOService
{
public:
	SelectIOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead);
	~SelectIOService();

	//-1:正常结束或者有错误发生而结束； 0:没数据; 1:有数据
	void eventLoop(UI32 count=1024, UI32 time_out=1000);

	bool connectPeer(const char* ip, UI32 port);

	void listen(const char* ip, UI32 port);

	void send(UI32 index, UI64 serial, const char* buf, UI32 len);

	void closePeer(UI32 index, UI64 serial);

private:
	bool onEventTick(UI32 time_out);
	void initSets();

	//有个新连接后，对socket的一些初始化
	void registerSocket(SOCKET sockfd);
private:
	SOCKET m_listen_sd;

	fd_set m_read_set;		//读队列
	fd_set m_write_set;		//写队列

	//事件接口
	OnConCallback m_onConCallback;
	OnDisConCallback m_onDisConCallback;
	OnReadCallback m_onReadCallback;

	std::vector<UI32> m_online_peers;
	PeerManager m_peer_manager;
};

NS_YY_END
#endif
