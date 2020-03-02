#pragma once

namespace rtti
{

struct PrimitiveTypeResolver
{
	template<typename T>
	static TypeInfo* get_primitive_type();

#define DECLARE_PRIMITIVE_TYPE(type)  \
	template<> \
	static TypeInfo* get_primitive_type<type>() \
	{ \
		static TypeInfo _staticInt = TypeInfo(#type, sizeof(type), ConstructObject<type>, DestructObject<type>); \
		_staticInt._flags = TypeFlags::Flags(int(TypeFlags::PrimitiveBit) | int(_staticInt._flags)); \
		return &_staticInt; \
	}

	DECLARE_PRIMITIVE_TYPE(int);
	DECLARE_PRIMITIVE_TYPE(unsigned int);
	DECLARE_PRIMITIVE_TYPE(float);
	DECLARE_PRIMITIVE_TYPE(double);
	DECLARE_PRIMITIVE_TYPE(short);
	DECLARE_PRIMITIVE_TYPE(std::string);
#undef DECLARE_PRIMITIVE_TYPE

	template<>
	static TypeInfo* get_primitive_type<void>() 
	{ 
		static TypeInfo _staticInt = TypeInfo("void", 0, nullptr, nullptr); 
		_staticInt._flags = TypeFlags::Flags(int(TypeFlags::PrimitiveBit) | int(_staticInt._flags)); 
		return &_staticInt; 
	}

	template<>
	static TypeInfo* get_primitive_type<XMVECTOR>()
	{
		static TypeInfo _staticInt = TypeInfo("XMVECTOR", 0, nullptr, nullptr);
		return &_staticInt;
	}

	template<>
	static TypeInfo* get_primitive_type<XMFLOAT3>()
	{
		static TypeInfo _staticInt = TypeInfo("XMFLOAT3", 0, nullptr, nullptr);
		return &_staticInt;
	}


};


struct DefaultResolver
{
	template <typename T> static char func(decltype(&T::_sType));
	template <typename T> static int func(...);
	template <typename T>
	struct IsReflected {
		enum { value = (sizeof(func<T>(nullptr)) == sizeof(char)) };
	};

	// TODO: Implement some SFINAE logic to deduce whether a type is built-in or not
	template<typename T, typename std::enable_if_t<IsReflected<T>::value, int> = 0>
	static TypeInfo* get()
	{
		return T::get_static_type();
	}

	template<typename T, typename std::enable_if_t<!IsReflected<T>::value,int> = 0>
	static TypeInfo* get() 
	{ 
		return PrimitiveTypeResolver::get_primitive_type<T>(); 
	}
};

template<typename T>
struct TypeResolver
{
	static TypeInfo* get()
	{
		return DefaultResolver::template get<T>();
	}
};


class TypeInfo_Vector : public TypeInfo
{
public:
	template<typename T>
	static TypeInfo_Vector create()
	{
		TypeInfo_Vector v{};
		char buffer[256];
		sprintf_s(buffer, "std::vector<%s>", Registry::template get<T>()->get_name());
		v._name = std::string(buffer);

		v._constructor = ConstructObject<std::vector<T>>;
		v._destructor = DestructObject<std::vector<T>>;
		v._size = sizeof(std::vector<T>);
		v._wrapped = Registry::template get<T>();
		return v;
	}

	TypeInfo_Vector() : TypeInfo("std::vector<>", 0, nullptr,nullptr)
	{
	}
	~TypeInfo_Vector() {}

	TypeInfo* _wrapped;
	template<typename T>
	friend struct  TypeResolver;
};

template<typename T>
struct TypeResolver<std::vector<T>>
{
	static TypeInfo* get()
	{
		static TypeInfo_Vector v = TypeInfo_Vector::create<T>();
		return &v;
	}
};

}