// TestPcNetWorkClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../TestPcNetwork/g_tcpsession.h"
#include "../TestPcNetwork/g_tcplistener.h"
class Client:public CG_TCPSession
{
public:
	Client(){}
	virtual~Client(){}
	virtual void OnReceive(CG_CmdPacket *packet)
	{
		char buff[256];
		short port;
		m_socket.GetRemoteAddr(buff,&port);
		char* str=NULL;
		if(packet->ReadString(&str))
		{
			Sys_Log("client recv:%s:%d:%s",buff,port,str);
		}
	}
	virtual void OnDisconnect()
	{
		char buff[256];
		short port;
		m_socket.GetRemoteAddr(buff,&port);
		Sys_Log("client disconnect:%s:%d",buff,port);
	}
	virtual void OnConnect(bool ret)
	{
		if(ret)
		{
			char buff[256];
			short port;
			m_socket.GetRemoteAddr(buff,&port);
			Sys_Log("client connect:%s:%d",buff,port);
		}
		else
			Sys_Log("client connect timeout");
	}
};
#include <iostream>
using namespace std;
#include <windows.h>
#include <process.h>
#include <stdio.h>

#define BUFFER_MAX 1024

char g_nbstdin_buffer[2][BUFFER_MAX];
HANDLE g_input[2];
HANDLE g_process[2];

DWORD WINAPI console_input(LPVOID lpParameter)
{
	for (;;) {
		int i;
		for (i=0;i<2;i++) {
			fgets(g_nbstdin_buffer[i],BUFFER_MAX,stdin);
			SetEvent(g_input[i]);
			WaitForSingleObject(g_process[i],INFINITE);
		}
	}
	return 0;
}

void create_nbstdin()
{
	int i;
	DWORD tid;
	CreateThread(NULL,1024,&console_input,0,0,&tid);
	for (i=0;i<2;i++) {
		g_input[i]=CreateEvent(NULL,FALSE,FALSE,NULL);
		g_process[i]=CreateEvent(NULL,FALSE,FALSE,NULL);
		g_nbstdin_buffer[i][0]='\0';
	}
}

const char* nbstdin()
{
	DWORD n=WaitForMultipleObjects(2,g_input,FALSE,0);
	if (n==WAIT_OBJECT_0 || n==WAIT_OBJECT_0+1) {
		n=n-WAIT_OBJECT_0;
		SetEvent(g_process[n]);
		return g_nbstdin_buffer[n];
	}
	else {
		return 0;
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	Client client;
	while(!client.Connect("127.0.0.1",3335))
	{

	}
	create_nbstdin();
	while(true)
	{
		client.Process();
		const char *line=nbstdin();
		if(line)
		{
			CG_CmdPacket packet;
			packet.BeginWrite();
			packet.WriteShort(100);
			packet.WriteString(const_cast<char*>(line));
			client.SendPacket(&packet);
			Sleep(50);
		}
	}
	return 0;
}

