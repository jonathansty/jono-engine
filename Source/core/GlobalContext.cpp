#include "core.pch.h"
#include "GlobalContext.h"

static GlobalContext g_GlobalContext{};
GlobalContext* GetGlobalContext()
{
	return &g_GlobalContext;
}
