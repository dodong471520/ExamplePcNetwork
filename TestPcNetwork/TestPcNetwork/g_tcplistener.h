/*
g_tcplistener.h: interface for the CG_TCPListener class.
*/

#ifndef __CG_TCP_LISTENER_H__
#define __CG_TCP_LISTENER_H__

#include "g_tcpsession.h"

class CG_TCPListener  
{
public:
	long ComputeSeed();
	void Close();
	bool StartListen(char *ip,int port,bool encrypt=true);
	virtual CG_TCPSession *AcceptNewSession();

	CG_TCPListener();
	virtual ~CG_TCPListener();

protected:
	CG_NetSocket m_socket;
	bool m_bEncrypt;
};

#endif 