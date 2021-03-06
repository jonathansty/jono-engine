#pragma once

#include "resource.h"	

// std lib includes
#include <vector>			// std lib vector
#include <array>
#include <unordered_map>
#include <map>
#include <string>			// std lib to_string
#include <sstream>			// std lib stringstream
#include <fstream>			// SVGParser
#include <iostream>
#include <iomanip>			// precision
#include <io.h>				// console
#include <deque>
#include <thread>
#include <algorithm>

#include <fmt/core.h>
#include <fmt/printf.h>

#define NOMINMAX
// WindowsSDK
#include <dwrite.h>			// Draw Text
#include <wincodec.h>		// WIC: image loading
#include <d2d1.h>
#include <d2d1helper.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXTK/VertexTypes.h>
#include <DirectXTK/DirectXHelpers.h>
using namespace DirectX;

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>

//#pragma GCC diagnostic ignored "-Wpragma-pack"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

// libs used for Direct2D
using namespace D2D1;

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <box2d/box2d.h>
#include <box2d/b2_world_callbacks.h>

// Define M_PI and other constants
#define _USE_MATH_DEFINES 
#include <math.h>

// Unicode defs for svg parsing
#ifdef _UNICODE
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tstring std::string
#define tstringstream std::stringstream
#endif

#include <imgui.h>
#include <TaskScheduler.h>
#include <tinyxml2/tinyxml2.h>

// HLSLPP
#include <hlsl++.h>
using hlslpp::double2;
using hlslpp::float1;
using hlslpp::float2;
using hlslpp::float3;
using hlslpp::float3x3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::double2;

// Default C++ types
using std::string;
using std::shared_ptr;
using std::weak_ptr;



// This is the only engine header that should be in the precompiled header to allow standard type usages across the project 
#include "core/types.h"
