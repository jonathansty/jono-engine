#pragma once

#include "Core/Singleton.h"

class IniStream;

class TypeMetaData
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

	void* CreateObject(std::string_view const& name) 
	{
		if(TypeMetaData* obj = FindType(name); obj)
		{
			ASSERT(obj->m_ConstructFn);
			return obj->m_ConstructFn(nullptr);
		}
		return nullptr;
	}

	void* SerializeObject(std::string_view const& typePath, IFileStream* data)
	{
		if (TypeMetaData* obj = FindType(typePath); obj)
		{
			ASSERT(obj->m_SerializeFn);

			void* typeData = obj->m_ConstructFn(nullptr);
			obj->m_SerializeFn(data, typeData);
			return typeData;
		}
		return nullptr;
	}

	void* SerializeObject(std::string_view const& typePath, void* dst, IFileStream* data)
	{
		if (TypeMetaData* obj = FindType(typePath); obj)
		{
			ASSERT(obj->m_SerializeFn);

			void* typeData = obj->m_ConstructFn(dst);
			obj->m_SerializeFn(data, typeData);
			return typeData;
		}
		return nullptr;
	}



	template<typename T>
	T* CreateObject()
	{
		TypeMetaData const* metaData = T::GetStaticType();
		ASSERT(metaData->m_ConstructFn);
		return static_cast<T*>(metaData->m_ConstructFn(nullptr));
	
	}

private:

	std::unordered_map<std::string, TypeMetaData> m_Types;


};

namespace Internal
{

struct BaseType
{
};

} // namespace Internal

template<typename T>
struct TypeRegistrationHelper 
{
public:
	TypeRegistrationHelper(const char* typePath, const char* typeName);

	~TypeRegistrationHelper();

	TypeMetaData* m_Data;
	const char* m_Path;
};

template<typename T>
TypeRegistrationHelper<T>::TypeRegistrationHelper(const char* typePath, const char* typeName)
		: m_Path(typePath)
{
	m_Data = TypeManager::instance()->AddType(m_Path);

	m_Data->m_Name = typeName;
	m_Data->m_Path = typePath;
	m_Data->m_ConstructFn = [](void* dest)->void* 
	{
		if(dest)
		{
			return new (dest) T();
		}
		else
		{
			return new T(); 
		}
	};

	m_Data->m_SerializeFn = [](IFileStream* iniStream, void* data) { T::Serialize(iniStream, (T*)data); };
}

template<typename T>
TypeRegistrationHelper<T>::~TypeRegistrationHelper()
{
	TypeManager::instance()->RemoveType(m_Path);
}


#define ANON_VARIABLE(VarName) _#VarName##__LINE__

#define REGISTER_TYPE(Path, TypeName) \
	TypeRegistrationHelper<TypeName> _typeRegister_##TypeName = TypeRegistrationHelper<TypeName>(Path, #TypeName); \
	TypeMetaData const* TypeName::GetStaticType() { return _typeRegister_##TypeName.m_Data; } \
	TypeMetaData const* TypeName::GetType() { return TypeName::GetStaticType(); } 

#define CLASS_BASE(ClassName) \
	static class TypeMetaData const* GetStaticType(); \
	virtual class TypeMetaData const* GetType();   \
	static void Serialize(IFileStream* data, ClassName* obj) { obj->Serialize(data); } \
	void Serialize(IFileStream* data);

#define CLASS(ClassName, BaseClass) \
	using Super = BaseClass;        \
	CLASS_BASE()

#define SERIALIZE_FN(TypeName) void TypeName::Serialize(IFileStream* fileStream)
#define SERIALIZE_PROPERTY(PropertyName) fileStream->ReadProperty<decltype(this->m_##PropertyName)>(#PropertyName, this->m_##PropertyName)
#define SERIALIZE_SUPER() Super::Serialize(fileStream)
