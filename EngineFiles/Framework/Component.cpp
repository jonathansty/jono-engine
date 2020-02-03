#include "stdafx.h"
#include "Component.h"

using namespace framework;

IMPL_REFLECT(Component)
{

	type.register_property("name", &Component::_name);

}

Component::Component(std::string const& name) : _name(name)
{

}

Component::~Component()
{

}
