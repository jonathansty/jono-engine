#pragma once

#include "Core/Singleton.h"

class ENGINE_API TypeManager final : public TSingleton<TypeManager>
{
public:
	TypeManager();
	~TypeManager();

	TypeManager(TypeManager const&) = delete;
	TypeManager const& operator=(TypeManager const&) = delete;

};