#ifndef _MEMORY_H__
#define _MEMORY_H__


#include "yy_macros.h"
#include <stdlib.h>

NS_YY_BEGIN

//c语言方式开辟内存，不会调用构造函数和析构函数
class MemoryPool
{
public:
	static MemoryPool* getInstance();
	void* mpAlloc(UI32 size);
	void mpFree(void* p);
};

//c++方式开辟内存, 会调用构造函数和析构函数，new/delete的封装,通过继承方式来使用。
class NewAlloc
{
public:
	void* operator new(size_t size);
	void operator delete(void* p);

	//new[]操作符中的个数参数是数组的大小加上额外的存储对象数目的一些字节。尽量避免分配对象数组
	void* operator new[](size_t size);
	void operator delete[](void* p);
};

NS_YY_END

//封装malloc, free api. 如果以后要优化内存管理，可以修改该实现
#define MPAlloc(size)			MemoryPool::getInstance()->mpAlloc(size);
#define MPFree(p)				if(p){MemoryPool::getInstance()->mpFree(p); p=NULL;}

#endif
