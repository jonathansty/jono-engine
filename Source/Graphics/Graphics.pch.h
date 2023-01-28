#pragma once

#include "Core/Core.h"
#include "Core/Memory.h"
#include "CLI/CLI.h"
#include "CLI/CommandLine.h"

#define USE_DX11

#ifdef USE_DX11
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#include <d3d11_1.h>
#include <wincodec.h>
#endif
