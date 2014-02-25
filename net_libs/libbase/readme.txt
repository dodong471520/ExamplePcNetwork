
目标: 跨平台基本类库，封装win32, posix api, 支持基于libbase开发的应用程序跨平台。
项目架构: 导出头文件为跨平台头文件。具体实现放在win32和posix目录中。利用Impl方式实现跨平台
命名方式:
	文件名用YY前缀，防止和系统文件混淆。比如include "time.h"，编译器会去找系统文件，而不是项目文件
	类名，首字母大写
	类成员函数，首字母小写
	全局函数，首字母大写

不屏蔽安全警告。强制自己把strcpy改成strcpy_s, 如果有越界，则会发生assert错误。个人觉得比用strncpy的截取方式好。
1,截取了说明肯定逻辑上有问题。
2,strncpy的话，如果要拷贝的内容少，会填充0(waste time)



脚本: 用lua-5.1.4版本，利用lua tinker来封装
net: 利用windows iocp， 单个网络线程来处理数据收发，取得一个完整消息，插入到消息队列。
serv: 单个工作线程从消息队列取消息，处理。
定时器： 工作线程onTick()来实现。保证单线程无锁。



#ifdef 之类的预处理，在只有头文件的情况下，在main中#define才有效。不然要在编译器的预处理命令中添加宏才有效.所以用到预处理宏的文件，都在头文件中实现.
-->head only lib



跨平台方案：
利用条件编译，把编译条件相同的代码放到单独的文件中。即不同平台的程序由不同的文件(pimpl)
1,log--log impl--win32 log impl, linux log impl
2,log--win32 log impl, linux log impl(利用typedef),目前用这种方案，少定义一层log impl,而pimpl的好处(二级制兼容，头文件和实现文件分离)并没有减少

最上层接口
//Log.h
#ifdef win32
	class Win_Log;
	typedef LogImpl Win_Log;
#endif

class Log
{
public:
	Log();
	~Log();
	void logging(const char* data);
private:
LogImpl* m_impl;
}

//Log.cpp

#ifdef win32
#include "win32/win_log.h"
#endif

Log::Log() : m_impl(new LogImpl)
{}

Log::~Log()
{ delete m_impl;}

void Log::logging(const char* data){}

//windows
class Win_Log
{
public:
	void logging(const char* data);
}