#pragma once

#define NOMINMAX
#include <Windows.h>

#include <vector>
#include <unordered_map>

#include <DirectXMath.h>
#include <dxgi.h>
#include <d3d11.h>

#include <rttr/type>
#include <rttr/registration>
#include <rttr/registration_friend>

#include <hlsl++.h>
using hlslpp::float4;
using hlslpp::float3;
using hlslpp::float2;
using hlslpp::float4x4;
using hlslpp::float3x3;

using namespace DirectX;


#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include "Core/logging.h"
#include "Core/PlatformIO.h"
#include "Core/Serialization.h"

#include "Framework/Entity.h"
#include "Framework/World.h"
