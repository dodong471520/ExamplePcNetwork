#include "Select_IOService.h"
#include "YYException.h"
#include <algorithm>

NS_YY_BEGIN


SelectIOService::SelectIOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead)
	:onConCallback_(onCon), onDisConCallback_(onDis), onReadCallback_(onRead)
{
	listen_socket_=INVALID_SOCKET;
	memset(listen_ip_, 0, sizeof(listen_ip_));
	listen_port_=0;

	open();


	FD_ZERO( &read_set_[SELECT_BAK] ) ;
	FD_ZERO( &write_set_[SELECT_BAK] ) ;
	FD_ZERO(&read_set_[SELECT_USE]);
	FD_ZERO(&write_set_[SELECT_USE]);
}

SelectIOService::~SelectIOService()
{
	close();
}

int SelectIOService::open()
{
#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	//包含winsock版本的信息
	WSADATA wsaData;

	//准备加载的Winsock库的版本号，目前win32平台的Winsock2库的最新版本是2.2
	WORD wVersionRequested=MAKEWORD(2,2);

	int err=WSAStartup(wVersionRequested, &wsaData);
	if(err != 0)
		return -1;
#endif

	return 0;
}

int SelectIOService::close()
{
#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	//卸载winsock2库，并释放资源
	WSACleanup();
#endif

	return 0;
}


bool SelectIOService::connectPeer(const char* ip, UI32 port)
{
	int socketfd=Connect(ip, port);
	if(socketfd==-1)
		return false;

	registerSocket(socketfd, ip, port);
	return true;
}

void SelectIOService::listen(const char* ip, UI32 port)
{
	//wsasocket要手动指定overlap, socket默认是overlap
	listen_socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket_ == INVALID_SOCKET)
	{
		THROWNEXCEPT("Error at socket(): %ld\n", WSAGetLastError());
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(ip);
	service.sin_port = htons(port);

	if (::bind( listen_socket_, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR)
	{
		THROWNEXCEPT("bind failed.");
	}

	//关闭发送缓冲,必须在listen之前设置,这样wsasend的时候，wsabuf指向用户缓冲区，内存锁定。
	int nZero=0;
	if(setsockopt(listen_socket_, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero))== SOCKET_ERROR)
	{
		THROWNEXCEPT("setsockopt(SNDBUF) failed: %d\n", WSAGetLastError());
	}

	if( ::listen(listen_socket_, 5) == SOCKET_ERROR )
	{
		THROWNEXCEPT("listen() failed: %d\n", WSAGetLastError());
	}
}


void SelectIOService::send(UI32 index, UI64 serial, const char* buf, UI32 len)
{
	//判断合法性
	peer* p=peer_manager_.getPeer(index);
	if(p && p->serial!=serial || p->socket<=0)
		return;

	//insert into send buffer
	p->write_buf.push(buf, len);

	//保存要发送数据的socket
	FD_SET(p->socket, &write_set_[SELECT_BAK]);
}

void SelectIOService::closePeer(UI32 index, UI64 serial)
{
	peer* p=peer_manager_.getPeer(index);

	//1,已经关闭且状态改过。2，已经关闭，且被别的连接占用
	if(p->serial!=serial || p->socket==INVALID_SOCKET)
		return;

	if(onDisConCallback_)
		onDisConCallback_(index, serial, p->remote_ip, p->remote_port);

	std::vector<UI32>::iterator itor=find(online_peer_set_.begin(), online_peer_set_.end(), index);
	if(itor!=online_peer_set_.end())
		online_peer_set_.erase(itor);

	p->socket=INVALID_SOCKET;
	CloseSocket(p->socket);

	//玩家退出，则把socket从备份队列删除，避免下次监听
	FD_CLR(p->socket, &read_set_[SELECT_BAK]);

	//把socket从当前队列删除
	FD_CLR(p->socket, &read_set_[SELECT_USE]);
}

