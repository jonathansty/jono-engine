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


class FunctionBase {
public:

	FunctionBase() {}
	virtual ~FunctionBase(){}

	virtual void invoke(rtti::Object const& obj) const = 0;
};

template<typename Class, typename Fn>
class Function : public FunctionBase {
public:
	Function(Fn fn) : _fn(fn) {}

	void invoke(rtti::Object const& obj) const {
		Class* s = obj.get<Class>();
		((*s).*_fn)();
	}

	// Function ptr
	Fn _fn;
};

}
