#include "yy_shared_mem.h"
#include "yy_exception.h"
#include <Windows.h>
#define MAXSharedMemLEN 100

NS_YY_BEGIN

class SharedMemImpl
{
public:
	SharedMemImpl(const char* name, size_t size);
	~SharedMemImpl();
	void* getData();

private:
	void createSM(const char* name, size_t size);

private:
	void* data_;
	char	shared_name_[MAXSharedMemLEN];			//共享内存标识名
	unsigned long	max_data_size;					//共享内存的最大值
	bool	created_;								//共享内存是否已经被打开
	HANDLE	file_mapping_;							//共享内存文件的句柄
};


SharedMemImpl::SharedMemImpl(const char* name, size_t size)
{
	createSM(name, size);
}

SharedMemImpl::~SharedMemImpl()
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

void* SharedMemImpl::getData()
{
	return data_;
}

void SharedMemImpl::createSM(const char* name, size_t size)
{
	strcpy_s(shared_name_, sizeof(shared_name_), name);

	//创建文件映射内核对象
	file_mapping_ = CreateFileMappingA(INVALID_HANDLE_VALUE,
		NULL,							// 默认的安全属性
		PAGE_READWRITE,					// 可读写文件的数据
		0,								// size: high 32-bits；对于4GB以下的文件，该参数总是0
		size,							// size: low 32-bits；映射文件的字节数
		name);							// 命名该映射对象，用于和其他进程共享该对象

	if(!file_mapping_)
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
	data_ = MapViewOfFile(file_mapping_,
		FILE_MAP_READ | FILE_MAP_WRITE,		//可以读写文件
		0,	// high offset:  map from
		0,	// low offset:   beginning
		0);	 // default: map entire file

	if(!data_)
	{
		CloseHandle(file_mapping_);
		THROWNEXCEPT("MapViewOfFile error.");
	}

	//如果是第一个创建共享内存的程序，则初始化
	if (!created_) 
	{
		memset(data_, 0, size);
	}
}


NS_YY_END