#pragma once
#include <functional>
#include <typeindex>
#include <optional>

#include "property.h"
#include "typeinfo.h"
#include "object.h"
#include "registry.h"

#define REFLECT(classname) \
	static rtti::TypeInfo_Register<classname> _sType; \
	static rtti::TypeInfo* get_type() { return &_sType._info;} \
	static void reflect(rtti::TypeInfo& type);

#define IMPL_REFLECT(classname) \
	rtti::TypeInfo_Register<classname> classname##::_sType = rtti::TypeInfo_Register<classname>(#classname);  \
	void classname##::reflect(rtti::TypeInfo& type)

namespace rtti
{
	template<typename T>
	class TConstructor;

	template<typename T>
	class TDestructor;

	// Helper class for our reflection structs to automatically register a class to the registry
	template<typename T>
	struct TypeInfo_Register
	{
		TypeInfo_Register(const char* name)
			: _info(rtti::TypeInfo::create<T>(name))
		{
			T::reflect(_info);
			_info._constructor = new TConstructor<T>();
			_info._destructor = new TDestructor<T>();
		}

		TypeInfo _info;
	};


}