#pragma once

#include "Core/Core.h"

#ifdef WIN64
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
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <wincodec.h> // WIC: image loading

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

#include <optick.h>

// XTK helpers
#include <Effects.h>
#include <Model.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>

//#pragma GCC diagnostic ignored "-Wpragma-pack"
#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <box2d/b2_world_callbacks.h>
#include <box2d/box2d.h>

#include <enkiTS/TaskScheduler.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include <tinyxml2/tinyxml2.h>
#include <stb_image.h>
