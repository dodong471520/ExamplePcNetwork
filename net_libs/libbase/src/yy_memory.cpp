#include "yy_memory.h"
#include "yy_exception.h"

NS_YY_BEGIN

MemoryPool* MemoryPool::getInstance()
{
	static MemoryPool s_memory_pool;
	return &s_memory_pool;
}

void* MemoryPool::mpAlloc(UI32 size)
{
	void* p = malloc(size);
	if(NULL == p)
		THROWNEXCEPT("malloc error.");

	return p;
}

void MemoryPool::mpFree(void* p)
{
	if(NULL == p)
		return;

	free(p);
}

void* NewAlloc::operator new(size_t size)
{
	return MPAlloc(size);
}

void NewAlloc::operator delete(void* p)
{
	MPFree(p);
}
void* NewAlloc::operator new[](size_t size)
{
	THROWNEXCEPT("not support new[]");
}

void NewAlloc::operator delete[](void* p)
{
	THROWNEXCEPT("not support delete[]");
}

NS_YY_END