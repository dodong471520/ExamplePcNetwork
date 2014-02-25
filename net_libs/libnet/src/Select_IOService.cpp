#include "Select_IOService.h"
#include <algorithm>
#include "YYException.h"
#include "YYLog.h"
NS_YY_BEGIN

SelectIOService::SelectIOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead)
	:m_onConCallback(onCon), m_onDisConCallback(onDis), m_onReadCallback(onRead), m_listen_sd(INVALID_SOCKET)
{
	NetInit();
}

SelectIOService::~SelectIOService()
{
	NetDestroy();
}

void SelectIOService::registerSocket(SOCKET socket)
{
	UI32 index=m_peer_manager.newPeer();
	peer* p=m_peer_manager.getPeer(index);

	//获得ip信息
	struct sockaddr_in remote;
	int remote_len = sizeof(remote);
	getpeername(socket, (struct sockaddr *)&remote, &remote_len);
	strcpy_s(p->remote_ip, sizeof(p->remote_ip), inet_ntoa(remote.sin_addr));
	p->remote_port=ntohs(remote.sin_port);

	struct sockaddr_in local;
	int local_len = sizeof(local);
	getsockname(socket, (struct sockaddr *)&local, &local_len);
	strcpy_s(p->local_ip, sizeof(p->local_ip), inet_ntoa(local.sin_addr));
	p->local_port=ntohs(local.sin_port);

	//初始化
	p->socket=socket;
	m_online_peers.push_back(index);

	if(m_onConCallback)
	{
		m_onConCallback(index, p->serial, p->remote_ip, p->remote_port);
	}
}

bool SelectIOService::connectPeer(const char* ip, UI32 port)
{
	//create socket
	SOCKET socketfd = ConnectSocket(m_listen_sd, socketfd, ip, port);
	if(socketfd == INVALID_SOCKET)
		return false;

	registerSocket(socketfd);
	return true;
}

void SelectIOService::listen(const char* ip, UI32 port)
{
	ListenSocket(m_listen_sd, ip, port);
}


void SelectIOService::send(UI32 index, UI64 serial, const char* buf, UI32 len)
{
	//判断合法性,有可能该连接已断开
	peer* p=m_peer_manager.getPeer(index);
	if(p && p->serial!=serial || p->socket<=0)
		return;

	//insert into send buffer
	p->write_buf.apendBuf(buf, len);
}

void SelectIOService::closePeer(UI32 index, UI64 serial)
{
	peer* p=m_peer_manager.getPeer(index);

	//1,已经关闭且状态改过。2，已经关闭，且被别的连接占用
	if(p->serial!=serial || p->socket==INVALID_SOCKET)
		return;

	if(m_onDisConCallback)
		m_onDisConCallback(index, serial, p->remote_ip, p->remote_port);

	std::vector<UI32>::iterator itor=find(m_online_peers.begin(), m_online_peers.end(), index);
	if(itor!=m_online_peers.end())
		m_online_peers.erase(itor);

	CloseSocket(p->socket);
	m_peer_manager.deletePeer(index);
	p=NULL;
}

void SelectIOService::eventLoop(UI32 count/* =1024 */, UI32 time_out/* =1000 */)
{
	for(UI32 i=0; i<count; i++)
	{
		//没有数据，或者正常退出，则退出该循环
		if(!onEventTick(time_out))
			break;
	}
}

