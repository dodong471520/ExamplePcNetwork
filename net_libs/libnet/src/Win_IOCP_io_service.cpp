#include "Win_IOCP_io_service.h"
#include <string>
#include "yy_log.h"
NS_YY_BEGIN

Win_IOCP_IOService::Win_IOCP_IOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead)
	:m_iocp_handle(NULL), m_onConCallback(onCon), m_onDisConCallback(onDis), m_onReadCallback(onRead), m_listen_sd(INVALID_SOCKET)
{

	NetInit();

	//创建iocp句柄
	m_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if( m_iocp_handle == NULL )
		THROWNEXCEPT("CreateIoCompletionPort error.errno:%d", GetLastError());
}

Win_IOCP_IOService::~Win_IOCP_IOService()
{
	//清理句柄
	if(m_iocp_handle)
	{
		PostQueuedCompletionStatus(m_iocp_handle, 0, 0, NULL);
		CloseHandle(m_iocp_handle);
		m_iocp_handle = NULL;
	}

	NetDestroy();
}

void Win_IOCP_IOService::updateCompletionPort(SOCKET sd, UI32 index)
{
	//新连接句柄和完成端口绑定
	m_iocp_handle = CreateIoCompletionPort((HANDLE)sd, m_iocp_handle, (DWORD_PTR)index, 0);
	if( m_iocp_handle == NULL )
		THROWNEXCEPT("CreateIoCompletionPort error.errno:%d", GetLastError());
}

void Win_IOCP_IOService::postRecvRequest(UI32 index, UI64 serial)
{
	peer* p=m_peer_manager.getPeer(index);
	if(p->serial!=serial || p->socket==INVALID_SOCKET)
		return;

	//投递读请求
	IOContext* io_cxt=newIOCxt(ClientIoRead);

	DWORD dwRecvNumBytes = 0;
	DWORD dwFlags = 0;

	p->read_buf.ensureWritableBytes(512);

	WSABUF wsabuf;
	wsabuf.buf=p->read_buf.writeStart();
	wsabuf.len=p->read_buf.writableBytes();


	int nRet = WSARecv(p->socket, &wsabuf, 1, &dwRecvNumBytes, &dwFlags, &io_cxt->overlapped, NULL);
	if( nRet == SOCKET_ERROR)
	{
		int nErrNo=WSAGetLastError();
		if(ERROR_IO_PENDING == nErrNo)
			return;

		//WSAECONNRESET, 对端关闭连接; WSAEWOULDBLOCK, 可能有多重错误，统一关闭该连接
		LOG_WARN("WSARecv error. errno:%d", nErrNo);
		closePeer(index, serial);
	}
}

void Win_IOCP_IOService::postSendRequest(UI32 index, UI64 serial)
{
	peer* socket_p=m_peer_manager.getPeer(index);
	if(socket_p->serial!=serial || socket_p->socket==INVALID_SOCKET)
		return;

	socket_p->wsa_send=true;

	DWORD dwSendNumBytes = 0;
	DWORD dwFlags = 0;

	IOContext* io_p=newIOCxt(ClientIoWrite);

	WSABUF wsabuf;
	wsabuf.buf=socket_p->write_buf.readStart();
	wsabuf.len=socket_p->write_buf.readableBytes();

	int nRet = WSASend(socket_p->socket, &wsabuf, 1, &dwSendNumBytes, dwFlags, (LPWSAOVERLAPPED)io_p, NULL);
	if( nRet == SOCKET_ERROR)
	{
		int nErrNo=WSAGetLastError();
		if(ERROR_IO_PENDING == nErrNo)
			return;

		//WSAECONNRESET, 对端关闭连接; WSAEWOULDBLOCK, 可能有多重错误，统一关闭该连接
		LOG_WARN("WSASend error. errno:%d", nErrNo);
		closePeer(index, serial);
	}
}


bool Win_IOCP_IOService::connectPeer(const char* ip, UI32 port)
{
	//create socket
	SOCKET socketfd =ConnectSocket(m_listen_sd, ip, port);
	if(socketfd == INVALID_SOCKET)
		return false;

	//注册
	registerSocket(socketfd);
	return true;
}

void Win_IOCP_IOService::listen(const char* ip, UI32 port)
{
	ListenSocket(m_listen_sd, ip, port);
}

