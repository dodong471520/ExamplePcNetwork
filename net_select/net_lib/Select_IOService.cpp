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
	//����winsock�汾����Ϣ
	WSADATA wsaData;

	//׼�����ص�Winsock��İ汾�ţ�Ŀǰwin32ƽ̨��Winsock2������°汾��2.2
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
	//ж��winsock2�⣬���ͷ���Դ
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
	//wsasocketҪ�ֶ�ָ��overlap, socketĬ����overlap
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

	//�رշ��ͻ���,������listen֮ǰ����,����wsasend��ʱ��wsabufָ���û����������ڴ�������
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
	//�жϺϷ���
	peer* p=peer_manager_.getPeer(index);
	if(p && p->serial!=serial || p->socket<=0)
		return;

	//insert into send buffer
	p->write_buf.push(buf, len);

	//����Ҫ�������ݵ�socket
	FD_SET(p->socket, &write_set_[SELECT_BAK]);
}

void SelectIOService::closePeer(UI32 index, UI64 serial)
{
	peer* p=peer_manager_.getPeer(index);

	//1,�Ѿ��ر���״̬�Ĺ���2���Ѿ��رգ��ұ��������ռ��
	if(p->serial!=serial || p->socket==INVALID_SOCKET)
		return;

	if(onDisConCallback_)
		onDisConCallback_(index, serial, p->remote_ip, p->remote_port);

	std::vector<UI32>::iterator itor=find(online_peer_set_.begin(), online_peer_set_.end(), index);
	if(itor!=online_peer_set_.end())
		online_peer_set_.erase(itor);

	p->socket=INVALID_SOCKET;
	CloseSocket(p->socket);

	//����˳������socket�ӱ��ݶ���ɾ���������´μ���
	FD_CLR(p->socket, &read_set_[SELECT_BAK]);

	//��socket�ӵ�ǰ����ɾ��
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

	//�����������ߵ�socket
	FD_SET(p->socket, &read_set_[SELECT_BAK]);
}


void SelectIOService::eventLoop()
{
	peer* tmp=NULL;

	//��ʼ�����
	initSets();

	//�߳�����5������û��Ϣ���򷵻ء�������nullһֱ������Ϊ�˷�ֹ�����˳���ʱ���io�߳��޷�����
	timeval timeout;
	timeout.tv_sec=5;
	timeout.tv_usec=0;

	//��������socket
	int ready_socket_num = select(	0,					//����
									&read_set_[SELECT_USE],			//ָ������е�ָ��
									&write_set_[SELECT_USE],		//ָ��д���е�ָ��
									NULL,
									&timeout);			//��ʱ

	//�����socket��״̬�����仯
	if(ready_socket_num>0)
	{

		if(listen_socket_ != INVALID_SOCKET)
		{
			//��������
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
					//����ʧ�ܣ��ͻ����Լ�����
					int err_no=GetLastError();
					THROWNEXCEPT("accept error. errno:%d", err_no);
				}
			}

			//����
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

			//���¼�
			if(FD_ISSET(p->socket, &read_set_))
			{
				int ret=p->read_buf.sockRead(p->socket);
				if(ret!=0)
				{
					//�Ͽ�����
					closePeer(index, p->serial);
					continue;
				}

				if(onReadCallback_(index, p->serial, &p->read_buf) < 0)
				{
					//�Ͽ�����
					closePeer(index, p->serial);
					continue;
				}
			}

			//д�¼�
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
	//����������еľ���������
	FD_ZERO(&read_set_[SELECT_USE]);
	FD_ZERO(&write_set_[SELECT_USE]);

	if(listen_socket_ != INVALID_SOCKET)
	{
		//��Ӽ��������read array,��������ӽ���������Ӧ
		FD_SET(listen_socket_, &read_set_);
		//FD_SET(listen_socket_, &exception_set_);
	}

	read_set_[SELECT_USE]=read_set_[SELECT_BAK];
	write_set_[SELECT_USE]=write_set_[SELECT_BAK];
	return 0;
}
NS_YY_END
