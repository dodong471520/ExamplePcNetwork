#ifndef __CG_NET_PACKET_H__
#define __CG_NET_PACKET_H__

#include "g_cmdpacket.h"
const int NET_PACKET_HEAD_SIZE = 5;
const int MAX_NET_PACKET_SIZE  = MAX_CMD_PACKET_SIZE + NET_PACKET_HEAD_SIZE; 
/*
net packet head struct
{
	4  bit	0101 
	1  bit	reserved
	1  bit  flag system 
	1  bit	flag compress 
	1  bit	flag encrypt
	16 bit  seq
	16 bit  data size
}
*/
const char PROTOCOL_FLAG = 5; // 0101
const char FLAG_ENCRYPT  = 1; // 0001
const char FLAG_COMPRESS = 2; // 0010
const char FLAG_SYSTEM   = 4; // 0100

const short SYS_PACKET_SET_SEED   = 1;
const short SYS_PACKET_KEEP_ALIVE = 2;
const short SYS_PACKET_ALIVE_ACK  = 3;

const unsigned short MAX_NET_PACKET_SEQ = 65000;

class CG_NetPacket : public CG_CmdPacket  
{
public:
	CG_NetPacket();
	virtual ~CG_NetPacket();

	void AddCmdPacket(CG_CmdPacket *packet);
	void AttachCmdPacket(CG_CmdPacket *packet);
	bool Uncompress();
	bool Compress();
	bool IsCompress();
	bool IsEncrypt();
	bool IsSysPacket();
	void SetSysPacket();
	void Decrypt(char *seed,int len);
	void Encrypt(char *seed,int len);
	void SetSeq(unsigned short seq);

	int  GetHeadSize();
	int  GetTotalSize();
	int  BeginRead(char *p,int len);

	char			 m_buffer[MAX_NET_PACKET_SIZE];
	int				 m_totalSize;
	char			*m_pktFlag;
	unsigned short  *m_pktSeq;
	unsigned short  *m_pktDataSize;
};
#endif