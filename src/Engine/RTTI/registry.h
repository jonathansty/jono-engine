#pragma once

#include <cassert>
#include <map>

#include "private/resolvers.h"

namespace rtti
{

class TypeInfo;

// Type database
class Registry final
{
public:

	static bool is_init() 
	{
		return _init;
	}
	static void init()
	{
		assert(!_init);
		_init = true;

		// Register some default types
		register_type<int>();
		register_type<unsigned int>();
		register_type<float>();
		register_type<double>();
		register_type<short>();
		register_type<std::string>();
	}

	template<typename T>
	static TypeInfo* get()
	{
		if (!_init)
		{
			init();
		}

		if (_types.find(std::type_index(typeid(T))) == _types.end())
		{
			_types[std::type_index(typeid(T))] = TypeResolver<T>::get();

		}
		return _types[std::type_index(typeid(T))];
	}

	template<typename T>
	static void register_type()
	{
		_types[std::type_index(typeid(T))] = TypeResolver<T>::get();
	}

	static void for_each_type(std::function<void(std::pair<std::type_index, TypeInfo*> const& pair)> func)
	{
		for (auto it = _types.begin(); it != _types.end(); ++it)
		{
			func(*it);
		}
	}

#ifdef _DEBUG
	static void dump_types()
	{
		for_each_type([](auto it)
			{
				printf("------------------\n");
				printf("Key: %d\n", int(it.first.hash_code()));
				printf("  Type Name: %s\n", it.second->get_name());
				printf("  Size: %zd\n", it.second->get_size());
				printf("------------------\n");
			});
	}
#endif
private:
	static bool _init;
	static std::map<std::type_index, TypeInfo*> _types;

};

__declspec(selectany) bool Registry::_init = false;
__declspec(selectany) std::map<std::type_index, TypeInfo*> Registry::_types;

}
