#include "engine.pch.h"
#include "TypeManager.h"

#include "GlobalContext.h"

TypeManager::TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = this;
}

TypeMetaData* TypeManager::FindType(const char* name)
{
	if (auto it = m_Types.find(name); it != m_Types.end())
	{
		return &it->second;
	}
	return nullptr;
}

TypeManager::~TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = nullptr;
}


REGISTER_TYPE("/Types/Test/Foo", Foo);
REGISTER_TYPE("/Types/Test/Bar", Bar);
