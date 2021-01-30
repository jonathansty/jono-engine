#include "engine.stdafx.h"
#include "typeinfo.h"

#include "property.h"
#include "object.h"

rtti::TypeInfo::~TypeInfo()
{
	// Free the constructor
	if (_constructor)
	{
		_constructor = nullptr;
	}

	if (_destructor)
	{
		_destructor = nullptr;
	}
}

rtti::TypeInfo::TypeInfo(const char* name, size_t size, void(*constructor)(void*), void(*destructor)(void*)) 
	: _name(name)
	, _constructor(constructor)
	, _destructor(destructor)
	, _size(size)
	, _properties()
	, _parent(nullptr)
	, _children()
	, _flags()
{

}

const char* rtti::TypeInfo::get_name() const
{
	return _name.c_str();
}

std::size_t rtti::TypeInfo::get_size() const
{
	return _size;
}

bool rtti::TypeInfo::is_primitive()
{
	return (_flags & TypeFlags::PrimitiveBit) == TypeFlags::PrimitiveBit;
}

bool rtti::TypeInfo::inherits(TypeInfo const* type)
{
	TypeInfo* parent = _parent;
	while (parent != nullptr)
	{
		if (parent == type)
		{
			return true;
		}
		parent = parent->_parent;
	}
	return false;
}

rtti::Property* rtti::TypeInfo::find_property(std::string const& field)
{
	if (auto it = _properties.find(field); it != _properties.end())
	{
		return &it->second;
	}

	// Recurse into parent
	if (_parent)
	{
		return _parent->find_property(field);
	}

	return nullptr;
}

rtti::FunctionBase* rtti::TypeInfo::find_function(std::string const& name) {
	if (auto it = _functions.find(name); it != _functions.end()) {
		return it->second.get();
	}

	// Recurse into parent
	if (_parent) {
		return _parent->find_function(name);
	}

	return nullptr;
}

rtti::Object rtti::TypeInfo::create_object()
{
	void* data = malloc(_size);
	_constructor(data);
	return rtti::Object(data, this);
}

bool rtti::TypeInfo::is(TypeInfo const* type)
{
	return this == type;

}

