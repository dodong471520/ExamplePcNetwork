#ifndef _COROUTINE_H_20130204_
#define _COROUTINE_H_20130204_

#include "yy_macros.h"
#include "FastDelegate.h"
NS_YY_BEGIN

//http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

//考虑多层调用，一个参数Level保存层数
//利用定时器，让主线程每一帧都跑一遍注册成协程的函数
//把工作线程处理消息时，一些费时的处理用coroutine来封装(sleep)。保证工作线程不挂起


#define crBegin				static int state=0; switch(state) { case 0:
#define crReturn(x)			do { state=__LINE__; return x; case __LINE__:; } while (0)
#define crFinish			}



bool function(void)
 {
	static int n=10;
    crBegin;

    while(--n)
	{
		//printf("%d\n", n);
        crReturn(false);
	}

    crFinish;
	return true;
}
/*


	Coroutine test;
	test.create(function);

	while(true)
	{
		test.onTick();
		Sleep(1000);
	}

typedef fastdelegate::FastDelegate0<bool> CoroutineCallback;

class Coroutine
{
public:
	//run all coroutine event in every tick
	void onTick();

	void create(CoroutineCallback cb);
	void yield();
	void resume();

private:
	std::vector<int> coroutine_set_;
};*/


NS_YY_END
#endif