//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------

#include "Resource.h"	

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
using namespace DirectX;


// libs used for Direct2D
using namespace D2D1;

#include <wrl.h>
using Microsoft::WRL::ComPtr;

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
#include <TaskScheduler.h>

#include "tinyxml2/tinyxml2.h"

#include "EString.h"
#include "Structs.h"
#include "Box2DDebugRenderer.h"
#include "InputManager.h"
#include "PrecisionTimer.h"
#include "GameSettings.h"
#include "GameEngine.h"
#include "Font.h"
#include "AudioSystem.h"
#include "AudioDecoder.h"
#include "Bitmap.h"
#include "GUIBase.h"
#include "TextBox.h"
#include "Button.h"
#include "SVGParser.h"
#include "sound.h"
#include "PhysicsActor.h"
#include "PhysicsRevoluteJoint.h"
#include "PhysicsPrismaticJoint.h"
#include "PhysicsDistanceJoint.h"

using std::shared_ptr;
using std::weak_ptr;

#include "rtti/rtti.h"


