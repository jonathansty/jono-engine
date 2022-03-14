#pragma once

#include "Core/Core.h"

//#pragma GCC diagnostic ignored "-Wpragma-pack"
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <box2d/b2_world_callbacks.h>
#include <box2d/box2d.h>

#include <enkiTS/TaskScheduler.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <implot.h>

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include <tinyxml2/tinyxml2.h>
