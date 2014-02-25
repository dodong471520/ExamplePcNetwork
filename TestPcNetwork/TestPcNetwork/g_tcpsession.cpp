/*
g_tcpsession.cpp: implementation of the CG_TCPSession class.
*/

#include "g_tcpsession.h"
#include <stdio.h>
#include <time.h>

CG_CmdPacket  CG_TCPSession::m_sendCmdPkt;
CG_CmdPacket  CG_TCPSession::m_recvCmdPkt;
CG_NetPacket  CG_TCPSession::m_sendNetPkt;
CG_NetPacket  CG_TCPSession::m_recvNetPkt;

CG_TCPSession::CG_TCPSession()
{
	m_sendCmdPkt.SetSize(MAX_CMD_PACKET_SIZE);
	m_recvCmdPkt.SetSize(MAX_CMD_PACKET_SIZE);
	m_bEncrypt  = false;
	m_bCompress = true;
	m_readBuf = m_writeBuf = NULL;
	Reset();
}

CG_TCPSession::~CG_TCPSession()
{
	delete []m_readBuf;
	delete []m_writeBuf;
}

void CG_TCPSession::OnConnect(bool ret)
{

}

void CG_TCPSession::OnDisconnect()
{

}

void CG_TCPSession::OnReceive(CG_CmdPacket *packet)
{

}

void CG_TCPSession::Process()
{
	DefProc();
	bool ret = false;
	
	switch (m_state)
	{
		case NET_STATE_CONNECTED:
			while(GetPacket(&m_recvCmdPkt))
			{
				OnReceive(&m_recvCmdPkt);
    		}
			break;

		case NET_STATE_DEAD:
			OnDisconnect();
			Close();
			break;

		case NET_STATE_WAITING_SEED:
			/* try get encrypt seed */
			if(RecvPacket(&m_recvNetPkt))
			{
				m_recvNetPkt.AttachCmdPacket(&m_recvCmdPkt);
				if(m_recvNetPkt.IsSysPacket())
				{
					HandleSysPacket(&m_recvCmdPkt);
				}
			}
			break;
			
		case NET_STATE_CONNECTING:
			if(m_socket.HasExcept() || 
				Sys_GetTime() - m_connTime > NET_CONNECT_TIMEOUT)
			{
				Sys_Log("connecting timeout");
				Close();
				OnConnect(false);
				break;
			}

			if(m_socket.CanWrite()) 
			{
				ret = true;
			}
			else
			{
				break;
			}
#ifdef LINUX 
			/* in linux should check again */
			int err,len;
			len = sizeof(err);
			if(getsockopt(m_socket.m_socket,SOL_SOCKET,SO_ERROR,
				(void *)&err,(socklen_t*)&len) != SOCKET_ERROR) 
			{
				if(err != 0) ret = false;
			}
			else
				ret = false;
#endif		
			if(ret)
			{
				m_state = NET_STATE_WAITING_SEED;
				m_startTime = Sys_GetTime();
			    m_lastSendTime = m_startTime;
				m_lastRecvTime = m_startTime;
			}
			else
			{
				OnConnect(false);
				Close();
			}
 			break;
	}
}

bool CG_TCPSession::Connect(char *remote, int port, char *bindIp,int bindPort)
{
	if(!m_socket.Initialize(PROTOCOL_TCP))
		return false;
	if(!m_socket.BindAddr(bindIp,bindPort))
		return false;
		
	if(m_socket.Connect(remote,port))
	{
		m_state = NET_STATE_CONNECTING;
		m_connTime = Sys_GetTime();
		return true;
	}
	return false;
}

void CG_TCPSession::Close()
{
	Reset();
}

void CG_TCPSession::Reset()
{
	m_rtt = 0;

	delete []m_readBuf;
	delete []m_writeBuf;

	m_readBuf = new char[MAX_BUFFER_SIZE];
	m_readBufSize = MAX_BUFFER_SIZE;

	m_writeBuf = new char[MAX_BUFFER_SIZE];
	m_writeBufSize = MAX_BUFFER_SIZE;
	m_maxBufSize   = MAX_BUFFER_SIZE;

	m_socket.Close();
	m_socket.Reset();
	
	m_seedLen = 0;
	m_seq = 0;
	m_sendBytes = 0;
	m_recvBytes = 0;
	m_state = NET_STATE_ZERO;
	m_writeLen = m_writeBeg = m_writeEnd = 0;
	m_readLen = m_readPos = 0;
	SetBufSize(MAX_BUFFER_SIZE/1024+1);
}

void CG_TCPSession::Attach(SOCKET socket)
{
	m_socket.Attach(socket);
	m_state = NET_STATE_CONNECTED;
	m_startTime = Sys_GetTime();
	m_lastRecvTime = m_startTime;
	m_lastSendTime = m_startTime;
}

