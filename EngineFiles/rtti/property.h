#pragma once


namespace rtti
{

class TypeInfo;

// A property contains a type and a offset
class Property
{
public:
	TypeInfo* type;
	size_t offset;
	std::string name;
};

}
