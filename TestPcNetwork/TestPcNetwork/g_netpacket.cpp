/*
g_NetPacket.cpp: implementation of the CG_NetPacket class.
*/

#include "g_netpacket.h"

CG_NetPacket::CG_NetPacket()
{
	m_pktFlag = &m_buffer[0];
	m_pktSeq  = (unsigned short*)&m_buffer[1];
	m_pktDataSize = (unsigned short*)&m_buffer[3];	
}

CG_NetPacket::~CG_NetPacket()
{

}

void CG_NetPacket::AddCmdPacket(CG_CmdPacket *packet)
{
	unsigned short dataSize = packet->GetDataSize();
	*m_pktFlag = PROTOCOL_FLAG<<4;
	*m_pktDataSize = dataSize;
	memcpy(m_buffer+NET_PACKET_HEAD_SIZE,packet->GetData(),dataSize);
	m_totalSize = NET_PACKET_HEAD_SIZE + dataSize;
}

void CG_NetPacket::AttachCmdPacket(CG_CmdPacket *packet)
{
	packet->BeginRead(m_buffer+NET_PACKET_HEAD_SIZE,*m_pktDataSize);
}

// -1	Error
//  0	Process Next Time
// >0	Len
int CG_NetPacket::BeginRead(char *p,int len)
{
	if (len < NET_PACKET_HEAD_SIZE) return 0;
	m_pktFlag = &p[0];
	m_pktSeq  = (unsigned short*)&p[1];
	m_pktDataSize = (unsigned short*)&p[3];	

	if (*m_pktFlag>>4 != PROTOCOL_FLAG)
	{
		Sys_Log("NetPacket Flag Error");
		return -1;
	}

	if (*m_pktDataSize <= 0 || *m_pktDataSize > MAX_CMD_PACKET_SIZE) 
	{
		Sys_Log("NetPacket Size Error : s = %d",*m_pktDataSize);
		return -1;
	}

	int total = *m_pktDataSize + NET_PACKET_HEAD_SIZE;

	if (len < total) 
	{
		// Sys_Log("NetPacket Error 2: len = %d,s = %d",len,s);
		return 0;
	}
	/* copy to self buffer */
	memcpy(m_buffer,p,total);
	m_pktFlag = &m_buffer[0];
	m_pktSeq  = (unsigned short*)&m_buffer[1];
	m_pktDataSize = (unsigned short*)&m_buffer[3];	
	return total;
}

void CG_NetPacket::Encrypt(char *seed,int len)
{
	/* encrypt here */
	int i,k;
	for(i=NET_PACKET_HEAD_SIZE,k=0; i<*m_pktDataSize;i++,k++)
	{
		if(k==len) k=0;
		m_buffer[i]^=seed[k];
	}
	*m_pktFlag |= FLAG_ENCRYPT;
}

void CG_NetPacket::Decrypt(char *seed,int len)
{
	/* decrypt here */
	int i,k;
	for(i=NET_PACKET_HEAD_SIZE,k=0; i<*m_pktDataSize;i++,k++)
	{
		if(k==len) k=0;
		m_buffer[i]^=seed[k];
	}
	*m_pktFlag &= (~FLAG_ENCRYPT);
}

bool CG_NetPacket::IsEncrypt()
{
	return (*m_pktFlag & FLAG_ENCRYPT);
}

int CG_NetPacket::GetHeadSize()
{
	return NET_PACKET_HEAD_SIZE;
}

int CG_NetPacket::GetTotalSize()
{
	return m_totalSize;
}

bool CG_NetPacket::Compress()
{
	/* compress here */
	*m_pktFlag |= FLAG_COMPRESS;
	return true;
}

bool CG_NetPacket::Uncompress()
{
	/* uncompress here */
	*m_pktFlag &= (~FLAG_COMPRESS);
	return true;
}

bool CG_NetPacket::IsCompress()
{
	if(*m_pktFlag & FLAG_COMPRESS) return true;
	return false;
}

void CG_NetPacket::SetSysPacket()
{
	*m_pktFlag |= FLAG_SYSTEM;
}

bool CG_NetPacket::IsSysPacket()
{
	if(*m_pktFlag & FLAG_SYSTEM) return true;
	return false;
}

void CG_NetPacket::SetSeq(unsigned short seq)
{
	*m_pktSeq = seq;
}
