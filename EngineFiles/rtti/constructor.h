#pragma once

namespace rtti
{

class Object;

// 
class Constructor
{
public:

	Constructor() {}
	~Constructor() {}

	using Argument = rtti::Object;
	virtual rtti::Object invoke();
};

}
