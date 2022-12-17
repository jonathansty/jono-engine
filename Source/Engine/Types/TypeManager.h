#pragma once

#include "Core/Singleton.h"

class IniStream;

class ENGINE_API TypeMetaData
{
public:
	void* (*m_ConstructFn)(void*);

	void (*m_SerializeFn)(class IFileStream* data, void*);

	const char* m_Name;
	const char* m_Path;

	u32 m_Size;
};

class ENGINE_API TypeManager final : public TSingleton<TypeManager, true>
{
public:
	TypeManager();
	~TypeManager();

	TypeManager(TypeManager const&) = delete;
	TypeManager const& operator=(TypeManager const&) = delete;

	TypeMetaData* AddType(const char* typePath)
	{
		m_Types[typePath] = {};

		return &m_Types[typePath];

	}

	void RemoveType(const char* name)
	{
		m_Types.erase(name);
	}

	TypeMetaData* FindType(std::string_view const& name);


	void* SerializeObject(std::string_view const& typePath, IFileStream* data);

	void* SerializeObject(std::string_view const& typePath, void* dst, IFileStream* data);

	void* CreateObject(std::string_view const& name);

	template<typename T>
	T* CreateObject()
	{
        return CreateObject<T>(T::GetStaticType());
	}

	template<typename T>
    T* CreateObject(TypeMetaData const* data)
    {
        return reinterpret_cast<T*>(CreateObject(data));
    }


    void* CreateObject(TypeMetaData const* data)
    {
        ASSERT(data->m_ConstructFn);
        return data->m_ConstructFn(nullptr);
    }



private:

	std::unordered_map<std::string, TypeMetaData> m_Types;
};

#include "FileStream.h"
#include "TypeRegistration.h"

#define ANON_VARIABLE(VarName) _#VarName##__LINE__

#define REGISTER_TYPE(Path, TypeName) \
	TypeRegistrationHelper<TypeName> _typeRegister_##TypeName = TypeRegistrationHelper<TypeName>(Path, #TypeName); \
	TypeMetaData const* TypeName::GetStaticType() { return _typeRegister_##TypeName.m_Data; } \
	TypeMetaData const* TypeName::GetType() { return TypeName::GetStaticType(); } 

#define CLASS_BASE(ClassName) \
	public:\
		static class TypeMetaData const* GetStaticType(); \
		virtual class TypeMetaData const* GetType();   \
		static void Serialize(class IFileStream* data, ClassName* obj) { obj->Serialize(data); } \
		void Serialize(class IFileStream* data);\

#define CLASS(ClassName, BaseClass) \
	using Super = BaseClass;        \
	CLASS_BASE(ClassName)

#define SERIALIZE_FN(TypeName) void TypeName::Serialize(IFileStream* fileStream)
#define SERIALIZE_PROPERTY(PropertyName) fileStream->ReadProperty<decltype(this->m_##PropertyName)>(#PropertyName, this->m_##PropertyName)
#define SERIALIZE_CONTAINER(PropertyName) reinterpret_cast<YamlStream*>(fileStream)->ReadContainer<decltype(this->m_##PropertyName)::value_type> (#PropertyName, this->m_##PropertyName)
#define SERIALIZE_SUPER() Super::Serialize(fileStream)
