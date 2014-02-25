#include <stdio.h>
#include <string>
#include "io_service.h"

USING_NS_YY
//#pragma comment(lib, "libbase.lib")
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

	//iocp thread or select io thread
	void onCon(UI32 index, UI64 serial, const char* ip, UI32 port)
	{
		printf("new connection, index:%d, serial:%llu, ip:%s, port:%d\n",index, serial, ip, port);
	}

	//io thread
	void onDisCon(UI32 index, UI64 serial, const char* ip, UI32 port)
	{
		printf("disconnection, index:%d, serial:%llu, ip:%s, port:%d\n",index, serial, ip, port);
	}

	//io thread
	int onRead(UI32 index, UI64 serial, SockBuf* buf)
	{
		const char* begin=buf->readStart();
		UI32 len=buf->readableBytes();

		//不完整的消息
		if(len <= MSGHEADERLEN)
			return 0;

		//根据网络协议，判断一条完整消息
		MsgHeader* msg_header=(MsgHeader*)begin;

		//未满一条完整消息
		UI32 total_msg_len=MSGHEADERLEN+msg_header->len;
		if(len<total_msg_len)
			return 0;

		std::string str_msg(begin, msg_header->len+MSGHEADERLEN);

		std::string str_msg_body(begin+MSGHEADERLEN, msg_header->len);
		printf("%s\n", str_msg_body.c_str());

		buf->readMove(total_msg_len);

		//onSend_(index, serial, str_msg.c_str(), str_msg.length());

		return 0;
	}

private:
	OnSendCallback onSend_;
};

int main()
{
	NetEvent net_event;
	IOService* io_service=new IOService(
		MakeDelegate(&net_event, &NetEvent::onCon),
		MakeDelegate(&net_event, &NetEvent::onDisCon),
		MakeDelegate(&net_event, &NetEvent::onRead));

	net_event.registerOnSend(MakeDelegate(io_service, &IOService::send));

	UI32 nCount=0;
	int ret=1;
	io_service->listen("127.0.0.1", 5001);
	//io_service.connect("127.0.0.1", 5002);

	while(true)
	{
		io_service->eventLoop(1024, 5);

		//OnFrame();
	}

	return 0;
}