bool CG_TCPSession::SendPacket(CG_CmdPacket *packet,bool flush,bool sys)
{
	if (m_state != NET_STATE_CONNECTED) 
		return false;
	
	/* compute seq number */
	m_seq++;
	if(m_seq > MAX_NET_PACKET_SEQ) m_seq = 1;

	m_sendNetPkt.AddCmdPacket(packet);
	m_sendNetPkt.SetSeq(m_seq);
	
	if(sys)
	{
		m_sendNetPkt.SetSysPacket();
	}
	else
	{
		/* check if need encrypt and compress */
		if(m_bEncrypt) m_sendNetPkt.Encrypt(m_seed,m_seedLen);
		if(m_bCompress) m_sendNetPkt.Compress();
	}

	/* add to send buffer */
	if(!PushData(m_sendNetPkt.m_buffer,m_sendNetPkt.m_totalSize)) 
		return false;
	
	/* flush data */
	if(flush) FlushData();
	return true;
}

int CG_TCPSession::GetAvgFluxIn()
{
	int time = (Sys_GetTime() - m_startTime)/1000;
	if(time == 0) time = 1;
	return GetBytesIn()/time;
}

int CG_TCPSession::GetAvgFluxOut()
{
	int time = (Sys_GetTime() - m_startTime)/1000;
	if(time == 0) time = 1;
	return (GetBytesOut()/time);
}

int CG_TCPSession::GetBytesIn()
{
	return m_recvBytes;
}

int CG_TCPSession::GetBytesOut()
{
	return m_sendBytes;
}

bool CG_TCPSession::GetPacket(CG_CmdPacket *packet)
{
	while(RecvPacket(&m_recvNetPkt))
	{
		/* attach a cmd packet for read */
		m_recvNetPkt.AttachCmdPacket(packet);
		if(m_recvNetPkt.IsSysPacket())
		{
			/* if a system msg packet,handle it */
			HandleSysPacket(packet);
		    continue;
		}
		else
		{
			/* get a user data packet,return */
			return true;
		}
	}
	return false;
}

bool CG_TCPSession::ReadData()
{
	if (m_readLen == 0)	
	{
		m_readPos = 0;
	}
	else if (m_readLen > 0 && m_readPos != 0)
	{
		memmove(m_readBuf,m_readBuf+m_readPos,m_readLen);
		m_readPos = 0;
	}
	
	/* m_readPos always = 0 */
	int ret;
	ret = m_socket.Recv(m_readBuf+m_readLen,m_readBufSize-m_readLen);
	if (ret > 0)
	{
		m_readLen += ret;
		m_recvBytes += ret;
		/* reset recv time */
		m_lastRecvTime = Sys_GetTime();
		return true;
	}
	else if(ret == 0)
	{
		return false;
	}
	else
	{
		m_state = NET_STATE_DEAD;
	}
	return false;
}

void CG_TCPSession::FlushData()
{
	/* flush data in buffer */
	int ret;
	if (m_writeLen == 0) return;

	/* reset last send time */
	/*
	m_lastSendTime = Sys_GetTime();
	*/
	if (m_writeEnd > m_writeBeg)
	{
		ret = m_socket.Send(m_writeBuf+m_writeBeg,m_writeLen);
		if (ret >= 0)
		{
			m_sendBytes += ret;
			m_writeLen -= ret;
			m_writeBeg = (m_writeBeg+ret)%m_writeBufSize;
		}
		else if (ret == -1)
		{
			m_state = NET_STATE_DEAD;
			return;
		}
	}
	else  /* end < begin */
	{
		ret = m_socket.Send(m_writeBuf+m_writeBeg,m_writeBufSize-m_writeBeg);
		if (ret >= 0)
		{
			m_sendBytes += ret;
			m_writeLen -= ret;
			m_writeBeg = (m_writeBeg+ret)%m_writeBufSize;
			if(m_writeBeg!=0) return;
		}
		else if (ret == -1)
		{
			m_state = NET_STATE_DEAD;
			return;
		}
		
		ret = m_socket.Send(m_writeBuf,m_writeEnd);
		if (ret >= 0)
		{
			m_sendBytes += ret;
			m_writeLen -= ret;
			m_writeBeg = (m_writeBeg+ret)%m_writeBufSize;
		}
		else if (ret == -1)
		{
			m_state = NET_STATE_DEAD;
			return;
		}
	}
}

bool CG_TCPSession::PushData(char *buf, int len)
{
	int tlen = len;
	/* first check if can hold data */
	if(len >= m_writeBufSize - m_writeLen) 
	{
		if (!ResizeBuf(m_writeLen + len +1)) 
		{
			return false;
		}
	}
	m_writeLen+=len;
	while(len--)
	{
		m_writeBuf[m_writeEnd++] = *buf++;
		if (m_writeEnd == m_writeBufSize)
			m_writeEnd = 0;
	}
	return true;
}

void CG_TCPSession::SetBufSize(int kb)
{
	m_maxBufSize = kb*1000;
}

