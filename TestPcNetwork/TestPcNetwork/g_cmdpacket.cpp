/*
g_CmdPacket.cpp: implementation of the CG_CmdPacket class.
*/

#include "g_cmdpacket.h"

CG_CmdPacket::CG_CmdPacket()
{
	m_nMaxSize = 0;
	m_nLen = 0;
	m_nReadOffset  = 0;
	m_nWriteOffset = 0;
	m_pData = NULL;
	/* set buffer to default size */
	SetSize(DEFAULT_CMD_PACKET_SIZE);
}

CG_CmdPacket::~CG_CmdPacket()
{
	delete []m_pData;
}

bool CG_CmdPacket::SetSize(int len)
{
	if(len > MAX_CMD_PACKET_SIZE) return false;
	delete []m_pData;
	m_pData = NULL;
	m_pData = new char[len];
	m_nMaxSize = len;
	return m_pData?true:false;
}

/* try write data to packet */
bool CG_CmdPacket::WriteData(void *data, int len)
{
	if ((m_nLen + len) > m_nMaxSize) return false;
	memcpy(m_pData+m_nWriteOffset,data,len);
	m_nLen += len;
	m_nWriteOffset += len;
	return true;
}

bool CG_CmdPacket::WriteByte(char c)
{
	return WriteData(&c,BYTE_SIZE);
}

void CG_CmdPacket::BeginRead(char *p,int len)
{
	m_pReadData = p;
	m_nLen = len;
	m_nReadOffset = 0;
}

void CG_CmdPacket::BeginRead()
{
	m_pReadData = m_pData;
	m_nReadOffset = 0;
}

void CG_CmdPacket::BeginWrite()
{
	m_nLen = 0;
	m_nWriteOffset = 0;
}

bool CG_CmdPacket::WriteBinary(char *data, int len)
{
	if (WriteShort(len)==false) 
		return false;
	return WriteData(data,len);
}

bool CG_CmdPacket::ReadBinary(char **data, int *len)
{
	short dataLen;
	if (!ReadShort(&dataLen)) return false;
	if (dataLen <= 0) return false;
	if ((m_nReadOffset + dataLen) > m_nLen) return false;
	*data = m_pReadData + m_nReadOffset;
	*len = dataLen;
	m_nReadOffset += dataLen;
	return true;
}

bool CG_CmdPacket::WriteShort(short s)
{
	return WriteData(&s,SHORT_SIZE);
}

bool CG_CmdPacket::WriteLong(long l)
{
	return WriteData(&l,LONG_SIZE);
}

bool CG_CmdPacket::WriteFloat(float f)
{
	return WriteData(&f,FLOAT_SIZE);
}

bool CG_CmdPacket::ReadByte(char *c)
{
	return ReadData(c,BYTE_SIZE);
}

bool CG_CmdPacket::ReadShort(short *s)
{
	return ReadData(s,SHORT_SIZE);
}

bool CG_CmdPacket::ReadLong(long *l)
{
	return ReadData(l,LONG_SIZE);
}

bool CG_CmdPacket::ReadFloat(float *f)
{
	return ReadData(f,FLOAT_SIZE);
}

bool CG_CmdPacket::ReadString(char **str)
{
	short len;
	if (!ReadShort(&len)) return false;
	if (len <= 0) return false;
	if ((m_nReadOffset + len) > m_nLen) return false;
	/* set end avoid overflow */
	*(m_pReadData+m_nReadOffset+len-1) = '\0';
	*str = m_pReadData + m_nReadOffset;
	m_nReadOffset += len;
	return true;
}

bool CG_CmdPacket::WriteString(char *str)
{
	short len = strlen(str) + 1;
	if (!WriteShort(len)) return false;
	return WriteData(str,len);
}

bool CG_CmdPacket::ReadData(void *data, int len)
{
	if ((m_nReadOffset + len)>m_nLen) return false;

	memcpy(data,m_pReadData+m_nReadOffset,len);
	m_nReadOffset += len;
	return true;
}

bool CG_CmdPacket::CopyData(char *buf,int len)
{
	if(!SetSize(len)) return false;
	memcpy(m_pData,buf,len);
	m_nLen = len;
	return true;
}

bool CG_CmdPacket::CloneFrom(CG_CmdPacket *packet)
{
	return CopyData(packet->GetData(),packet->GetDataSize());
}

char *CG_CmdPacket::GetData()
{
	return m_pData;
}

int CG_CmdPacket::GetDataSize()
{
	return m_nLen;
}

int CG_CmdPacket::GetMaxSize()
{
	return m_nMaxSize;
}

