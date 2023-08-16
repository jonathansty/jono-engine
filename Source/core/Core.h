#pragma once

#ifdef WIN64
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <commdlg.h>
#endif // _WIN64

#ifdef ENABLE_RTTR
#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>
#else
#define RTTR_REGISTRATION_FRIEND
#endif

// Feature Defines used to control compile time behaviour
//#define FEATURE_D2D    true    // Flag to control if D2D can be used or not
//#define FEATURE_XAUDIO true    // Flag to control if XAudio can be used or not

#define HLSLPP_FEATURE_TRANSFORM
#include <hlsl++.h>

#include <stdint.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <array>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

#include <Identifier.h>

// Define M_PI and other constants
#define _USE_MATH_DEFINES
#include <math.h>

#include <optional>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <fmt/xchar.h>

#include <TaskScheduler.h>

#include <cassert>
#include <filesystem>

// Unicode defs for svg parsing
#ifdef _UNICODE
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tstring std::string
#define tstringstream std::stringstream
#endif


// Optick integration
#define JONO_FRAME(_arg0) OPTICK_FRAME(_arg0)
#define JONO_EVENT(...) OPTICK_EVENT(__VA_ARGS__)
#define JONO_THREAD(_arg0) OPTICK_THREAD(_arg0)
#define JONO_CATEGORY(_arg0, _category) OPTICK_CATEGORY(_arg0, _category)
#define JONO_TAG(_tag, _tagValue) OPTICK_TAG(_tag, _tagValue)
#define JONO_SET_STATE_CHANGED_CALLBACK(callback_fn) OPTICK_SET_STATE_CHANGED_CALLBACK(callback_fn)
#define JONO_SET_MEMORY_ALLOCATOR(alloc_fn, dealloc_fn) OPTICK_SET_MEMORY_ALLOCATOR(alloc_fn, dealloc_fn);
#define JONO_SHUTDOWN() OPTICK_SHUTDOWN()

#define JONO_GPU_CONTEXT(ctx) OPTICK_GPU_CONTEXT(ctx)
#define JONO_GPU_EVENT(name) OPTICK_GPU_EVENT(name)
#define JONO_GPU_FLIP(swapchain) OPTICK_GPU_FLIP(swapchain)
#define JONO_INIT_GPU(device, cmd_queue, n_cmd_queues) OPTICK_GPU_INIT_D3D12(device, cmd_queue, n_cmd_queues)

#define JONO_NEW(TypeName, ...) new TypeName(__VA_ARGS__)

#define JONO_INLINE inline

#ifdef CORE_DLL
#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif
#else
#define CORE_API
#endif

namespace Tasks
{

CORE_API enki::TaskScheduler* get_scheduler();

}

#include "Asserts.h"
#include "Hash.h"
#include "Math.h"
#include "Types.h"
#include "Identifier.h"
#include "Thread.h"
#include <chrono>
#include <semaphore>

