// 引用计数类.
class smart_count
{
public:
	smart_count(int c = 0) : use_count(c) {}
	~smart_count() {}

	// 增加引用计数, 并返回计数值.
	int addref() { return ++use_count; }
	// 减少引用计数, 并返回计数值.
	int release() { return --use_count; }

private:
	// 计数变量.
	int use_count;
};

// 智能指针.
template <class T>
class smart_ptr
{
public:
	// 构造指针, 并使引用计数置为1.
	explicit smart_ptr (T* ptr) : p(ptr), u(new smart_count(1))
	{}

	// 构造空指针.
	explicit smart_ptr () : p(NULL), u(NULL)
	{}

	// 智能指针析构.
	~smart_ptr (void)
	{
		// 如果引用计数等于0, 则删除数据和引用计数, 并置p为NULL.
		// 此处需要注意的是, 共用的u并未置为 NULL, 在其它指针析构
		// 时, p为NULL, 则不会重复delete.
		if (p && u->release() <= 0)
		{
			delete p;
			delete u;
			p = NULL;
		}		
	}

	// 智能指针拷贝构造函数.
	smart_ptr (const smart_ptr<T>& t)
	{
		p = t.p;
		u = t.u;

		if (u) // 必须判断空值.
		{
			u->addref();	// 增加引用计数.
		}
	}

	// 指针赋值.
	smart_ptr<T>& operator= (smart_ptr<T>& t)
	{
		// 增加引用计数.
		if (t.u)
		{
			t.u->addref();
		}

		T *temp_p = p;
		smart_count *temp_u = u;

		// 直接赋值.
		p = t.p;
		u = t.u;

		if (temp_p && temp_u->release() <= 0)
		{
			delete temp_p;
			delete temp_u;
		}

		// 返回当前smart_ptr指针.
		return *this;
	}

	// 重载->操作和*操作符.
	T *operator-> (void) { return p; }
	T& operator *(void) { return *p; }
	// 重载!操作符.
	bool operator! () const { return !p;}

	// 重载指针bool值操作符.
	typedef smart_ptr<T> this_type;
	typedef T * this_type::*unspecified_bool_type;
	operator unspecified_bool_type() const { return !p ? 0: &this_type::p; }
	// 得到原指针.
	T* get() { return p; }
	void reset(T* ptr)
	{
		T *temp_p = p;
		smart_count *temp_u = u;

		// 赋值, 如果是NULL, 则不创建引用计数.
		p = ptr;
		if (p)
			u = new smart_count(1);
		else
			u = NULL;

		if (temp_p && temp_u->release() <= 0)
		{
			delete temp_p;
			delete temp_u;
		}
	}

	void reset(smart_ptr<T>& t)
	{
		// 增加引用计数.
		if (t.u)
		{
			t.u->addref();
		}

		T *temp_p = p;
		smart_count *temp_u = u;

		// 赋值.
		p = t.p;
		u = t.u;

		if (temp_p && temp_u->release() <= 0)
		{
			delete temp_p;
			delete temp_u;
		}
	}

private:
	T* p;
	smart_count* u;
};

// 重载==操作符.
template<class T, class U> inline bool operator==(smart_ptr<T> & a, smart_ptr<U> & b)
{
	return a.get() == b.get();
}

// 重载!=操作符.
template<class T, class U> inline bool operator!=(smart_ptr<T> & a, smart_ptr<U> & b)
{
	return a.get() != b.get();
}