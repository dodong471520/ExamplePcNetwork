#include "SockBuf.h"
#include <stdlib.h>
#include <string.h>
#include "YYException.h"
#include "SocketWrap.h"
NS_YY_BEGIN

SockBuf::SockBuf(const UI32 size)
{
	m_size=size;
	m_buf=(char*)malloc(m_size);
	m_read_index=0;
	m_write_index=0;
}

SockBuf::~SockBuf()
{
	free(m_buf);
}

void SockBuf::reset()
{
	m_read_index=0;
	m_write_index=0;
}

void SockBuf::pop(char* buf, UI32 len)
{
	peek(buf, len);
	m_read_index += len;
}

void SockBuf::peek(char* buf, UI32 len)
{
	if(m_read_index > m_write_index || m_write_index > m_size || m_read_index > m_size)
		THROWNEXCEPT("read index:%ud, write index:%ud, out of range.", m_read_index, m_write_index);

	UI32 size=readableBytes();
	if(len > size)
		THROWNEXCEPT("len:%ud, readable bytes:%ud, out of range.", len, size);

	memcpy(buf, m_buf+m_read_index, len);
}

int SockBuf::sockRead(int sd)
{
	ensureWritableBytes(512);

	int ret=0;
#ifdef WIN32
	ret= ::recv(sd, writeStart(), writableBytes(), 0);
	if(ret==0)
	{
		//对端断开连接
		return -1;
	}
#else
	ret=read(p->socket, p->read_buf.writeStart(), p->read_buf.writableBytes());
#endif

	if(ret<0)
	{
		//有错误
		return -2;
	}

	writeMove(ret);
	return 0;
}

int SockBuf::sockWrite(int sd)
{

	//写缓冲中有数据需要发送
	int readable_byets=readableBytes();
	if(readable_byets>0)
	{
		int bytes=0;
#ifdef WIN32
		bytes=::send(sd, readStart(), readableBytes(), 0);
#else
		bytes=write(sd, readStart(), readableBytes());
#endif

		if(bytes<0)
		{
			return -1;
		}

		readMove(bytes);
	}

	return 0;
}


char* SockBuf::writeStart()
{
	return m_buf+m_write_index;
}

char* SockBuf::readStart()
{
	return m_buf+m_read_index;
}

void SockBuf::readMove(UI32 len)
{
	m_read_index+=len;
	if(m_read_index > m_write_index)
		THROWNEXCEPT("write index:%u is not bigger than read index:%u", m_write_index, m_read_index);
}

void SockBuf::writeMove(UI32 len)
{
	m_write_index+=len;
	if(m_write_index > m_size)
		THROWNEXCEPT("write index is out of range.");
}

UI32 SockBuf::readableBytes()
{
	int nret=m_write_index - m_read_index;
	if(nret < 0)
		THROWNEXCEPT("write index:%u is not bigger than read index:%u", m_write_index, m_read_index);

	return nret;
}

UI32 SockBuf::writableBytes()
{
	int nret= m_size - m_write_index;
	if(nret < 0)
		THROWNEXCEPT("write index:%u is bigger than size:%u", m_write_index, m_size);

	return nret;
}

void SockBuf::makeSpace(UI32 len)
{
	UI32 readable_bytes=readableBytes();

	//空间不够，则开辟
	if (writableBytes() + m_read_index < len)
	{
		//开辟新空间
		char* new_buf=(char*)malloc(m_write_index+len);
		m_size=m_write_index+len;

		//拷贝可读数据
		memcpy(new_buf, m_buf+m_read_index, readable_bytes);
		m_read_index=0;
		m_write_index=readable_bytes;

		//保存就地址
		char* old_buf=m_buf;
		//指向新空间
		m_buf=new_buf;

		//回收旧空间
		if(old_buf)
			free(old_buf);
		old_buf=NULL;
	}
	//内部腾挪
	else
	{
		//自拷贝
		memmove(m_buf, m_buf+m_read_index,readable_bytes);
		m_read_index=0;
		m_write_index=readable_bytes;
	}
}

void SockBuf::ensureWritableBytes(UI32 len)
{
	if (writableBytes() < len)
	{
		makeSpace(len);
	}
}

void SockBuf::push(const char* buf, UI32 len)
{
	//1,判断大小，如果不够则重新开辟
	ensureWritableBytes(len);

	//2，拷贝
	memcpy(m_buf+m_write_index, buf, len);
	m_write_index += len;
	if(m_write_index > m_size)
		THROWNEXCEPT("write index:%u is bigger than size:%u", m_write_index, m_size);

}

NS_YY_END

