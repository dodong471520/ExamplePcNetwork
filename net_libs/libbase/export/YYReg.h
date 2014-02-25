#ifndef _REG_HPP_
#define _REG_HPP_

#include "YYMacros.h"
#include "YYException.h"
#include "FastDelegate.h"
#include <map>

#define MAXREGERRLEN 100

NS_YY_BEGIN

//注册消息和消息处理函数，invoke指定消息和函数

class Notification
{
public:
	typedef fastdelegate::FastDelegate4<UI32, UI64, const char*, UI32> Fun;

	static Notification* getInstance()
	{
		static Notification s_Notification;
		return &s_Notification;
	}

	void Bind(const int& key, Fun bindItem)
	{
		if(NULL != bindItem)
			m_mpBindContainer[key] = bindItem;
	}

	void Invoke(const int& key, UI32 index, UI64 serial, const char* buf, UI32 len)
	{
		std::map<int, Fun>::iterator it=m_mpBindContainer.find(key);
		if(m_mpBindContainer.end() == it)
		{
			THROWNEXCEPT("not support key:%d", key);
		}

		it->second(index, serial, buf, len);
	}

private:
	std::map<int, Fun> m_mpBindContainer;
};


template<class Key, class Cls, class Retn=void, class Compare = std::less<Key>>
class IBindDispatch0
{
public:
	typedef Retn (Cls::*Fun)(void);

	static IBindDispatch0* getInstance()
	{
		static IBindDispatch0 s_IBindDispatch0;
		return &s_IBindDispatch0;
	}

	void Bind(const Key& key, Fun bindItem)
	{
		if(NULL != bindItem)
			m_mpBindContainer[key] = bindItem;
	}

	Retn Invoke(const Key& key, Cls& cls)
	{
		std::map<key, Fun, Compare>::iterator it=m_mpBindContainer.find(key);
		if(m_mpBindContainer.end() == it)
		{
			char err[MAXREGERRLEN]={0};
			sprintf_s(err, sizeof(err), "not support key:%d", key);
			THROWNEXCEPT(err);
		}

		return (cls.*(it->second))();
	}

private:
	std::map<Key, Fun, Compare> m_mpBindContainer;
};

template<class Key, class Cls, class Param1, class Retn=void, class Compare = std::less<Key>>
class IBindDispatch1
{
public:

	//定义函数指针
	typedef Retn (Cls::*Fun)(Param1);

	static IBindDispatch1* getInstance()
	{
		static IBindDispatch1 s_IBindDispatch1;
		return &s_IBindDispatch1;
	}

	void Bind(const Key& key, Fun bindItem)
	{
		if(NULL != bindItem)
			m_mpBindContainer[key] = bindItem;
	}

	Retn Invoke(const Key& key, Cls& cls, Param1 arg1)
	{
		std::map<Key, Fun, Compare>::iterator it=m_mpBindContainer.find(key);
		if(m_mpBindContainer.end() == it)
		{
			char err[MAXREGERRLEN]={0};
			sprintf_s(err, sizeof(err), "not support key:%d", key);
			THROWNEXCEPT(err);
		}

		return (cls.*(it->second))(arg1);
	}

private:
	std::map<Key, Fun, Compare> m_mpBindContainer;
};


template<class Key, class Cls, class Param1, class Param2, class Param3, class Retn=void, class Compare = std::less<Key>>
class IBindDispatch3
{
public:

	//定义函数指针
	typedef Retn (Cls::*Fun)(Param1, Param2, Param3);

	static IBindDispatch1* getInstance()
	{
		static IBindDispatch3 s_IBindDispatch3;
		return &s_IBindDispatch3;
	}

	void Bind(const Key& key, Fun bindItem)
	{
		if(NULL != bindItem)
			m_mpBindContainer[key] = bindItem;
	}

	Retn Invoke(const Key& key, Cls& cls, Param1 arg1, Param2 arg2, Param3 arg3)
	{
		std::map<Key, Fun, Compare>::iterator it=m_mpBindContainer.find(key);
		if(m_mpBindContainer.end() == it)
		{
			char err[MAXREGERRLEN]={0};
			sprintf_s(err, sizeof(err), "not support key:%d", key);
			THROWNEXCEPT(err);
		}

		return (cls.*(it->second))(arg1, arg2, arg3);
	}

private:
	std::map<Key, Fun, Compare> m_mpBindContainer;
};

NS_YY_END
#endif