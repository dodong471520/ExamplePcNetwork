#include "io_service.h"


#ifdef IO_SELECT
#include "../src/Select_io_service.h"
#else
	#include "../src/Win_IOCP_io_service.h"
#endif
NS_YY_BEGIN

IOService::IOService(OnConCallback onCon, OnDisConCallback onDis, OnReadCallback onRead) : m_impl(new IOServiceImpl(onCon, onDis, onRead))
{}

IOService::~IOService()
{
	if(m_impl)
		delete m_impl;
}

bool IOService::connectPeer(const char* ip, UI32 port)
{
	return m_impl->connectPeer(ip, port);
}


void IOService::listen(const char* ip, UI32 port)
{
	m_impl->listen(ip, port);
}

void IOService::send(UI32 index, UI64 serial, const char* buf, UI32 len)
{
	m_impl->send(index, serial, buf, len);
}

void IOService::closePeer(UI32 index, UI64 serial)
{
	m_impl->closePeer(index, serial);
}

void IOService::eventLoop(UI32 count, UI32 time_out)
{
	m_impl->eventLoop(count, time_out);
}

NS_YY_END