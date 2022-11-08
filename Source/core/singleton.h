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

template <typename T>
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

template <typename T>
void TSingleton<T>::shutdown()
{
	ASSERTMSG(_obj, "Singleton was never created! This is an invalid shutdown.");
	delete _obj;
	_obj = nullptr;
}

template <typename T>
T* TSingleton<T>::instance()
{
	ASSERT(_obj);
	return _obj;
}

template <typename T>
void TSingleton<T>::create()
{
	ASSERTMSG(!_obj, "Singleton already contains an instance. Double create called!");
	_obj = new T();
}

template <typename T>
__declspec(selectany) T* TSingleton<T>::_obj = nullptr;
