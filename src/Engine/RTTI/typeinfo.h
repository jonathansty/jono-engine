#pragma once

namespace rtti {
class Registry;
class Object;
class Property;
class FunctionBase;

template <typename T>
void ConstructObject(void* object) {
	new (object) T;
}

template <typename T>
void DestructObject(void* object) {
	if (std::is_array<T>::value)
		delete[]((T*)(object));
	else
		delete ((T*)(object));
}

// Calculates the offset of a member variable in a type.
// https://stackoverflow.com/questions/13180842/how-to-calculate-offset-of-a-class-member-at-compile-time
template <typename T, typename U>
constexpr size_t offsetOf(U T::*member) {
	return (char*)&((T*)nullptr->*member) - (char*)nullptr;
}

struct TypeFlags {
	enum Flags : uint32_t {
		None = 0,
		PrimitiveBit = 0x1,
	};
};

// Type info
class TypeInfo {
public:
	template <typename T>
	static TypeInfo create(const char* name) {
		auto des = DestructObject<T>;
		auto cons = ConstructObject<T>;
		return TypeInfo(name, sizeof(T), cons, des);
	}

	TypeInfo(const char* name, size_t size, void (*constructor)(void*), void (*destructor)(void*));

	virtual ~TypeInfo();

	const char* get_name() const;
	std::size_t get_size() const;

	template <typename T>
	void bind_parent() {
		_parent = T::get_static_type();
	}

	bool is_primitive();

	bool inherits(TypeInfo const* type);

	bool is(TypeInfo const* type);

	Property* find_property(std::string const& field);
	Property const* find_property(std::string const& field) const { return find_property(field); }

	FunctionBase* find_function(std::string const& name);
	FunctionBase const* find_function(std::string const& name) const { return find_function(name); }

	rtti::Object create_object();

	template <typename Class, typename Fn>
	void register_function(std::string const& name, Fn(Class::*fn)); 

	template <typename Class, typename MemberType>
	void register_property(std::string const& name, MemberType Class::*offset);

	template <typename Class, typename MemberType>
	void register_property(std::string const& name, void (*setterFn)(Class* obj, MemberType const* v), void (*getterFn)(Class* obj, MemberType** v));

	TypeInfo* _parent;
	std::vector<TypeInfo*> _children;

	void (*_constructor)(void*);
	void (*_destructor)(void*);

	std::string _name;
	std::size_t _size;
	TypeFlags::Flags _flags;

	using PropertyCollection = std::unordered_map<std::string, Property>;
	PropertyCollection _properties;

	using FunctionCollection = std::unordered_map<std::string, std::shared_ptr<FunctionBase>>;
	FunctionCollection _functions;
};
} // namespace rtti

#include "private/resolvers.h"

namespace rtti
{


template <typename Class, typename Fn>
void TypeInfo::register_function(std::string const& name, Fn(Class::*fn)) {
	_functions[name] = std::make_shared<Function<Class, Fn(Class::*)>>(fn);
}

template<typename Class, typename MemberType>
void TypeInfo::register_property(std::string const& name, void(*setterFn)(Class* obj, MemberType const* v), void(*getterFn)(Class * obj, MemberType** v))
{
	assert(_properties.find(name) == _properties.end());
	assert(this == Class::get_static_type());
	auto type = rtti::Registry::template get<MemberType>();

	_properties[name] = { type, 0, name };
	_properties[name].setter = [=](void* obj, rtti::Object const& data) {

		MemberType const* d = data.get<MemberType>();
		Class* class_obj = (Class*)obj;
		assert(class_obj && d);
		setterFn(class_obj, d);
	};
	_properties[name].getter = [=](void* obj, void** out) {
		Class* c = (Class*)obj;

		MemberType* ptr;
		getterFn(c, &ptr);

		*out = ptr;
	};


}

template<typename Class, typename MemberType>
void TypeInfo::register_property(std::string const& name, MemberType Class::* offset)
{
	assert(this == Class::get_static_type());
	auto type = rtti::Registry::template get<MemberType>();

	// Create getter and setter 
	std::size_t offset_of = offsetOf(offset);

	_properties[name] = { type, 0, name };
	_properties[name].setter = [offset_of](void* obj, rtti::Object const& data) {

		auto inner = [offset_of](Class* obj, MemberType const& v) {
			MemberType* object_value = (MemberType*)(((unsigned char*)obj) + offset_of);
			*object_value = v;
			printf("SET\n");
		};

		MemberType const* d = data.get<MemberType>();
		Class* class_obj = (Class*)obj;
		assert(class_obj && d);
		inner(class_obj, *d);
	};
	std::function<void(void*, void** out)> getter_fn = [offset_of](void* obj, void** out) {
		unsigned char* offset = (((unsigned char*)obj) + offset_of);

		*out = reinterpret_cast<void*>(offset);
	};
	 _properties[name].getter = getter_fn;
}


}



