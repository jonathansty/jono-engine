#pragma once
/*!
 * \file Types.h
 * \date 2021/03/14 19:24
 *
 * \author Jonathan Steyfkens
 *
 * \brief typedefs for commonly used types across the engine
 *
 * \note
*/

#define NOMINMAX
#define HLSLPP_FEATURE_TRANSFORM

#include <hlsl++.h>

#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>

#ifdef _WIN64
// WindowsSDK
#include <DirectXMath.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <dwrite.h> // Draw Text
#include <dxgi.h>
#include <wincodec.h> // WIC: image loading
using namespace DirectX;
#include <wrl.h>
using Microsoft::WRL::ComPtr;
using namespace D2D1;
#endif


#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>

#include <Identifier.h>

#ifdef WIN64
#include <Windows.h>
#endif

// Define M_PI and other constants
#define _USE_MATH_DEFINES
#include <math.h>


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

using hlslpp::double2;
using hlslpp::float1;
using hlslpp::float2;
using hlslpp::float3;
using hlslpp::float3x3;
using hlslpp::float4;
using hlslpp::float4x4;

// Default C++ types
using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::weak_ptr;
using std::make_shared;
using std::make_unique;
using std::make_optional;
using std::optional;

// Containers
using std::vector;
using std::array;
using std::unordered_map;
using std::unordered_set;


// LEGACY - Unicode defs for svg parsing
#ifdef _UNICODE
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tstring std::string
#define tstringstream std::stringstream
#endif

// Store Box2D contactdata retrieved from userData pointers
struct ContactData
{
	void *contactListenerPtr = nullptr;
	void *actThisPtr = nullptr, *actOtherPtr = nullptr;
};

struct ImpulseData
{
	void *contactListenerAPtr = nullptr, *contactListenerBPtr = nullptr;
	double impulseA = 0, impulseB = 0;
	void *actAPtr = nullptr, *actBPtr = nullptr;
};

namespace helpers {

// Wraps around hlslpp float4 to provide support for RTTR
struct WrapperFloat4 {
	hlslpp::float4 value;

	operator hlslpp::float4() {
		return value;
	}

private:
	RTTR_REGISTRATION_FRIEND;
	float get_x() { return value.x; }
	float get_y() { return value.y; }
	float get_z() { return value.z; }
	float get_w() { return value.w; }

	void set_x(float v) { value.x = v; }
	void set_y(float v) { value.y = v; }
	void set_z(float v) { value.z = v; }
	void set_w(float v) { value.w = v; }
};

struct WrapperQuat {
	hlslpp::quaternion value;
	operator hlslpp::quaternion() {
		return value;
	}

	RTTR_REGISTRATION_FRIEND;
	float get_x() { return value.x; }
	float get_y() { return value.y; }
	float get_z() { return value.z; }
	float get_w() { return value.w; }

	void set_x(float v) { value.x = v; }
	void set_y(float v) { value.y = v; }
	void set_z(float v) { value.z = v; }
	void set_w(float v) { value.w = v; }
};

// Wraps around hlslpp float3 to provide support for RTTR
struct WrapperFloat3 {
	hlslpp::float3 value;

	operator hlslpp::float3() {
		return value;
	}

	private:
	RTTR_REGISTRATION_FRIEND;

	public:
	float get_x() { return value.x; }
	float get_y() { return value.y; }
	float get_z() { return value.z; }

	void set_x(float v) { value.x = v; }
	void set_y(float v) { value.y = v; }
	void set_z(float v) { value.z = v; }


};

} // namespace helpers

using helpers::WrapperFloat3;
using helpers::WrapperFloat4;
using helpers::WrapperQuat;

namespace helpers {


rttr::type const& get_type_by_id(u64 id);

GUID StringToGuid(const std::string& str);
std::string GuidToString(GUID guid);

void SetDebugObjectName(ID3D11DeviceChild* res, std::string const& name);

}


#define MK_COLOR(r, g, b, a) ((u32)((r & 0xFF) << 24 | (g & 0xFF) << 16 | (b & 0xFF) << 8 | a))
#define COLOR_R(c) ( c & 0xFF000000)
#define COLOR_G(c) ( c & 0x00FF0000)
#define COLOR_B(c) ( c & 0x0000FF00)
#define COLOR_A(c) ( c & 0x000000FF)

