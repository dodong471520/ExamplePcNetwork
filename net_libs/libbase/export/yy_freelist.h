/*
@author:  junliang
@brief:   freelist
@time:    20130327
*/
#ifndef _FREELIST_H_
#define _FREELIST_H_

#include "yy_macros.h"
#include "yy_memory.h"
#include "yy_synchronize.h"
NS_YY_BEGIN

//用new/delete, 而不是malloc/free, 好处是TYPE结构体可以包含类，会自动初始化
template<typename TYPE>
class FreeList
{
private:
	//非侵入式链表
	struct Node
	{
		TYPE data_;		//data一定要在第一个，这样，free的时候强转okay
		Node * next_node_;
	};

public:
	FreeList();
	~FreeList();

	//分配新的elem
	TYPE * allocElem();

	//释放elem
	void freeElem(TYPE * p);

	//void reserve();预分配
private:
	void clear();
private:
	Node *	free_list_;		//空闲链表
	CSLock	lock_;			//锁
};



template<typename T>
FreeList<T>::FreeList()
{
	free_list_ = NULL;
}

template<typename T>
FreeList<T>::~FreeList()
{
	clear();
}


template<typename T>
T * FreeList<T>::allocElem()
{
	//return new TYPE();
	if (free_list_)
	{
		AutoLock lock(&lock_);
		if (free_list_)
		{
			Node* p = free_list_;
			free_list_ = p->next_node_;
			return &p->data_;
		}
	}

	Node * p=new Node;
	p->next_node_ = NULL;
	return &p->data_;
}

template<typename T>
void FreeList<T>::freeElem(T * pData)
{
	if(NULL == pData)
		return;

	//delete pData;
	AutoLock autolock(&lock_);
	Node* p = (Node *)pData;
	p->next_node_ = free_list_;
	free_list_ = p;

}


template<typename T>
void FreeList<T>::clear()
{
	AutoLock autolock(&lock_);

	Node * p;
	Node * t;
	p = free_list_;
	while (p)
	{
		t = p;
		p = p->next_node_;
		delete p;
	}

	free_list_ = NULL;
}


NS_YY_END
#endif