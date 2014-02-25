#ifndef __CG_TCP_SESSION_H__
#define __CG_TCP_SESSION_H__

#include "g_netsocket.h"
#include "g_netpacket.h"
const int MAX_BUFFER_SIZE = MAX_NET_PACKET_SIZE;
const int MAX_NET_SEED_SIZE  = 16;
enum TNetState {
	NET_STATE_CONNECTING,
	NET_STATE_WAITING_SEED,
	NET_STATE_CONNECTED,
	NET_STATE_DEAD,
	NET_STATE_ZERO,
};
const int NET_CONNECT_TIMEOUT		= 30*1000;
const int NET_KEEP_ALIVE_INTERVAL	= 10*1000;
#ifdef _DEBUG
const int NET_TIMEOUT			= 900*1000;
#else
const int NET_TIMEOUT			= 90*1000;
#endif

class CG_TCPSession  
{
public:
	CG_TCPSession();
	virtual ~CG_TCPSession();

public:
	void Attach(SOCKET socket);
	bool GetPacket(CG_CmdPacket *packet);
	bool IsDead();
	bool Connect(char *remote,int port,char *bindIp=0,int bindPort=0);
	bool SendPacket(CG_CmdPacket *packet,bool flush=true,bool sys=false);

	int  GetRtt();
	int  GetState();
	int  GetBytesOut();
	int  GetBytesIn();
	int  GetAvgFluxOut();
	int  GetAvgFluxIn();

	void DefProc();
	void Reset();
	void Close();
	void Process();
	void SetBufSize(int kb);
	void SendSeed(long seed);

	virtual void OnReceive(CG_CmdPacket *packet);
	virtual void OnDisconnect();
	virtual void OnConnect(bool ret);
protected:
	int  HandleSysPacket(CG_CmdPacket *packet);
	bool RecvPacket(CG_NetPacket *packet);
	bool ReadData();
	bool PushData(char *buf,int len);
	bool ResizeBuf(int min);
	void KeepAlive(long now);
	void SetSeed(long seed);
	void FlushData();

	CG_NetSocket  m_socket;
	/* share packet */
	static CG_CmdPacket  m_sendCmdPkt;	
	static CG_CmdPacket  m_recvCmdPkt;
	static CG_NetPacket  m_sendNetPkt;	
	static CG_NetPacket  m_recvNetPkt;

	TNetState	  m_state;
	unsigned long m_startTime;
	unsigned long m_lastRecvTime;
	unsigned long m_lastSendTime;
	unsigned long m_connTime;

	char *m_readBuf;
	int	  m_readBufSize;
	int   m_readPos;
	int   m_readLen;

	char *m_writeBuf;
	int   m_writeBufSize;
	int	  m_writeLen;
	int   m_writeBeg;
	int	  m_writeEnd;

	char m_seed[MAX_NET_SEED_SIZE];
	int  m_seedLen;
	int  m_maxBufSize;

	bool m_bEncrypt;
	bool m_bCompress;
	unsigned short m_seq;

	int   m_sendBytes;
	int   m_recvBytes;
	int   m_rtt;
};

#endif
