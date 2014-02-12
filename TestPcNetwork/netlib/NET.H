#if !defined(AFX_NET_H__BAB99BDE_FD81_46A4_B0A7_FAC6CE9FD2BD__INCLUDED_)
#define AFX_NET_H__BAB99BDE_FD81_46A4_B0A7_FAC6CE9FD2BD__INCLUDED_

#include "net_lib.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




#define PACKET_CONTENT_POS sizeof(NET_PACKET_HEADER) + 4
struct _PACKET_DATA 
{
	uint16	data_len;
	void*	ptr_data;
};
//网络数据包基类
//packet结构：
//        数据头(不加密) -- 较认码 -- 命令 -- 数据1 -- 数据2 -- ……
class NET_LIB_API NET_Packet  
{
public:
	void reset();
	NET_Packet(int data_size = MAX_PACKET_SIZE);
	virtual ~NET_Packet();

	NET_Packet* get_cling_packet();		//解决粘包问题
	bool		is_cling();
	NET_PACKET_HEADER*	get_NET_PACKET_HEADER()	{return (NET_PACKET_HEADER *) m_data;}
	//in packet operation
	uint8 read8();
	uint16 read16();
	uint32 read32();
	const char *readString();	
	void* readData();
	_PACKET_DATA	readBinary();

	uint16 getVersion() { return ntohs(((NET_PACKET_HEADER *) m_data)->ver); }
	uint16 getIC()	{return ntohs(((NET_PACKET_HEADER *) m_data)->ic);}
	uint32 getSID() { return ntohl(((NET_PACKET_HEADER *) m_data)->sid); }
	uint32 getUID() { return ntohl(((NET_PACKET_HEADER *) m_data)->uid); }
	uint32 getLen() { return ntohs(((NET_PACKET_HEADER *) m_data)->len); }

	//out packet operation
	void write8(uint8 b);
	void write16(uint16 w);
	void write32(uint32 dw);
	void writeString(const char *str);
	void writeData(void* pData, uint16 len);

	void setVersion(uint16 ver) { ((NET_PACKET_HEADER *) m_data)->ver = htons(ver); }
	void setIC(uint16 ic) { ((NET_PACKET_HEADER *) m_data)->ic = htons(ic); }
	void setSID(uint32 sid) { ((NET_PACKET_HEADER *) m_data)->sid = htonl(sid); }
	void setUID(uint32 uid) { ((NET_PACKET_HEADER *) m_data)->uid = htonl(uid); }

	uint16 getCmd() { return ntohs(*((uint16*)(m_data + sizeof(NET_PACKET_HEADER) + 2))); }
	void setCmd(uint16 cmd) { *((uint16*)(m_data + sizeof(NET_PACKET_HEADER) + 2)) = htons(cmd); }

	uint16 getChecker()	{ return ntohs(*((uint16*)(m_data + sizeof(NET_PACKET_HEADER)))); }
	void setChecker(uint16 ck)	{ *((uint16*)(m_data + sizeof(NET_PACKET_HEADER))) = htons(ck); }
	bool check()	{return ntohs(*((uint16*)(m_data + sizeof(NET_PACKET_HEADER))))==m_datalen;}

	void begin();
	void end();

	void skip_all()
	{
		m_cursor = m_data + m_datalen;
	}

	char *skip(int delta) 
	{
		m_cursor += delta;
		return (m_cursor - delta);
	}
	char *setCursor(char *cur) 
	{
		char *old = m_cursor;
		m_cursor = cur;
		return old;
	}
	char* getCursor()	{return m_cursor;}
	char* getData()		{return m_data;}
	char* getContent()	{return m_data+PACKET_CONTENT_POS;}

	void setSessionUID(uint32 uid);

	//	virtual int send_to(uint32 uin, NET_Socket *ptr_socket)=0;
	//	virtual int send_to(NET_Session* ptr_session)=0;
	//	virtual int recv_from(uint32 uin, NET_Socket *ptr_socket)=0;
	//	virtual int recv_from(NET_Session* ptr_session)=0;
	//	
	//	virtual int send(NET_Socket* ptr_socket)=0;
	//	virtual int recv(NET_Socket* ptr_socket)=0;
	void setSendto(uint32 uid);
	int send_to(uint32 uin, UDP_Socket *ptr_socket);
	int send_to(UDP_Session* ptr_session);

	int send(UDP_Socket* ptr_socket);
	int recv(UDP_Socket* ptr_socket);

	//TCP
	int send(SOCKET socket);
	int recv(SOCKET socket);
	int send_to(TCP_Session* pSession);
	int recv_from(TCP_Session* pSession);

	uint32 getIP() 
	{
		return ntohl(m_addr_in.sin_addr.s_addr);
	}
	uint16 getPort() 
	{
		return ntohs(m_addr_in.sin_port);
	}
	sockaddr_in* get_addr()
	{
		return &m_addr_in;
	}

private:
	union
	{
		//for in packet
		sockaddr_in m_addr_in;
		//for out packet
		uint32		m_uid_sendto;
	};

protected:
	char* m_data;
	char *m_cursor;

