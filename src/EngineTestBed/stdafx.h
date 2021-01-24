#pragma once

#define NOMINMAX

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

// WindowsSDK
#include <dwrite.h>			// Draw Text
#include <wincodec.h>		// WIC: image loading
#include <d2d1.h>
#include <d2d1helper.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <d3d11shader.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <directxtk/DirectXHelpers.h>
#include <directxtk/VertexTypes.h>
#include <directxtk/Effects.h>


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
#include <Box2D/Box2D.h>

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

#include <hlsl++.h>
using hlslpp::double2;
using hlslpp::float2;
using hlslpp::float3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::float3x3;
using hlslpp::double2;


#include "Engine/EString.h"
#include "Engine/Structs.h"
#include "Engine/Box2DDebugRenderer.h"
#include "Engine/InputManager.h"
#include "Engine/PrecisionTimer.h"
#include "Engine/GameSettings.h"
#include "Engine/GameEngine.h"
#include "Engine/Font.h"
#include "Engine/AudioSystem.h"
#include "Engine/AudioDecoder.h"
#include "Engine/Bitmap.h"
#include "Engine/GUIBase.h"
#include "Engine/TextBox.h"
#include "Engine/Button.h"
#include "Engine/SVGParser.h"
#include "Engine/sound.h"
#include "Engine/PhysicsActor.h"
#include "Engine/PhysicsRevoluteJoint.h"
#include "Engine/PhysicsPrismaticJoint.h"
#include "Engine/PhysicsDistanceJoint.h"
#include "GameEngine.h"

#include "Game/BitmapManager.h"
#include "Game/SoundManager.h"


using namespace DirectX;
