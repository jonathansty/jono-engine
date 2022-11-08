#pragma once

template <typename T>
void safe_delete(T*& obj)
{
	if (obj)
	{
		delete obj;
		obj = nullptr;
	}
}

template <typename T, bool LazyCreate = false>
class TSingleton
{
public:
	TSingleton()
	{
	}

	virtual ~TSingleton()
	{
	}

	static T* instance();

	static void create();
	static void shutdown();

private:
	static T* _obj;
};

template <typename T, bool LazyCreate>
void TSingleton<T, LazyCreate>::shutdown()
{
	ASSERTMSG(_obj, "Singleton was never created! This is an invalid shutdown.");
	delete _obj;
	_obj = nullptr;
}

template <typename T, bool LazyCreate>
T* TSingleton<T, LazyCreate>::instance()
{
	if constexpr(LazyCreate)
	{
		if(!_obj)
		{
			create();
		}
	}
	else
	{
		ASSERT(_obj);
	}
	return _obj;
}

template <typename T, bool LazyCreate>
void TSingleton<T, LazyCreate>::create()
{
	ASSERTMSG(!_obj, "Singleton already contains an instance. Double create called!");
	_obj = JONO_NEW(T);
}

template <typename T, bool LazyCreate>
__declspec(selectany) T* TSingleton<T, LazyCreate>::_obj = nullptr;
