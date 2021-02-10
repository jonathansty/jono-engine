#pragma once
// Based on RTTR and https://preshing.com/20180124/a-flexible-reflection-system-in-cpp-part-2/
#include <functional>
#include <typeindex>
#include <type_traits>
#include <optional>

#include "property.h"
#include "typeinfo.h"
#include "object.h"
#include "registry.h"
#include "method.h"

#define REFLECT(classname) \
	friend struct rtti::DefaultResolver; \
	static rtti::TypeInfo_Register<classname> _sType; \
public:\
	static void reflect(rtti::TypeInfo& type); \
	static rtti::TypeInfo* get_static_type() { return &_sType._info;}  \
	virtual rtti::TypeInfo* get_type() { return &_sType._info;}  

#define IMPL_REFLECT(classname) \
	rtti::TypeInfo_Register<classname> classname::_sType = rtti::TypeInfo_Register<classname>(#classname);  \
	void classname::reflect(rtti::TypeInfo& type)

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
		}

		~TypeInfo_Register()
		{
		}

		TypeInfo _info;
	};


}