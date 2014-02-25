#ifndef _SOCK_H_
#define _SOCK_H_

#include <stddef.h>
#include "YYMacros.h"
NS_YY_BEGIN

//内存地址连续的缓冲
class SockBuf
{
public:
	SockBuf(const UI32 size=100);
	~SockBuf();

	void reset();
	void push(const char* buf, UI32 size);
	void pop(char* buf, UI32 len);
	void peek(char* buf, UI32 len);
	UI32 readableBytes();

public:
	//网络库内部用
	int sockRead(int sd);
	int sockWrite(int sd);

private:
	char* writeStart();
	char* readStart();

	void readMove(UI32 len);
	void writeMove(UI32 len);

	UI32 writableBytes();

	void ensureWritableBytes(UI32 len);

	void makeSpace(UI32 len);

private:
	char* m_buf;
	UI32 m_size;
	UI32 m_read_index;
	UI32 m_write_index;
};

NS_YY_END
#endif
