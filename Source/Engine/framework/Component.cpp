#include "engine.pch.h"
#include "Component.h"

#ifdef ENABLE_RTTR
#include <rttr/policy.h>

using namespace framework;

RTTR_REGISTRATION 
{
	using namespace rttr;
	registration::class_<Component>("Component")
		.constructor<>()(
				rttr::policy::ctor::as_raw_ptr
		)
		.property("Active", &Component::_active);
}
#endif

namespace framework
{
Component::~Component()
{
}

Component::Component()
		: _active(true)
		, _parent(nullptr)
{
}
}

