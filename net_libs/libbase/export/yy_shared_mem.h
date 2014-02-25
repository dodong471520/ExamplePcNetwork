#ifndef _SharedMemORY_H_20130204_
#define _SharedMemORY_H_20130204_


#include "yy_macros.h"

NS_YY_BEGIN

/************************************************************************/
/* 
@class:    ShareMem
@brief:    共享内存

ShareMem<ShareMemoryTest> g_smdt("test");
g_smdt->index = num;
g_smdt->msg = strMsg;
*/
/************************************************************************/
template<class ShareMemoryDataType>
class ShareMem
{
public:
	ShareMem();
	ShareMem(const char* name);
	~ShareMem();

	void createShareMemory(const char* name);

	//注意锁
	ShareMemoryDataType *operator->();
	ShareMemoryDataType* getData();
	void clear();

private:
	char	shared_name_[MAXSHAREMEMLEN];			//共享内存标识名
	unsigned long	max_data_size;					//共享内存的最大值
	ShareMemoryDataType*	data_;					//共享内存中存放的数据
	bool	created_;								//共享内存是否已经被打开
	HANDLE	file_mapping_;							//共享内存文件的句柄
};



template<class ShareMemoryDataType>
ShareMem<ShareMemoryDataType>::ShareMem()
{
	data_ = NULL;
	file_mapping_ = NULL;
}


template<class ShareMemoryDataType>
ShareMem<ShareMemoryDataType>::ShareMem(const char* name)
{
	ShareMem();
	createShareMemory(name);
}

template<class ShareMemoryDataType>
ShareMem<ShareMemoryDataType>::~ShareMem()
{
	if(data_)
	{
		//把映射对象从当前应用程序的地址空间unmap
		UnmapViewOfFile(data_);
		data_ = NULL;
	}

	if(file_mapping_)
	{
		CloseHandle(file_mapping_);
	}
}

template<class ShareMemoryDataType>
ShareMemoryDataType *ShareMem<ShareMemoryDataType>::operator->()
{
	return data_;
}

template<class ShareMemoryDataType>
ShareMemoryDataType* ShareMem<ShareMemoryDataType>::getData()
{
	return data_;
}

template<class ShareMemoryDataType>
void ShareMem<ShareMemoryDataType>::clear()
{
	memset(data_, 0, max_data_size); 
}

template<class ShareMemoryDataType>
void ShareMem<ShareMemoryDataType>::createShareMemory(const char* name)
{
	max_data_size = sizeof(ShareMemoryDataType);

	strcpy_s(shared_name_, sizeof(shared_name_), name);

	//创建文件映射内核对象
	file_mapping_ = CreateFileMappingA(INVALID_HANDLE_VALUE,
		NULL,							// 默认的安全属性
		PAGE_READWRITE,					// 可读写文件的数据
		0,								// size: high 32-bits；对于4GB以下的文件，该参数总是0
		sizeof(ShareMemoryDataType),	// size: low 32-bits；映射文件的字节数
		name);							// 命名该映射对象，用于和其他进程共享该对象

	if(file_mapping_ == NULL)
	{
		int err = GetLastError();
		THROWNEXCEPT("CreateFileMappingA error no:%d", err);
	}

	//判断是否第一个访问共享内存的程序
	created_ = (GetLastError() == ERROR_ALREADY_EXISTS); 

	//// 设置共享内存的访问权限
	//SetNamedSecurityInfo(name,
	//	SE_KERNEL_OBJECT,
	//	DACL_SECURITY_INFORMATION,	//// change only the object's DACL
	//	0,	// do not change owner or group
	//	0,	// do not change owner or group
	//	(PACL)NULL,	//no   DACL   specified   -   allows   everyone   access 
	//	NULL);	// do not change SACL系统访问控制列表


	//将一个文件映射对象映射到当前应用程序的地址空间
	data_ = (ShareMemoryDataType*)MapViewOfFile(file_mapping_,
		FILE_MAP_READ | FILE_MAP_WRITE,		//可以读写文件
		0,	// high offset:  map from
		0,	// low offset:   beginning
		0);	 // default: map entire file

	if(data_ == NULL)
	{
		CloseHandle(file_mapping_);
		THROWNEXCEPT("MapViewOfFile error.");
	}

	//如果是第一个创建共享内存的程序，则初始化
	if (!created_) 
	{
		clear();
	}
}


NS_YY_END
#endif