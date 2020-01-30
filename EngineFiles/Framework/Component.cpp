#include "stdafx.h"
#include "Component.h"

using namespace framework;

IMPL_REFLECT(Component)
{

}

Component::Component(std::string const& name) : _name(name)
{

}

Component::~Component()
{

}
