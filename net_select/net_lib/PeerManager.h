#ifndef _PEER_MANAGER_HPP_
#define _PEER_MANAGER_HPP_

#include "SockBuf.h"
#include <memory.h>


#if (YY_TARGET_PLATFORM==YY_PLATFORM_WIN32)
	#include <windows.h>
#endif

#if (YY_TARGET_PLATFORM==YY_PLATFORM_LINUX)
	#define INVALID_SOCKET -1
#endif

#define MAX_PEER_NUM 1024*10	//���������

NS_YY_BEGIN

struct peer
{
	int socket;
	char local_ip[50];
	UI32 local_port;
	char remote_ip[50];
	UI32 remote_port;

	UI64 serial;				//���кţ�ÿ�������������ɵ����кţ�session key��Ч��
	bool wsa_send;				//��֤һ��socketֻ��һ��sendͶ��, recvֻ��io thread���������Կ��Ա�ֻ֤��һ��recvͶ��

	SockBuf read_buf;
	SockBuf write_buf;			//wsa buf, Ͷ��֮�����ϵͳ�������ڴ棬����Ͷ��֮��Ͳ����޸Ļ�������
	SockBuf write_buf_backup_;	//����Ѿ�Ͷ��д������û��汣��Ҫ���͵�����

	peer()
	{
		wsa_send=false;
		socket=INVALID_SOCKET;
		memset(local_ip, 0, sizeof(local_ip));
		local_port=0;
		memset(remote_ip, 0, sizeof(remote_ip));
		remote_port=0;
		serial=0;
	}
};

class PeerManager
{
public:
	PeerManager();
	~PeerManager();

	UI32 getFreeIndex();
	peer* getPeer(UI32 index);

private:
	peer* peer_array_;
};

NS_YY_END
#endif