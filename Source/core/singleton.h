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

	static T* Instance();

	static void Create();
	static void Shutdown();

private:
	static T* _obj;
};

template <typename T, bool LazyCreate>
void TSingleton<T, LazyCreate>::Shutdown()
{
	ASSERTMSG(_obj, "Singleton was never created! This is an invalid shutdown.");
	delete _obj;
	_obj = nullptr;
}

template <typename T, bool LazyCreate>
T* TSingleton<T, LazyCreate>::Instance()
{
	if constexpr(LazyCreate)
	{
		if(!_obj)
		{
			Create();
		}
	}
	else
	{
		ASSERT(_obj);
	}
	return _obj;
}

template <typename T, bool LazyCreate>
void TSingleton<T, LazyCreate>::Create()
{
	ASSERTMSG(!_obj, "Singleton already contains an instance. Double create called!");
	_obj = JONO_NEW(T);
}

template <typename T, bool LazyCreate>
__declspec(selectany) T* TSingleton<T, LazyCreate>::_obj = nullptr;
