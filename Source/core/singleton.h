#pragma once

template<typename T>
void safe_delete(T*& obj)
{
	if (obj)
	{
		delete obj;
		obj = nullptr;
	}
}

template<typename T>
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
	static void Shutdown();
private:

	static T* _obj;
};

template<typename T>
void TSingleton<T>::Shutdown()
{
	if (_obj)
	{
		delete _obj;
		_obj = nullptr;
	}

}

template<typename T>
T* TSingleton<T>::instance()
{
	ASSERT(_obj);
	return _obj;
}

template<typename T>
void TSingleton<T>::create()
{
	if (!_obj)
	{
		_obj = new T();
	}
}

template<typename T>
__declspec(selectany) T* TSingleton<T>::_obj;