void SelectIOService::registerSocket(int socket, const char* ip, UI32 port)
{
	UI32 index=peer_manager_.getFreeIndex();
	peer* p=peer_manager_.getPeer(index);

	struct sockaddr_in remote;
	char remote_ip[50];
	int remote_len = sizeof(remote);
	getpeername(socket, (struct sockaddr *)&remote, &remote_len);
	strcpy(remote_ip, inet_ntoa(remote.sin_addr));

	strcpy_s(p->remote_ip, sizeof(p->remote_ip), remote_ip);
	p->remote_port=ntohs(remote.sin_port);

	struct sockaddr_in local;
	char local_ip[50];
	int local_len = sizeof(local);
	getsockname(socket, (struct sockaddr *)&local, &local_len);
	strcpy(local_ip, inet_ntoa(local.sin_addr));

	strcpy_s(p->local_ip, sizeof(p->local_ip), local_ip);
	p->local_port=ntohs(local.sin_port);

	p->socket=socket;
	online_peer_set_.push_back(index);

	if(onConCallback_)
	{
		int ret=onConCallback_(index, p->serial, p->remote_ip, p->remote_port);
		if(ret<0)
		{
			closePeer(index, p->serial);
			return;
		}
	}

	//保存所有在线的socket
	FD_SET(p->socket, &read_set_[SELECT_BAK]);
}


void SelectIOService::eventLoop()
{
	peer* tmp=NULL;

	//初始化句柄
	initSets();

	//线程阻塞5秒后，如果没消息，则返回。不设置null一直阻塞是为了防止进程退出的时候该io线程无法返回
	timeval timeout;
	timeout.tv_sec=5;
	timeout.tv_usec=0;

	//遍历所有socket
	int ready_socket_num = select(	0,					//忽略
									&read_set_[SELECT_USE],			//指向读队列的指针
									&write_set_[SELECT_USE],		//指向写队列的指针
									NULL,
									&timeout);			//超时

	//如果有socket的状态发生变化
	if(ready_socket_num>0)
	{

		if(listen_socket_ != INVALID_SOCKET)
		{
			//有新连接
			if(FD_ISSET(listen_socket_, &read_set_))
			{
				sockaddr_in clientAddr;
				int nClientLen = sizeof(sockaddr_in);

				SOCKET socket=accept(listen_socket_, (sockaddr*)&clientAddr, &nClientLen);
				if(socket != INVALID_SOCKET)
				{
					registerSocket(socket, listen_ip_, listen_port_);
				}
				else
				{
					//连接失败，客户端自己处理
					int err_no=GetLastError();
					THROWNEXCEPT("accept error. errno:%d", err_no);
				}
			}

			//出错
			//if(FD_ISSET(listen_socket_, &exception_set_))
			//{
			//	LOG_ERR("listen socket exception:%d", err_no);
				//continue;
			//}
		}

		for(size_t i=0; i<online_peer_set_.size(); i++)
		{
			UI32 index=online_peer_set_[i];
			peer* p=peer_manager_.getPeer(index);

			//读事件
			if(FD_ISSET(p->socket, &read_set_))
			{
				int ret=p->read_buf.sockRead(p->socket);
				if(ret!=0)
				{
					//断开连接
					closePeer(index, p->serial);
					continue;
				}

				if(onReadCallback_(index, p->serial, &p->read_buf) < 0)
				{
					//断开连接
					closePeer(index, p->serial);
					continue;
				}
			}

			//写事件
			if(FD_ISSET(p->socket, &write_set_))
			{
				int ret=p->write_buf.sockWrite(p->socket);
				if(ret != 0)
				{
					closePeer(index, p->serial);
					continue;
				}

				if(p->write_buf.readableBytes() == 0)
				{
					FD_CLR(p->socket, &write_set_[SELECT_BAK]);
					FD_CLR(p->socket, &write_set_[SELECT_USE]);
				}
			}
		}
	}
	else if(-1 ==ready_socket_num)
	{
		//select error
		printf("Error at socket(): %ld\n", WSAGetLastError());
		THROWNEXCEPT("Error at socket(): %ld\n", WSAGetLastError());
	}
	else
	{
		//timeout
	}
}

int SelectIOService::initSets()
{
	//三个句柄队列的句柄个数清空
	FD_ZERO(&read_set_[SELECT_USE]);
	FD_ZERO(&write_set_[SELECT_USE]);

	if(listen_socket_ != INVALID_SOCKET)
	{
		//添加监听句柄到read array,如果有连接进来，则响应
		FD_SET(listen_socket_, &read_set_);
		//FD_SET(listen_socket_, &exception_set_);
	}

	read_set_[SELECT_USE]=read_set_[SELECT_BAK];
	write_set_[SELECT_USE]=write_set_[SELECT_BAK];
	return 0;
}
NS_YY_END
