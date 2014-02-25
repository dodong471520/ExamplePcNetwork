#ifndef _PEER_MANAGER_HPP_
#define _PEER_MANAGER_HPP_

#include "SockBuf.h"
#include <memory.h>


#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	#include <windows.h>
#endif

#if (YY_TARGET_PLATFORM==YY_PLATFORM_LINUX)
	#define INVALID_SOCKET -1
#endif

#define MAX_PEER_NUM 1024*10	//最大连接数

NS_YY_BEGIN

struct peer
{
	int socket;
	char local_ip[50];
	UI32 local_port;
	char remote_ip[50];
	UI32 remote_port;

	UI64 serial;				//序列号，每次新连接新生成的序列号，session key的效果
	bool wsa_send;				//保证一个socket只有一个send投递, recv只有io thread操作，所以可以保证只有一个recv投递

	SockBuf read_buf;
	SockBuf write_buf;			//wsa buf, 投递之后操作系统会锁定内存，所以投递之后就不能修改缓冲内容
	SockBuf write_buf_backup_;	//如果已经投递写请求，则该缓存保存要发送的数据

	peer()
	{
		wsa_send=false;
		socket=INVALID_SOCKET;
		memset(local_ip, 0, sizeof(local_ip));
		local_port=0;
		memset(remote_ip, 0, sizeof(remote_ip));
		remote_port=0;
		serial=0;
	}
};

class PeerManager
{
public:
	PeerManager();
	~PeerManager();

	UI32 getFreeIndex();
	peer* getPeer(UI32 index);

private:
	peer* peer_array_;
};

NS_YY_END
#endif