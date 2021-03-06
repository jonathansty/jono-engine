#include "engine.stdafx.h"
#include "Component.h"

using namespace framework;

RTTR_REGISTRATION {

	using namespace rttr;
	registration::class_<Component>("Component")
		.constructor()
		.property("Active", &Component::_active);
}

Component::~Component()
{

}

Component::Component()
	: _active(true)
{

}
