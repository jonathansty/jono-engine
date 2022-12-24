#include "engine.pch.h"

#include "CLI/CommandLine.h"

#include "TypeManager.h"
#include "GlobalContext.h"

TypeManager::TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = this;
}

TypeMetaData* TypeManager::FindType(std::string_view const& name)
{
	// Force a copy here
	std::string n = std::string(name);
	if (auto it = m_Types.find(n); it != m_Types.end())
	{
		return &it->second;
	}
	return nullptr;
}

void* TypeManager::SerializeObject(std::string_view const& typePath, IFileStream* data)
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

void* TypeManager::SerializeObject(std::string_view const& typePath, void* dst, IFileStream* data)
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


TypeManager::~TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = nullptr;
}


