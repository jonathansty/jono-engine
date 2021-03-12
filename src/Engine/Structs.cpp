#include "engine.stdafx.h"

#include "Structs.h"

RTTR_REGISTRATION{
	using namespace rttr;
	using namespace helpers;

	registration::class_<WrapperFloat4>("float4")
		.property("X", &WrapperFloat4::get_x, &WrapperFloat4::set_x)
		.property("Y", &WrapperFloat4::get_y, &WrapperFloat4::set_y)
		.property("Z", &WrapperFloat4::get_z, &WrapperFloat4::set_z)
		.property("W", &WrapperFloat4::get_w, &WrapperFloat4::set_w);

	registration::class_<WrapperFloat4>("float3")
		.property("X", &WrapperFloat4::get_x, &WrapperFloat4::set_x)
		.property("Y", &WrapperFloat4::get_y, &WrapperFloat4::set_y)
		.property("Z", &WrapperFloat4::get_z, &WrapperFloat4::set_z);
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