bool SelectIOService::onEventTick(UI32 time_out)
{
	//如果没有句柄监听，则返回
	if(m_online_peers.empty() && m_listen_sd == INVALID_SOCKET)
		return false;

	peer* tmp=NULL;

	//初始化句柄
	initSets();

	//线程阻塞后，如果没消息，则返回。不设置null一直阻塞是为了防止进程退出的时候该io线程无法返回
	timeval timeout;
	timeout.tv_sec=time_out/1000;			//seconds
	timeout.tv_usec=(time_out%1000) * 1000;	//micro seconds

	//遍历所有socket
	int ready_socket_num = select(	0,					//忽略
									&m_read_set,		//指向读队列的指针
									&m_write_set,		//指向写队列的指针
									NULL,
									&timeout);			//超时

	int err_num=WSAGetLastError();
	if(ready_socket_num == -1)
		THROWNEXCEPT("select erro.errno:%d", WSAGetLastError());

	//如果有socket的状态发生变化
	if(ready_socket_num>0)
	{
		if(m_listen_sd != INVALID_SOCKET)
		{
			//有新连接
			if(FD_ISSET(m_listen_sd, &m_read_set))
			{
				SOCKET sd;
				if(AcceptSocket(m_listen_sd, sd))
					registerSocket(sd);
			}

			//出错
			//if(FD_ISSET(m_listen_sd, &exception_set_))
			//{
			//	LOG_ERR("listen socket exception:%d", err_no);
				//continue;
			//}
		}

		//如果有断开连接，则跳出该循环，防止m_online_peers错误，其余连接的数据可以等待下一次select
		for(size_t i=0; i<m_online_peers.size(); i++)
		{
			UI32 index=m_online_peers[i];
			peer* p=m_peer_manager.getPeer(index);

			//读事件
			if(FD_ISSET(p->socket, &m_read_set))
			{
				p->read_buf.ensureWritableBytes(512);

				int ret=0;
				ret= ::recv(p->socket, p->read_buf.writeStart(), p->read_buf.writableBytes(), 0);
				if(ret==0)
				{
					//对端调用closesocket api断开连接
					closePeer(index, p->serial);
					break;
				}

				if(ret<0)
				{
					//WSAECONNRESET, 对端关闭连接; WSAEWOULDBLOCK, 可能有多重错误，统一关闭该连接
					LOG_WARN("recv error. errno:%d", WSAGetLastError());
					closePeer(index, p->serial);
					break;
				}

				p->read_buf.writeMove(ret);
				if(m_onReadCallback(index, p->serial, &p->read_buf) < 0)
				{
					//断开连接
					closePeer(index, p->serial);
					break;
				}
			}

			//写事件
			if(FD_ISSET(p->socket, &m_write_set))
			{
				//写缓冲中有数据需要发送
				int readable_byets=p->write_buf.readableBytes();
				if(readable_byets>0)
				{
					int bytes=0;
					bytes=::send(p->socket, p->write_buf.readStart(), p->write_buf.readableBytes(), 0);
					if(bytes<0)
					{
						//WSAECONNRESET, 对端关闭连接; WSAEWOULDBLOCK, 可能有多重错误，统一关闭该连接
						LOG_WARN("send error. errno:%d", WSAGetLastError());
						closePeer(index, p->serial);
						break;
					}

					p->write_buf.readMove(bytes);
				}
			}
		}
		
		return true;
	}
	else if(ready_socket_num == 0)
	{
		//timeout, 只有超时的时候返回false,说明当前没有网络数据了
		return false;
	}

	return false;
}

void SelectIOService::initSets()
{
	//三个句柄队列的句柄个数清空
	FD_ZERO(&m_read_set);
	FD_ZERO(&m_write_set);

	if(m_listen_sd != INVALID_SOCKET)
	{
		//添加监听句柄到read array,如果有连接进来，则响应
		FD_SET(m_listen_sd, &m_read_set);
		//FD_SET(m_listen_sd, &exception_set_);
	}

	std::vector<UI32>::iterator itor=m_online_peers.begin();
	for(; itor!=m_online_peers.end(); itor++)
	{
		UI32 index=*itor;
		peer* p=m_peer_manager.getPeer(index);

		//有数据要发送(tcp是全双工的，允许同时发送接收数据)
		if(p->write_buf.readableBytes()>0)
		{
			FD_SET(p->socket, &m_write_set);
		}

		//添加到read array
		FD_SET(p->socket, &m_read_set);
	}
}

NS_YY_END