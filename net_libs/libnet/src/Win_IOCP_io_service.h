/************************************************************************/
/*
@author:  junliang
@brief:   iocp io, 单线程
*/
/************************************************************************/
#ifndef _WIN_IOCP_IO_SERVICE_H_
#define _WIN_IOCP_IO_SERVICE_H_

#include "socket_wrap.h"
//#include <mswsock.h>
#include "yy_freelist.h"
#include "sockbuf.h"
#include "peer_manager.h"
#include "io_service.h"
NS_YY_BEGIN


enum IO_OPERATION
{
	ClientIoUnknown,
	ClientIoRead,
	ClientIoWrite,
};

//每次wsasend, wsarecv都应该提交一个新的overlap,存放本次操作
//频繁分配释放内存，用freelist
struct IOContext
{
	OVERLAPPED overlapped;		//一定要在第一位，强转用
	IO_OPERATION io_operation;
};


class Win_IOCP_IOService
{
public:
	Win_IOCP_IOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead);
	~Win_IOCP_IOService();

	void eventLoop(UI32 count=1024, UI32 time_out=1000);

	bool connectPeer(const char* ip, UI32 port);

	void listen(const char* ip, UI32 port);

	void send(UI32 index, UI64 serial, const char* buf, UI32 len);

	void closePeer(UI32 index, UI64 serial);
private:
	bool onEventTick(UI32 time_out);

	//检查是否有新连接
	void onAcceptTick();

	//把指定socket绑定到完成端口上
	void updateCompletionPort(SOCKET socket, UI32 index);

	//投递请求
	void postRecvRequest(UI32 index, UI64 serial);
	void postSendRequest(UI32 index, UI64 serial);

	//服务端有新连接或者客户端连接成功后，注册该socket到完成队列
	void registerSocket(SOCKET sockfd);

	//创建销毁io context
	IOContext* newIOCxt(IO_OPERATION type);
	void deleteIOCxt(IOContext* io_cxt);

	void processWrite(UI32 index, UI32 size);
	void processRead(UI32 index, UI32 size);
private:
	//accept event
	SOCKET m_listen_sd;
	fd_set m_read_set;		//读队列

	HANDLE m_iocp_handle;
	FreeList<IOContext> m_iocxt_pool;		//每次发送接收消息都要投递io context,所以该结构体用freelist来提高内存使用率

	//网络事件
	OnConCallback m_onConCallback;
	OnDisConCallback m_onDisConCallback;
	OnReadCallback m_onReadCallback;

	PeerManager m_peer_manager;			//所有连接节点的管理。数组+serial标志位来保证唯一性(用new, delete的话，可能会收到多次断开通知，导致重复删除，重复通知等)
};

NS_YY_END
#endif