#pragma once

#include "core/Types.h"
#include "Core/Asserts.h"

#include <fmt/core.h>
#include <fmt/printf.h>

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


#include <imgui.h>
#include <TaskScheduler.h>
#include <tinyxml2/tinyxml2.h>

