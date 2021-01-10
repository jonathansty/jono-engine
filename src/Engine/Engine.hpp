#pragma once

// Monolithic header to include the game engine 
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

#define NOMINMAX
#include <Windows.h>

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
using namespace D2D1;

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <Box2D/Box2D.h>

#include <imgui.h>

#include "EString.h"
#include "Structs.h"

#include "rtti/rtti.h"

#include "Box2DDebugRenderer.h"
#include "DebugOverlays/OverlayManager.h"
#include "GameSettings.h"
#include "GameEngine.h"

#include "AbstractGame.h"

#include "Core/logging.h"
#include "Framework/framework.h"
#include "PhysicsActor.h"