void Win_IOCP_IOService::onAcceptTick()
{
	if(m_listen_sd == INVALID_SOCKET)
		THROWNEXCEPT("no socket is listen.");

	//init
	FD_ZERO(&m_read_set);

	//添加监听句柄到read array,如果有连接进来，则响应
	FD_SET(m_listen_sd, &m_read_set);

	//select accept event
	timeval timeout;
	timeout.tv_sec=0;		//seconds
	timeout.tv_usec=0;		//micro seconds
	int ready_socket_num = select(	0,					//忽略
									&m_read_set,		//指向读队列的指针
									NULL,
									NULL,
									&timeout);			//超时

	if(ready_socket_num == -1)
		THROWNEXCEPT("select erro.errno:%d", WSAGetLastError());

	//new connection
	if(ready_socket_num>0 && FD_ISSET(m_listen_sd, &m_read_set))
	{
		//如果有新连接，则用非阻塞io去accept,有可能失败
		SOCKET sd;
		if(AcceptSocket(m_listen_sd, sd))
			registerSocket(sd);
	}
}

void Win_IOCP_IOService::send(UI32 index, UI64 serial, const char* buf, UI32 len)
{
	peer* p=m_peer_manager.getPeer(index);
	if(p->serial!=serial || p->socket==INVALID_SOCKET)
		return;

	//把要发送的数据放到备用缓冲中
	p->write_buf_backup_.apendBuf(buf, len);

	//如果当前有数据没有数据在发送，则发送一个信号；如果有数据在发送，则在数据发送完毕后会检查备用缓冲
	if(!p->wsa_send)
	{
		p->wsa_send=true;
		IOContext* io_p=newIOCxt(ClientIoWrite);
		if(!PostQueuedCompletionStatus(m_iocp_handle, 0, (ULONG_PTR)index, (LPOVERLAPPED)io_p))
			THROWNEXCEPT("post erro. errnor:%d", GetLastError());
	}
}

void Win_IOCP_IOService::closePeer(UI32 index, UI64 serial)
{
	peer* p=m_peer_manager.getPeer(index);
	if(!p || p->serial!=serial || p->socket==INVALID_SOCKET)
		return;

	if(m_onDisConCallback)
		m_onDisConCallback(index, serial, p->remote_ip, p->remote_port);

	//让GetQueuedCompletionStatus返回false
	CloseSocket(p->socket);
	m_peer_manager.deletePeer(index);
	p=NULL;

}


IOContext* Win_IOCP_IOService::newIOCxt(IO_OPERATION type)
{
	IOContext* io_cxt=m_iocxt_pool.allocElem();
	memset(io_cxt, 0, sizeof(OVERLAPPED));
	io_cxt->io_operation=type;
	return io_cxt;
}

void Win_IOCP_IOService::deleteIOCxt(IOContext* io_cxt)
{
	m_iocxt_pool.freeElem(io_cxt);
}

void Win_IOCP_IOService::registerSocket(SOCKET sd)
{

	UI32 index=m_peer_manager.newPeer();
	peer* p=m_peer_manager.getPeer(index);

	//绑定到完成端口
	updateCompletionPort(sd, index);

	//获得ip信息
	struct sockaddr_in remote;
	int remote_len = sizeof(remote);
	getpeername(sd, (struct sockaddr *)&remote, &remote_len);
	strcpy_s(p->remote_ip, sizeof(p->remote_ip), inet_ntoa(remote.sin_addr));
	p->remote_port=ntohs(remote.sin_port);

	struct sockaddr_in local;
	int local_len = sizeof(local);
	getsockname(sd, (struct sockaddr *)&local, &local_len);
	strcpy_s(p->local_ip, sizeof(p->local_ip), inet_ntoa(local.sin_addr));
	p->local_port=ntohs(local.sin_port);

	p->socket=sd;

		//调用回调函数
	if(m_onConCallback)
	{
		m_onConCallback(index, p->serial, p->remote_ip, p->remote_port);
	}

	//投递读请求
	postRecvRequest(index, p->serial);
}


