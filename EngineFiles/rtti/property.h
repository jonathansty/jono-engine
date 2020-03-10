#pragma once


namespace rtti
{

class Object;
class TypeInfo;

// A property contains a type and a offset
class Property
{
public:
	TypeInfo* type;
	size_t offset;
	std::string name;

	std::function<void(void*, rtti::Object const&)> setter;
	std::function<void(void*, void**)> getter;
};

}
