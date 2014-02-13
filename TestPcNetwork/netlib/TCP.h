// TCP.h: interface for the TCP_Session class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCP_H__BAB99BDE_FD81_46A4_B0A7_FAC6CE9FD2BD__INCLUDED_)
#define AFX_TCP_H__BAB99BDE_FD81_46A4_B0A7_FAC6CE9FD2BD__INCLUDED_

#include "net_lib.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<typename T>
class NET_LIB_API TCP_Session  : public NET_Session
{
public:
	virtual bool Connect();
	//	void Connect();
	SOCKET			get_SOCKET()				{return m_socket;}


	bool			PushSendPacket(NET_Packet* ptr_packet);
	bool			PushRecvPacket(NET_Packet* ptr_packet, time_t time);
	NET_Packet*		PopSendPacket();

	int				recv(NET_Packet* pPacket);
	int				send(NET_Packet* pPacket, bool bIC=false);
	TCP_Session(TCP_Socket<T> *tcpsocket);
	virtual ~TCP_Session();

protected:
	SOCKET		m_socket;
	TCP_Socket<T>	*m_ptr_tcp_socket;
	std::list<NET_Packet*>			m_lst_send_packet;				//发送列表
	TYPE_CS							m_lock_send_packet;				//发送列表锁
};
template<typename T>
class NET_LIB_API TCP_Socket : public NET_Socket  
{
public:
	TCP_Socket();
	virtual ~TCP_Socket();
	bool CreateServer(uint16 port);
	bool CreateClient(char const* szAddr,uint16 port);
public:
	void CreateSession();
	void Ping(uint32 uid=0);
	bool change_session_uid(uint32 uid, uint32 uid_to);
	void CheckConnect(time_t time);
	uint16 getPort();
	uint32 getIP();
	bool CanRead();	
	bool CanWrite();
	void CloseSession(TCP_Session *session);
	int GetSessionCount();
	void ThreadInit();
	void ThreadEnd();
	void ProcessServer();
	//添加发送数据包，因此要发送者分配数据部对象，发送后将由线程自动删除
	virtual bool		PushSendPacket(uint32 uid_sendto, NET_Packet* ptr_packet);
	
	virtual NET_Packet*	PopRecvPacket();

	template<typename T>
	TCP_Session* CreateSession(sockaddr_in* ptr_addr);
	template<typename T>
	TCP_Session* CreateUniqueSession(uint32 ip, uint16 port, uint32 uid_unique, const char* szAddr=NULL);
	template<typename T>
	TCP_Session* CreateSession(uint32 ip, uint16 port, const char* szAddr=NULL);
	template<typename T>
	TCP_Session* get_TCP_Session(uint32 uid);

	NET_Session* get_NET_Session(uint32 uid);


	virtual void Shutdown();
	//id在100以内为内部使用
	bool Create(uint16 port);

	SOCKET		get_SOCKET()	{return m_socket;}

	//	void Unlock();
	//	bool Lock(uint32 uid);

	bool LockSessionMap();
	void UnlockSessionMap();

	void CleanupAllSession();
	enum{
		SELECT_BAK = 0,	//当前系统中拥有的完整句柄数据
		SELECT_USE = 1,	//用于select调用的句柄数据
		SELECT_MAX = 2, //结构使用数量
	};
	fd_set		m_ReadFDs[SELECT_MAX];
	fd_set		m_WriteFDs[SELECT_MAX];
	fd_set		m_ExceptFDs[SELECT_MAX];
	timeval		m_Timeout[SELECT_MAX];
protected:
	std::map<SOCKET,T*> m_map_session;
	unsigned long			m_lastTime;
};




#endif // !defined(AFX_TCP_H__BAB99BDE_FD81_46A4_B0A7_FAC6CE9FD2BD__INCLUDED_)
