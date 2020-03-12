#include "stdafx.h"
#include "Component.h"

using namespace framework;

IMPL_REFLECT(Component)
{
	type.register_property("Active", &Component::_active);
}

Component::~Component()
{

}

Component::Component()
	: _active(true)
{

}
