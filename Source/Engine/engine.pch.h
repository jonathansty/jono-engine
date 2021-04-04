#pragma once

#include "core/Types.h"
#include "Core/Asserts.h"

#include <fmt/core.h>
#include <fmt/printf.h>


#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>

//#pragma GCC diagnostic ignored "-Wpragma-pack"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include <box2d/box2d.h>
#include <box2d/b2_world_callbacks.h>

#include <imgui.h>
#include <enkiTS/TaskScheduler.h>
#include <tinyxml2/tinyxml2.h>

