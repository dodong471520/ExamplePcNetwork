#include <stdio.h>
#include <string>
#include "../net_lib/Select_IOService.h"

USING_NS_YY
#pragma comment(lib, "ws2_32.lib")

struct MsgHeader
{
	int len;
};

#define MSGHEADERLEN sizeof(MsgHeader)

class NetEvent
{
private:
	typedef FastDelegate4<UI32, UI64, const char*, UI32> OnSendCallback;
public:
	NetEvent(){}

	void registerOnSend(OnSendCallback onSend)
	{
		onSend_=onSend;
	}

	int onCon(UI32 index, UI64 serial, const char* ip, UI32 port)
	{
		printf("new connection, index:%d, serial:%llu, ip:%s, port:%d",index, serial, ip, port);
		sendWord(index, serial, "i am client");
		return 0;
	}

	void onDisCon(UI32 index, UI64 serial, const char* ip, UI32 port)
	{
		printf("disconnection, index:%d, serial:%llu, ip:%s, port:%d",index, serial, ip, port);
	}

	//io thread
	int onRead(UI32 index, UI64 serial, SockBuf* buf)
	{
		UI32 len=buf->readableBytes();

		//不完整的消息
		if(len <= MSGHEADERLEN)
			return 0;
		char header[MSGHEADERLEN]={0};
		buf->peek(header, MSGHEADERLEN);

		//根据网络协议，判断一条完整消息
		MsgHeader* msg_header=(MsgHeader*)header;

		//未满一条完整消息
		UI32 total_msg_len=MSGHEADERLEN+msg_header->len;
		if(len<total_msg_len)
			return 0;

		char* total_msg=new char[MSGHEADERLEN+msg_header->len];
		buf->pop(total_msg, msg_header->len+MSGHEADERLEN);

		std::string str_msg_body(total_msg+MSGHEADERLEN, msg_header->len);
		printf("%s\n", str_msg_body.c_str());

		onSend_(index, serial, total_msg, MSGHEADERLEN+msg_header->len);

		return 0;
	}

	void sendWord(UI32 index,UI64 serial,char const* word)
	{
		char* total_msg=new char[MSGHEADERLEN+strlen(word)];
		MsgHeader head;
		head.len=strlen(word);
		memcpy(total_msg,&head,sizeof(MsgHeader));
		memcpy(total_msg+sizeof(MsgHeader),word,strlen(word));
		onSend_(index, serial, total_msg,MSGHEADERLEN+head.len);
	}
private:
	OnSendCallback onSend_;
};

int main()
{
	NetEvent net_event;
	SelectIOService* io_service=new SelectIOService(
		MakeDelegate(&net_event, &NetEvent::onCon),
		MakeDelegate(&net_event, &NetEvent::onDisCon),
		MakeDelegate(&net_event, &NetEvent::onRead));

	net_event.registerOnSend(MakeDelegate(io_service, &SelectIOService::send));

	UI32 nCount=0;
	int ret=1;
	while(!io_service->connectPeer("127.0.0.1",5001))
	{
		Sleep(100);
	}
	//io_service.connect("127.0.0.1", 5002);

	while(true)
	{
		io_service->eventLoop();
		//OnFrame();
	}

	return 0;
}
