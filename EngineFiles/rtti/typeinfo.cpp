#include "stdafx.h"
#include "typeinfo.h"

#include "property.h"
#include "constructor.h"
#include "destructor.h"

rtti::TypeInfo::~TypeInfo()
{
	// Free the constructor
	if (_constructor)
	{
		delete _constructor;
		_constructor = nullptr;
	}

	if (_destructor)
	{
		delete _destructor;
		_destructor = nullptr;
	}
}

rtti::TypeInfo::TypeInfo(const char* name, size_t size) : _name(name)
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

bool rtti::TypeInfo::inherits(TypeInfo* type)
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

rtti::Constructor* rtti::TypeInfo::get_constructor() const
{
	return _constructor;
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

