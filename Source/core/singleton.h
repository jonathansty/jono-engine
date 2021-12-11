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
	static void Shutdown();

private:

	static void CreateIfNull();
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
	CreateIfNull();

	return _obj;
}

template<typename T>
void TSingleton<T>::CreateIfNull()
{
	if (!_obj)
	{
		_obj = new T();
	}
}

template<typename T>
__declspec(selectany) T* TSingleton<T>::_obj;
