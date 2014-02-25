/************************************************************************/
/*
@author:  junliang
@brief:   win select io(select, iocp), 用预处理命令来选择
			默认是iocp模式, 有预处理命令IO_SELECT,则是select模式
*/
/************************************************************************/
#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_


#include "sockbuf.h"
#include "FastDelegate.h"
using namespace fastdelegate;
NS_YY_BEGIN



//判断网络模式
#ifdef IO_SELECT
	class SelectIOService;
	typedef SelectIOService IOServiceImpl;
#else
	class Win_IOCP_IOService;
	typedef Win_IOCP_IOService IOServiceImpl;
#endif


//回调接口, 只有io thread会跑这些函数，即线程安全
typedef FastDelegate4<UI32, UI64, const char*, UI32> OnConCallback;
typedef FastDelegate4<UI32, UI64, const char*, UI32> OnDisConCallback;
typedef FastDelegate3<UI32, UI64, SockBuf*, int> OnReadCallback;


class IOService
{
public:
	IOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead);
	~IOService();

	//阻塞connect
	bool connectPeer(const char* ip, UI32 port);

	//listen for the peer connect
	void listen(const char* ip, UI32 port);

	//send data to peer, 如果连接已断开，则丢弃该消息
	void send(UI32 index, UI64 serial, const char* buf, UI32 len);

	//close peer,套接字上不能再发出发送或接收请求；发送缓冲中的内容被发送到对端。接收缓冲区的内容被丢弃。如果描述符引用计数为0：
	//在发送完发送缓冲区中的数据后，跟以正常的tcp连接终止序列(即发送FIN)
	void closePeer(UI32 index, UI64 serial);

	//参数count是一帧最多可以接受或者发送多少次网络包，好处是让帧率稳定，不会导致一帧时间过长。
	//参数timeout是控制一次网络事件的超时时间
	void eventLoop(UI32 count=1024, UI32 time_out=1000);
private:
	IOServiceImpl* m_impl;
};

NS_YY_END
#endif