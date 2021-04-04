#pragma once

#define NOMINMAX

#include "Core/Types.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXTK/DirectXHelpers.h>
#include <DirectXTK/VertexTypes.h>
#include <DirectXTK/Effects.h>

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>


// libs used for Direct2D
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")			// Direct2D
#pragma comment(lib, "WindowsCodecs.lib")	// Image Loading
#pragma comment(lib, "dwrite.lib")			// Draw Text
#pragma comment(lib, "d3d11.lib")
using namespace D2D1;

// Box2D 
// copy the headers box2d map in a folder with somename:  projectdir/somepath/box2d
// additional include directories: do NOT add ./ to make this include <> work, use ./somepath instead -> bug in vs2013?
#include <box2d/box2d.h>

// Define M_PI and other constants
#define _USE_MATH_DEFINES 
#include "math.h"

// Unicode defs for svg parsing
#ifdef _UNICODE
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tstring std::string
#define tstringstream std::stringstream
#endif

#include <imgui.h>

#include "tinyxml2/tinyxml2.h"

#include "Core/identifier.h"

#include "Engine/Bitmap.h"
#include "Engine/Framework/framework.h"
#include "Engine/GameEngine.h"



