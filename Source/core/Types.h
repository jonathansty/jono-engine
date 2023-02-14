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



using u8 = unsigned __int8;
using u16 = unsigned __int16;
using u32 = unsigned __int32;
using u64 = unsigned __int64;

using s8  = __int8;
using s16 = __int16;
using s32 = __int32;
using s64 = __int64;

using f32 = float;
using f64 = double;


using hlslpp::double2;
using hlslpp::float1;
using hlslpp::float2;
using hlslpp::float3;
using hlslpp::float3x3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::quaternion;
using hlslpp::int4;
using hlslpp::int3;
using hlslpp::int2;
using hlslpp::uint2;

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

namespace Helpers {

// Wraps around hlslpp float4 to provide support for RTTR
struct WrapperFloat4
{
	hlslpp::float4 value;

	operator hlslpp::float4()
	{
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

struct WrapperQuat
{
	hlslpp::quaternion value;
	operator hlslpp::quaternion()
	{
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
struct WrapperFloat3
{
	hlslpp::float3 value;

	operator hlslpp::float3()
	{
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

using Helpers::WrapperFloat3;
using Helpers::WrapperFloat4;
using Helpers::WrapperQuat;

namespace Helpers
{

#ifdef ENABLE_RTTR
rttr::type const& get_type_by_id(u64 id);
#endif

GUID StringToGuid(const std::string& str);
std::string GuidToString(GUID guid);

template <typename T>
void SafeRelease(T*& obj)
{
	if (obj != nullptr)
	{
		obj->Release();
		obj = nullptr;
	}
}

} // namespace helpers

#define MK_COLOR(r, g, b, a) ((u32)((r & 0xFF) << 24 | (g & 0xFF) << 16 | (b & 0xFF) << 8 | a))
#define COLOR_R(c) ( c & 0xFF000000)
#define COLOR_G(c) ( c & 0x00FF0000)
#define COLOR_B(c) ( c & 0x0000FF00)
#define COLOR_A(c) ( c & 0x000000FF)

#define ENSURE_HR(result)     \
	{                         \
		HRESULT __COUNTER__##_result = result;  \
		assert(S_OK == (__COUNTER__##_result)); \
	}

// Enum related implementation macros

// Implements basic enum bitflag operators and the underlying operator* overload
#define ENUM_BITFLAGS(enum_type)                                           \
	inline enum_type operator&(enum_type const& lhs, enum_type const& rhs) \
	{                                                                      \
		return static_cast<enum_type>((uint32_t)(lhs) & (uint32_t)(rhs));                \
	}                                                                      \
	inline enum_type operator|(enum_type const& lhs, enum_type const& rhs) \
	{                                                                      \
		return static_cast<enum_type>((uint32_t)lhs | (uint32_t)rhs);                \
	}                                                                      \
	ENUM_UNDERLYING_TYPE(enum_type)	

// Implement operator* to convert the enum to it's underlying type
#define ENUM_UNDERLYING_TYPE(enum_type) \
	constexpr auto operator*(enum_type e) noexcept { return static_cast<std::underlying_type<enum_type>::type>(e); }
