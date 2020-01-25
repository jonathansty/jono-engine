#pragma once

namespace rtti
{

	class Constructor;
	class Destructor;

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
			return TypeInfo(name, sizeof(T));
		}

		TypeInfo(const char* name, size_t size);

		~TypeInfo();

		const char* get_name() const;
		std::size_t get_size() const;

		bool is_primitive();

		bool inherits(TypeInfo* type);

		inline class Property* find_property(std::string const& field);

		template<typename Class, typename MemberType>
		void register_property(std::string const& name, MemberType Class::* offset);

		Constructor* get_constructor() const;

		TypeInfo* _parent;
		std::vector<TypeInfo*> _children;

		Constructor* _constructor;
		Destructor* _destructor;

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
	assert(this == Class::get_type());
	auto type = TypeResolver::template get<MemberType>();
	_properties[name] = { type, offsetOf(offset), name };
}