void Win_IOCP_IOService::processWrite(UI32 index, UI32 size)
{
	peer* p=NULL;
	p=m_peer_manager.getPeer(index);
	if(p->socket==INVALID_SOCKET)
		return;

	//投递的请求得到回应。
	p->wsa_send=false;

	//清空已发送的数据
	p->write_buf.readMove(size);

	UI32 send_bytes=p->write_buf_backup_.readableBytes();
	if(send_bytes > 0)
	{
		//从被用写缓冲中获取数据
		p->write_buf.apendBuf(p->write_buf_backup_.readStart(), send_bytes);
		p->write_buf_backup_.readMove(send_bytes);
	}

	//如果还有数据要发送，则继续投递
	if(p->write_buf.readableBytes() > 0)
		postSendRequest(index, p->serial);
}

void Win_IOCP_IOService::processRead(UI32 index, UI32 size)
{
	peer* p=NULL;

	p=m_peer_manager.getPeer(index);
	if(p->socket==INVALID_SOCKET)
		return;

	//保存收到的数据
	p->read_buf.writeMove(size);

	//通知应用层--读事件
	if(m_onReadCallback)
	{
		int ret=m_onReadCallback(index, p->serial, &p->read_buf);
		if(ret<0)
		{
			//断开连接
			closePeer(index, p->serial);
			return;
		}
	}

	//继续投递读请求
	postRecvRequest(index, p->serial);
}

void Win_IOCP_IOService::eventLoop(UI32 count/* =1024 */, UI32 time_out/* =1000 */)
{
	for(UI32 i=0; i<count; i++)
	{
		if(m_listen_sd!=INVALID_SOCKET)
			onAcceptTick();

		//没有数据，或者正常退出，则退出该循环
		if(!onEventTick(time_out))
			break;
	}
}

bool Win_IOCP_IOService::onEventTick(UI32 time_out)
{
	IOContext* io_context=NULL;
	peer* p=NULL;
	int index=-1;

	LPOVERLAPPED* lpOverlapped = NULL;
	BOOL bSuccess = FALSE;
	int nRet = 0;
	DWORD dwRecvNumBytes = 0;
	DWORD dwSendNumBytes = 0;
	DWORD dwFlags = 0;
	DWORD dwIoSize = 0;		//传输的字节数

	bSuccess = GetQueuedCompletionStatus(m_iocp_handle, &dwIoSize,(PDWORD_PTR)&index, (LPOVERLAPPED *)&lpOverlapped, time_out);
	if(!bSuccess)
	{
		//time out
		DWORD err_num=GetLastError();
		if(err_num==WAIT_TIMEOUT)
		{
			//只有超时的时候返回,说明暂时没有网络数据
			return false;
		}

		//NULL == lpOverlapped, 则lpNumberOfBytes and lpCompletionKey 都未被初始化；
		//NULL != lpOverlapped, 则lpNumberOfBytes and lpCompletionKey 有被初始化

		//client close socket
		//wsasend/wsarecv are not finished, and the socket is cancelled.
		if(err_num==ERROR_NETNAME_DELETED &&NULL != lpOverlapped)
		{
			p=m_peer_manager.getPeer(index);
			closePeer(index, p->serial);
			io_context=(IOContext*)lpOverlapped;
			deleteIOCxt(io_context);
			return true;
		}


		//有错误直接崩溃
		THROWNEXCEPT("shoud not be here.");
		if(NULL == lpOverlapped && err_num == ERROR_ABANDONED_WAIT_0)
		{
			//error
		}
	}


	//对完成消息的处理
	p=m_peer_manager.getPeer(index);
	if(p->socket==INVALID_SOCKET)
	{
		//该socket已经断开
		return true;
	}

	io_context=(IOContext*)lpOverlapped;

	//程序退出的时候,发来的通告
	if(!lpOverlapped && 0 == index && 0 == dwIoSize)
	{
		closePeer(index, p->serial);
		if(io_context)
			deleteIOCxt(io_context);

		return false;
	}

	//判断具体消息
	switch(io_context->io_operation)
	{
	case ClientIoRead:

		//peer shutdown 
		if(0 == dwIoSize)
		{
			closePeer(index, p->serial);
		}
		else
			processRead(index, dwIoSize);
		break;

		//send post, 0 if sending
	case ClientIoWrite:
		processWrite(index, dwIoSize);
		break;

	default:
		THROWNEXCEPT("should not be here.");
		break;
	}

	if(io_context)
	{
		deleteIOCxt(io_context);
	}

	return true;
}

NS_YY_END
