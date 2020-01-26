#include "stdafx.h"
#include "object.h"

#include "typeinfo.h"

rtti::Object::~Object()
{
	if (_type->_destructor)
	{
		_type->_destructor(_data);
	}

}

rtti::Object::Object() 
	: _data(nullptr)
	, _type(nullptr)
{

}

rtti::TypeInfo* rtti::Object::get_type() const
{
	return _type;
}
