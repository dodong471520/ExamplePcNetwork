// TestPcNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "g_tcplistener.h"
#include <list>
class CD_ServerSession:public CG_TCPSession
{
public:
	CD_ServerSession(){}
	virtual ~CD_ServerSession(){}
public:
	virtual void OnReceive(CG_CmdPacket *packet)
	{
		char buff[256];
		short port;
		m_socket.GetRemoteAddr(buff,&port);
		char* str=NULL;
		short cmd;
		packet->ReadShort(&cmd);
		if(packet->ReadString(&str))
		{
			Sys_Log("server_client recv:%s:%d:%s",buff,port,str);
		}
	}
	virtual void OnDisconnect()
	{
		char buff[256];
		short port;
		m_socket.GetRemoteAddr(buff,&port);
		Sys_Log("server_client disconnect:%s:%d",buff,port);
	}
	virtual void OnConnect(bool ret)
	{
		if(ret)
		{
			char buff[256];
			short port;
			m_socket.GetRemoteAddr(buff,&port);
			Sys_Log("server_client connect:%s:%d",buff,port);
		}
		else
			Sys_Log("server_client connect timeout");
	}
};
class CD_Server:public CG_TCPListener
{
public:
	CD_Server(){}
	virtual ~CD_Server(){}
	std::list<CG_TCPSession*> m_sessions;
	virtual CG_TCPSession *AcceptNewSession()
	{
		if(!m_socket.CanRead())return false;
		SOCKET socket=m_socket.Accept();
		if(!socket)return false;
		CD_ServerSession *tmp=new CD_ServerSession;
		tmp->Attach(socket);
		tmp->SendSeed(ComputeSeed());
		return tmp;
	}
	void Process()
	{
		CG_TCPSession * session=AcceptNewSession();
		if(session)
			m_sessions.push_back(session);
		std::list<CG_TCPSession*>::iterator it;
		for(it=m_sessions.begin();it!=m_sessions.end();++it)
		{
			(*it)->Process();	
		}
	}
};
int _tmain(int argc, _TCHAR* argv[])
{
	CD_Server server;
	server.StartListen("127.0.0.1",3335);
	while(true)
	{
		server.Process();
		Sleep(50);
	}
	getchar();
	return 0;
}

