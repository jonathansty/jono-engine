#pragma once

#include "Core/Singleton.h"

class TypeMetaData
{
public:
	void* (*m_Construct)();
	void (*m_Destruct)(void*);

	const char* m_Name;
	const char* m_Path;
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

	TypeMetaData* FindType(const char* name);

	void* CreateObject(const char* name) 
	{
		if(TypeMetaData* obj = FindType(name); obj)
		{
			ASSERT(obj->m_Construct);
			return obj->m_Construct();
		}
		return nullptr;
	}

	template<typename T>
	T* CreateObject()
	{
		TypeMetaData const* metaData = T::GetStaticType();
		ASSERT(metaData->m_Construct);
		return static_cast<T*>(metaData->m_Construct());
	
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
struct StaticType 
{
public:
	StaticType(const char* typePath, const char* typeName);

	~StaticType();

	TypeMetaData* m_Data;
	const char* m_Path;
};

template<typename T>
StaticType<T>::StaticType(const char* typePath, const char* typeName)
		: m_Path(typePath)
{
	m_Data = TypeManager::instance()->AddType(m_Path);

	m_Data->m_Name = typeName;
	m_Data->m_Path = typePath;
	m_Data->m_Construct = []()->void* { return new T(); };
}

template<typename T>
StaticType<T>::~StaticType()
{
	TypeManager::instance()->RemoveType(m_Path);
}


#define ANON_VARIABLE(VarName) _#VarName##__LINE__

#define REGISTER_TYPE(Path, TypeName) \
	StaticType<TypeName> _typeRegister_##TypeName = StaticType<TypeName>(Path, #TypeName); \
	TypeMetaData const* TypeName::GetStaticType() { return _typeRegister_##TypeName.m_Data; } \
	TypeMetaData const* TypeName::GetType() { return TypeName::GetStaticType(); } 

#define CLASS_BASE() \
	static class TypeMetaData const* GetStaticType(); \
	virtual class TypeMetaData const* GetType();  

#define CLASS(ClassName, BaseClass) \
	using Super = BaseClass;        \
	static class TypeMetaData const* GetStaticType(); \
	virtual class TypeMetaData const* GetType(); 


struct Foo
{
	CLASS_BASE();

	Foo(){};
};

struct Bar : public Foo
{
	CLASS(Bar, Foo);

	Bar() {}
};
