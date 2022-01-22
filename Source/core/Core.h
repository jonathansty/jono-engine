#pragma once

#ifdef WIN64
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <commdlg.h>
#endif

#include <array>
#include <mutex>
#include <vector>

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>


// Feature Defines used to control compile time behaviour
#define FEATURE_D2D    true    // Flag to control if D2D can be used or not
#define FEATURE_XAUDIO true    // Flag to control if XAudio can be used or not

#define NOMINMAX
#define HLSLPP_FEATURE_TRANSFORM
#include <hlsl++.h>

#include <stdint.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>

#ifdef _WIN64
// WindowsSDK
#include <DirectXMath.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <dwrite.h> // Draw Text
#include <dxgi.h>
#include <dxgidebug.h>
#include <wincodec.h> // WIC: image loading
using namespace DirectX;
#include <wrl.h>
using Microsoft::WRL::ComPtr;
using namespace D2D1;

// libs used for Direct2D
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib") // Direct2D
#pragma comment(lib, "WindowsCodecs.lib") // Image Loading
#pragma comment(lib, "dwrite.lib") // Draw Text
#pragma comment(lib, "d3d11.lib")
#endif

#include <Identifier.h>

// Define M_PI and other constants
#define _USE_MATH_DEFINES
#include <math.h>

#include <optional>
#include "Types.h"
#include "Asserts.h"
#include "identifier.h"


#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>

//#pragma GCC diagnostic ignored "-Wpragma-pack"
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <box2d/b2_world_callbacks.h>
#include <box2d/box2d.h>

#include "enkiTS/TaskScheduler.h"

#include <filesystem>
#include <cassert>

// Unicode defs for svg parsing
#ifdef _UNICODE
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tstring std::string
#define tstringstream std::stringstream
#endif


namespace Tasks {
enki::TaskScheduler* get_scheduler();

}