	int			m_datalen;

	int			m_max_data_size;

};

//网络接口基类
class NET_LIB_API NET_Socket  
{
public:
	bool is_active();
	//添加发送数据包，因此要发送者分配数据部对象，发送后将由线程自动删除
	virtual bool		PushSendPacket(uint32 uid_sendto, NET_Packet* ptr_packet)=0;
	//	virtual NET_Packet*	PopSendPacket()=0;
	virtual bool		PushRecvPacket(NET_Packet* ptr_packet, time_t time);
	//获取接收数据包，数据包由线程分配空间，因此使用完后要删除
	virtual NET_Packet*	PopRecvPacket();


	virtual void Ping(uint32 uid=0)=0;
	virtual void CheckConnect(time_t time)=0;
	virtual uint16 getPort()=0;
	virtual uint32 getIP()=0;
	virtual void CloseSession(uint32 uid)=0;
	virtual int GetSessionCount()=0;

	virtual NET_Session* get_NET_Session(uint32 uid)=0;

	virtual void Shutdown()=0;
	//id在100以内为内部使用
	virtual bool Create(uint16 port, uint32 uid_limit_min=100, uint32 uid_limit_max=100000)=0;

	SOCKET		get_SOCKET()	{return m_socket;}
	Thread_Pool*	get_Thread_Pool()			{return &m_thread_pool;}


	int			get_recv_packet_count();

	NET_Socket();
	virtual ~NET_Socket();

	bool LockRecvList();
	void UnlockRecvList();

	void CleanupRecvList();

protected:
	SOCKET			m_socket;
	Thread_Pool		m_thread_pool;
	uint32			m_uid_lmt_min;
	uint32			m_uid_lmt_max;
	std::list<NET_Packet*>			m_lst_recv_packet;
	TYPE_CS							m_lock_recv_packet;

};

//网络会话基类
class NET_LIB_API NET_Session  
{
	friend NET_Socket;
public:
	void update_ic_image(uint16 last_ic);
	void clear_ic_image();
	void resend(uint16 ic);
	void Query_IC();
	bool check_index(NET_Packet* ptr_packet);
	void mark_index(NET_Packet* ptr_packet);
	void Disconnect();
	virtual void Ping();
	virtual void Check_connect(time_t tm);
	virtual void Connect();
	virtual bool PushSendPacket(NET_Packet* ptr_packet)=0;
	virtual bool RunTransfersThread()=0;
	virtual int	send(NET_Packet* pPacket, bool bIC=false)=0;
	virtual int	recv(NET_Packet* pPacket)=0;
	virtual SOCKET get_SOCKET()=0;

	bool	is_ready()		{return m_uid>0 && m_uid_opposite>0;}
	time_t time_get();
	bool test_time_out(time_t time, time_t time_out=CONNECT_CHECK_TIME);
	void time_set(time_t time);
	void set_connect_check_time(time_t time=CONNECT_CHECK_TIME)	{m_connect_check_time	= time;}
	time_t get_connect_check_time()	{return m_connect_check_time;}

	void setUID_opposite(uint32 uid)	{m_uid_opposite	= uid;}

	uint32			getUID()					{return m_uid;}
	uint32			getUID_opposite()			{return m_uid_opposite;}
	NET_Socket*		get_NET_Socket()			{return m_ptr_net_socket;}
	sockaddr_in*	get_sockaddr_in()			{return &m_addr_in;}
	Thread_Pool*	get_Thread_Pool()			{return &m_thread_pool;}

	uint32 get_net_delay()			{return m_net_delay;}

	uint32 getIP() {return ntohl(m_addr_in.sin_addr.s_addr);}
	uint16 getPort() {return ntohs(m_addr_in.sin_port);	}


	NET_Session(uint32 uid);
	virtual ~NET_Session();

protected:
	uint32			m_uid;
	uint32			m_uid_opposite;
	NET_Socket*		m_ptr_net_socket;
	sockaddr_in		m_addr_in;
	Thread_Pool		m_thread_pool;
	time_t			m_time_set;
	uint32			m_net_delay;

	time_t			m_connect_check_time;

	uint16			m_index_send_check;									//当前发送校验序号
	uint16			m_index_recv_check;									//当前接收校验序号
	time_t			m_ric_seq;											//上次请求重发的时间
	typedef std::map<uint16, NET_Packet*>	MAP_PACKET_IMAGE;
	MAP_PACKET_IMAGE	m_map_packet_image;								//发送镜像，在序号校验出错时进行重发
	TYPE_CS			m_lock_ic_image;
};







#endif // !defined(AFX_NET_H__BAB99BDE_FD81_46A4_B0A7_FAC6CE9FD2BD__INCLUDED_)
