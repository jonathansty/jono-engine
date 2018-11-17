//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
//#define WINDOWS7

#ifdef WIN32
#include <windows.h>
#include <tchar.h>			// support for both unicode and MBCS

#undef MessageBox

#include "../Resource.h"	
#endif

// std lib includes
#include <vector>			// std lib vector
#include <string>			// std lib to_string
#include <sstream>			// std lib stringstream
#include <fstream>			// SVGParser
#include <iostream>
#include <iomanip>			// precision

// Windows specific includes
#if defined(WIN32)
#include <io.h>				// console

// WindowsSDK
#include <dwrite.h>			// Draw Text
#include <wincodec.h>		// WIC: image loading
#include <d2d1.h>
#include <d2d1helper.h>

// libs used for Direct2D
#pragma comment(lib, "d2d1.lib")			// Direct2D
#pragma comment(lib, "WindowsCodecs.lib")	// Image Loading
#pragma comment(lib, "dwrite.lib")			// Draw Text
using namespace D2D1;
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Box2D 
// copy the headers box2d map in a folder with somename:  projectdir/somepath/box2d
// additional include directories: do NOT add ./ to make this include <> work, use ./somepath instead -> bug in vs2013?
#include <Box2D/Box2D.h>
#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <GL/gl.h>

#if !defined(_BODY_TYPE)
#define _BODY_TYPE
//! BodyType enumeration
//! @enum Box2D enumerations
enum class BodyType
{
	STATIC, DYNAMIC, KINEMATIC
};
#endif

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

// Include Direct2DVK
#if !defined(WIN32)
#include "Compat/D2DPoint.h"

using BYTE = uint8_t;
using WORD = uint16_t;
using HINSTANCE = uint64_t;
using HWND = uint64_t;

#endif

#include "EngineFiles/String.h"
#include "EngineFiles/Structs.h"
#include "EngineFiles/Box2DDebugRenderer.h"
#include "EngineFiles/InputManager.h"
#include "EngineFiles/PrecisionTimer.h"
#include "EngineFiles/GameSettings.h"
#include "EngineFiles/GameEngine.h"
#include "EngineFiles/Font.h"
#include "EngineFiles/AudioSystem.h"
#include "EngineFiles/AudioDecoder.h"
#include "EngineFiles/Bitmap.h"
#include "EngineFiles/GUIBase.h"
#include "EngineFiles/TextBox.h"
#include "EngineFiles/Button.h"
#include "EngineFiles/SVGParser.h"
#include "EngineFiles/Sound.h"
#include "EngineFiles/PhysicsActor.h"
#include "EngineFiles/PhysicsRevoluteJoint.h"
#include "EngineFiles/PhysicsPrismaticJoint.h"
#include "EngineFiles/PhysicsDistanceJoint.h"



