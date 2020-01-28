#pragma once

namespace rtti
{
	class Object;

	template<typename T>
	void ConstructObject(void* object)
	{
		new (object) T;
	}

	template<typename T>
	void DestructObject(void* object)
	{
		if (std::is_array<T>::value)
			delete[]((T*)(object));
		else
			delete ((T*)(object));
	}

	// Calculates the offset of a member variable in a type.
	// https://stackoverflow.com/questions/13180842/how-to-calculate-offset-of-a-class-member-at-compile-time
	template<typename T, typename U> constexpr size_t offsetOf(U T::* member)
	{
		return (char*)&((T*)nullptr->*member) - (char*)nullptr;
	}





	struct TypeFlags
	{
		enum Flags : uint32_t
		{
			None = 0,
			PrimitiveBit = 0x1,
		};
	};


	// Type info
	class TypeInfo final
	{
	public:

		template<typename T>
		static TypeInfo create(const char* name)
		{
			auto des = DestructObject<T>;
			auto cons = ConstructObject<T>;
			return TypeInfo(name, sizeof(T), cons, des);
		}

		TypeInfo(const char* name, size_t size, void(*constructor)(void*), void(*destructor)(void*));

		~TypeInfo();

		const char* get_name() const;
		std::size_t get_size() const;

		bool is_primitive();

		bool inherits(TypeInfo* type);

		bool is(TypeInfo* type);

		class Property* find_property(std::string const& field);

		rtti::Object create_object();

		template<typename Class, typename MemberType>
		void register_property(std::string const& name, MemberType Class::* offset);

		TypeInfo* _parent;
		std::vector<TypeInfo*> _children;

		void  (*_constructor)(void*);
		void  (*_destructor)(void*);

		std::string _name;
		std::size_t _size;
		TypeFlags::Flags _flags;

		using PropertyCollection = std::unordered_map<std::string, Property>;
		PropertyCollection _properties;
	};
}

#include "private/resolvers.h"

template<typename Class, typename MemberType>
void rtti::TypeInfo::register_property(std::string const& name, MemberType Class::* offset)
{
	assert(this == Class::get_static_type());
	auto type = TypeResolver::template get<MemberType>();
	_properties[name] = { type, offsetOf(offset), name };
}


