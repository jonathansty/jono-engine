#pragma once

#ifdef WIN64
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>

#include <hlsl++.h>

#include <fmt/core.h>