bool CG_TCPSession::ResizeBuf(int min)
{
	int newSize = min*2;
	if (min >= m_maxBufSize) return false;
	if (newSize>m_maxBufSize)
		newSize = m_maxBufSize;
	Sys_Log("resize socket buf to %d",newSize);
	char *buf = new char[newSize];

	/* copy src data */
	if (m_writeLen > 0)
	{
		if(m_writeEnd > m_writeBeg)
			memcpy(buf,m_writeBuf+m_writeBeg,m_writeLen);
		else
		{
			int nLen = m_writeBufSize - m_writeBeg;
			memcpy(buf,m_writeBuf+m_writeBeg,nLen);
			memcpy(buf+nLen,m_writeBuf,m_writeLen - nLen);
		}
	}
	delete []m_writeBuf;
	m_writeBuf = buf;
	m_writeBeg = 0;
	m_writeEnd = m_writeLen;
	m_writeBufSize = newSize;
	return true;
}

int CG_TCPSession::GetState()
{
	return m_state;
}

bool CG_TCPSession::IsDead()
{
	return (m_state == NET_STATE_DEAD);
}

void CG_TCPSession::DefProc()
{
	if(m_state != NET_STATE_CONNECTED && m_state != NET_STATE_WAITING_SEED) return;
	
	if(m_state == NET_STATE_CONNECTED)
	{
		long now = Sys_GetTime();
		if(now - m_lastSendTime > NET_KEEP_ALIVE_INTERVAL)
		{
			//KeepAlive(now);
			m_lastSendTime = now;
		}
		if(now - m_lastRecvTime > NET_TIMEOUT)
		{
			Sys_Log("tcp session dead reason = timeout");
			m_state = NET_STATE_DEAD;
			return;
		}
		FlushData();
	}
	ReadData();
}

bool CG_TCPSession::RecvPacket(CG_NetPacket *packet)
{
	int ret;
	ret = packet->BeginRead(m_readBuf+m_readPos,m_readLen);
	if(ret > 0)
	{
		m_readLen -= ret;
		m_readPos += ret;
		if(packet->IsEncrypt())  
		{
			packet->Decrypt(m_seed,m_seedLen);
		}
		if(packet->IsCompress()) 
		{
			if(!packet->Uncompress()) return false;
		}
		return true;
	}
	else if(ret == 0)
	{
		return false;
	}
	else 
	{
		Sys_Log("error getcmd = -1"); 
		/* the packet flag error,not belong to this session */
		m_state = NET_STATE_DEAD;
		return false;
	}
	return false;
}

int CG_TCPSession::HandleSysPacket(CG_CmdPacket *packet)
{
	short type;
	long param;

	if(!packet->ReadShort(&type)) return NULL;
	switch(type)
	{
		case SYS_PACKET_KEEP_ALIVE:
			if(!packet->ReadLong(&param)) return NULL;
			Sys_Log("recv keep alive msg");
			m_sendCmdPkt.BeginWrite();
			m_sendCmdPkt.WriteShort(SYS_PACKET_ALIVE_ACK);
			m_sendCmdPkt.WriteLong(param);
			SendPacket(&m_sendCmdPkt,true,true);
			break;

		case SYS_PACKET_ALIVE_ACK:
			if(!packet->ReadLong(&param)) return NULL;
			Sys_Log("recv alive ack msg");
			m_rtt = Sys_GetTime() - param;
			break;

		case SYS_PACKET_SET_SEED:
			if(!packet->ReadLong(&param)) return NULL;
			SetSeed(param);
			m_state = NET_STATE_CONNECTED;
			OnConnect(true);
	    	break;

		default :
			return NULL;
	}
	return type;
}

void CG_TCPSession::SetSeed(long seed)
{
	Sys_Log("set seed = %d",seed);
	if(seed == 0)
	{
		m_bEncrypt = false;
		return;
	}

	int i;
	for(i=0;i<MAX_NET_SEED_SIZE;i++)
	{
		m_seed[i] = (seed % 255)+1;
		seed /= 19;
	}
	m_seedLen = i;
	m_bEncrypt = true;
}

void CG_TCPSession::SendSeed(long seed)
{
	m_sendCmdPkt.BeginWrite();
	m_sendCmdPkt.WriteShort(SYS_PACKET_SET_SEED);
	m_sendCmdPkt.WriteLong(seed);
	SendPacket(&m_sendCmdPkt,true,true);
	SetSeed(seed);
}

void CG_TCPSession::KeepAlive(long now)
{
	m_sendCmdPkt.BeginWrite();
	m_sendCmdPkt.WriteShort(SYS_PACKET_KEEP_ALIVE);
	m_sendCmdPkt.WriteLong(now);
	SendPacket(&m_sendCmdPkt,true,true);
}

int CG_TCPSession::GetRtt()
{
	return m_rtt;
}
