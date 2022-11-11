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

TypeManager::~TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = nullptr;
}


