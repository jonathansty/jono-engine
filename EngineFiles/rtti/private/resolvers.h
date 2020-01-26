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

};

struct TypeResolver
{
	// TODO: Implement some SFINAE logic to deduce whether a type is built-in or not

	template<typename T>
	static TypeInfo* get()
	{
		return T::get_type();
	}

	template<>
	static TypeInfo* get<int>() { return PrimitiveTypeResolver::get_primitive_type<int>(); }

	template<>
	static TypeInfo* get<unsigned int>() { return PrimitiveTypeResolver::get_primitive_type<unsigned int>(); }

	template<>
	static TypeInfo* get<short>() { return PrimitiveTypeResolver::get_primitive_type<short>(); }

	template<>
	static TypeInfo* get<float>() { return PrimitiveTypeResolver::get_primitive_type<float>(); }

	template<>
	static TypeInfo* get<double>() { return PrimitiveTypeResolver::get_primitive_type<double>(); }

	template<>
	static TypeInfo* get<std::string>() { return PrimitiveTypeResolver::get_primitive_type<std::string>(); }

};
}