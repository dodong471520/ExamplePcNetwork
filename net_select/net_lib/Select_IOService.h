#ifndef _SELECT_IO_SERVICE_H_
#define _SELECT_IO_SERVICE_H_

#include "YYMacros.h"
#include "SocketWrap.h"
#include "PeerManager.h"
#include <vector>
#include "FastDelegate.h"
using namespace fastdelegate;


NS_YY_BEGIN

//select io
class SelectIOService
{
private:
	typedef FastDelegate4<UI32, UI64, const char*, UI32, int> OnConCallback;
	typedef FastDelegate4<UI32, UI64, const char*, UI32> OnDisConCallback;
	typedef FastDelegate3<UI32, UI64, SockBuf*, int> OnReadCallback;

public:
	SelectIOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead);
	~SelectIOService();

	//io monitor
	void eventLoop();

	//connect to peer
	bool connectPeer(const char* ip, UI32 port);

	//listen for the peer connect
	void listen(const char* ip, UI32 port);

	//send data to peer
	void send(UI32 index, UI64 serial, const char* buf, UI32 len);

	//close peer
	void closePeer(UI32 index, UI64 serial);

private:
	int open();
	int close();
	int initSets();
	void registerSocket(int sockfd, const char* local_ip, UI32 local_port);
private:

	//fd_set数组方式，在select之前初始化的时候用到
	enum
	{
		SELECT_BAK,
		SELECT_USE,
		SELECT_MAX,
	};
	fd_set read_set_[SELECT_MAX];		//读队列
	fd_set write_set_[SELECT_MAX];		//写队列

	//事件接口
	OnConCallback onConCallback_;
	OnDisConCallback onDisConCallback_;
	OnReadCallback onReadCallback_;

	int listen_socket_;

	std::vector<UI32> online_peer_set_;
	char listen_ip_[50];
	UI32 listen_port_;
	PeerManager peer_manager_;
};

NS_YY_END
#endif
