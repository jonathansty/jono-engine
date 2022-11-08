#include "engine.pch.h"
#include "TypeManager.h"

#include "GlobalContext.h"

TypeManager::TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = this;
}

TypeManager::~TypeManager()
{
	GlobalContext* globalContext = GetGlobalContext();
	globalContext->m_TypeManager = nullptr;
}
