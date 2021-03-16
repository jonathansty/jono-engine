#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>
#include <hlsl++.h>
using hlslpp::float3;
using hlslpp::float4;

#include "Types.h"

RTTR_REGISTRATION{
	using namespace rttr;
	using namespace helpers;

	registration::class_<WrapperFloat4>("float4")
		.property("X", &WrapperFloat4::get_x, &WrapperFloat4::set_x)
		.property("Y", &WrapperFloat4::get_y, &WrapperFloat4::set_y)
		.property("Z", &WrapperFloat4::get_z, &WrapperFloat4::set_z)
		.property("W", &WrapperFloat4::get_w, &WrapperFloat4::set_w);

	registration::class_<WrapperFloat3>("float3")
		.property("X", &WrapperFloat3::get_x, &WrapperFloat3::set_x)
		.property("Y", &WrapperFloat3::get_y, &WrapperFloat3::set_y)
		.property("Z", &WrapperFloat3::get_z, &WrapperFloat3::set_z);
}

//-----------------------------------------------------------------
// COLOR Constructors
//-----------------------------------------------------------------
COLOR::COLOR() : red(255), green(255), blue(255), alpha(255)
{}
COLOR::COLOR(unsigned char redVal, unsigned char greenVal, unsigned char blueVal, unsigned char alphaVal) : red(redVal), green(greenVal), blue(blueVal), alpha(alphaVal)
{}
//-----------------------------------------------------------------
// RECT2 Constructors
//-----------------------------------------------------------------
RECT2::RECT2() : left(0), top(0), right(0), bottom(0)
{}

RECT2::RECT2(double leftVal, double topVal, double rightVal, double bottomVal) : left(leftVal), top(topVal), right(rightVal), bottom(bottomVal)
{}


struct GlobalData {
	std::atomic<bool> initialised = false;
	std::mutex mutex;
	std::unordered_map<u64, rttr::type const*> mappings;
};
GlobalData g_data{};

rttr::type const& helpers::get_type_by_id(u64 id) {
	if (!g_data.initialised) {
		std::lock_guard g{ g_data.mutex };
		std::for_each(rttr::type::get_types().begin(), rttr::type::get_types().end(), [&](rttr::type const& type) {
			Identifier64 h = Identifier64(type.get_name().begin());
			g_data.mappings[h.get_hash()] = &type;
		});
		g_data.initialised = true;
	}

	return *g_data.mappings[id];
}
