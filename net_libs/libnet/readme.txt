

windows: iocp, select
linux: epoll, select


貌似流行用协程序来封装异步io的网络库接口。对协程还不怎么了解，且当前网络库的回调方式用的蛮舒服的啊，就暂无计划去更改了。


win32默认模式IOCP
linux默认模式EPOLL
预处理宏定义IO_SELECT



适配器：
定义统一接口，通过预处理定义来选择网络模式。

定义一个platform.h--行不通
main中#define-------行不通
预处理定义。ok. 以后可以写成脚本，select.cmd. iocp.cmd; linux: select.make, epoll.make


主流网卡： 1000M
单核 cpu（奔三）就可以达到10000page／second，在公网上
