#pragma once

namespace rtti 
{

class Registry;
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
	static Object create_as_ref(T* obj);

	template<typename T>
	static Object create_with_copy(T obj);

	// Gets a pointer to the object it's value if the types match. If no match it will return nullptr
	template<typename T>
	T* get() const;

	template<typename T>
	bool set_property(std::string const& field, T const& value);

	template<typename T>
	T const& get_property(std::string const& name);


	TypeInfo* get_type() const;


	template<typename T>
	bool is_type() const
	{
		return _type == Registry::template get<T>();
	}

	template<typename T>
	T& get_value() const
	{
		return *get<T>();
	}

private:
	Object(void* data, TypeInfo* type, bool is_ref = false)
		: _data(data)
		, _type(type)
		, _is_ref(is_ref)
	{

	}


public:
	TypeInfo* _type;
	void* _data;
	bool _is_ref;

	friend class TypeInfo;
};

template<typename T>
T const& rtti::Object::get_property(std::string const& name)
{
	static T invalid{};
	rtti::Property* property_info = _type->find_property(name);
	if (!property_info)
		return invalid;

	if (property_info->type != Registry::template get<T>())
		return invalid;

	if (property_info->getter)
	{
		T* result;
		property_info->getter(_data, (void**)&result);
		return *result;
	}

	return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(_data) + property_info->offset);
}

template<typename T>
bool rtti::Object::set_property(std::string const& field, T const& value)
{
	rtti::Property* property_info = _type->find_property(field);
	if (!property_info)
		return false;

	TypeInfo* property_type = property_info->type;

	if (property_type != Registry::template get<T>())
		return false;

	if (property_info->setter)
	{
		rtti::Object obj = rtti::Object::create_as_ref(const_cast<T*>(&value));
		property_info->setter(_data, obj);
		return true;
	}

	size_t offset = property_info->offset;
	size_t size = property_type->get_size();

	uint8_t* dst = (uint8_t*)_data;
	memcpy(dst + offset, &value, size);

	return true;
}

template<typename T>
T* rtti::Object::get() const
{
	if (Registry::template get<T>() == _type)
	{
		return reinterpret_cast<T*>(_data);
	}
	return nullptr;
}

template<typename T>
Object rtti::Object::create_as_ref(T* obj)
{
	rtti::TypeInfo* type = nullptr;
	if constexpr (DefaultResolver::IsReflected<T>::value)
	{
		return Object(obj, obj->get_type(), true);
	}
	else
	{
		return Object(obj, Registry::template get<T>(), true);
	}
}

template<typename T>
Object rtti::Object::create_with_copy(T obj)
{
	T* d = new T();
	*d = obj;
	return Object(d, Registry::template get<T>());
}

template<typename T, typename ...Args>
Object rtti::Object::create(Args... args)
{
	T* obj = new T(args...);
	return Object(obj, Registry::template get<T>());
}

}
