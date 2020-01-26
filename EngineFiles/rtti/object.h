#pragma once

namespace rtti 
{

class TypeInfo;

// RTTI Object containing the actual object data and type info related to the object
// TODO: Fix constructing and destructing of objects. Constructions works but causes memory leaks
// as we are "deleting" a void pointer which is not calling any destructors
class Object
{
public:
	~Object();
	Object();

	// TODO: Implement copy operators  
	Object(Object const&) = delete;
	Object& operator=(Object const&) = delete;


	template<typename T, typename ...Args>
	static Object create(Args ... args);

	template<typename T>
	static Object create_with_copy(T obj);

	// Gets a pointer to the object it's value if the types match. If no match it will return nullptr
	template<typename T>
	T* get();

	template<typename T>
	bool set_property(std::string const& field, T const& value);

	template<typename T>
	T* get_property(std::string const& name);


	TypeInfo* get_type() const;

private:
	Object(void* data, TypeInfo* type)
		: _data(data)
		, _type(type)
	{

	}


public:
	TypeInfo* _type;
	void* _data;

	friend class TypeInfo;
};

template<typename T>
T* rtti::Object::get_property(std::string const& name)
{
	rtti::Property* property_info = _type->find_property(name);
	if (!property_info)
		return nullptr;

	if (property_info->type != TypeResolver::template get<T>())
		return nullptr;

	return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(_data) + property_info->offset);
}

template<typename T>
bool rtti::Object::set_property(std::string const& field, T const& value)
{
	rtti::Property* property_info = _type->find_property(field);
	if (!property_info)
		return false;

	TypeInfo* property_type = property_info->type;

	if (property_type != TypeResolver::template get<T>())
		return false;

	size_t offset = property_info->offset;
	size_t size = property_type->get_size();

	uint8_t* dst = (uint8_t*)_data;
	memcpy(dst + offset, &value, size);

	return true;
}

template<typename T>
T* rtti::Object::get()
{
	if (TypeResolver::template get<T>() == _type)
	{
		return reinterpret_cast<T*>(_data);
	}
	return nullptr;
}

template<typename T>
Object rtti::Object::create_with_copy(T obj)
{
	T* d = new T();
	memcpy(d, (const void*)(&obj), sizeof(T));
	return Object(d, TypeResolver::template get<T>());
}

template<typename T, typename ...Args>
Object rtti::Object::create(Args ... args)
{
	T* obj = new T(args...);
	return Object(obj, TypeResolver::template get<T>());
}

}
