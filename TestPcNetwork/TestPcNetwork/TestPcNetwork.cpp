// TestPcNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "g_netsocket.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CG_NetSocket listen;
	if(!listen.Initialize(PROTOCOL_TCP))
		return 1;
	listen.SetNonBlocking();
	if(!listen.BindAddr(NULL,7788))
		return 1;
	if(!listen.Listen())
		return 1;
	CG_NetSocket client,*server;
	if(!client.Initialize(PROTOCOL_TCP))
		return 1;
	client.Connect("localhost",7788);
	if(!listen.CanRead())
		return 1;
	SOCKET tmp=listen.Accept();
	if(!tmp)
	{
		Sys_Log("accept failed");
		return 1;
	}
	server=new CG_NetSocket;
	server->Attach(tmp);
	Sys_Log("accept ok");
	char addr[20];
	short port;
	server->GetLocalAddr(addr,&port);
	Sys_Log("local ip=%s,port=%d",addr,port);
	server->GetRemoteAddr(addr,&port);
	Sys_Log("remote ip=%s,port=%d",addr,port);

	char msg[128];
	strcpy(msg,"hello world");
	client.Send(msg,strlen(msg));
	
	char buf[128];
	int ret=server->Recv(buf,128);
	if(ret>0)
	{
		buf[ret]='\0';
		Sys_Log("recv bytes = %d,msg = %s",ret,buf);
	}
	getchar();
	return 0;
}

