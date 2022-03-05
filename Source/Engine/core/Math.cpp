#include "Math.h"
#include "engine.pch.h"

namespace hlslpp_helpers
{

float3 to_euler(hlslpp::quaternion q)
{
	// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	float3 angles;

	// roll (x-axis rotation)
	float1 sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float1 cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = hlslpp::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float1 sinp = 2 * (q.w * q.y - q.z * q.x);
	if (hlslpp::all(hlslpp::abs(sinp) >= float1(1)))
		angles.y = std::copysign(3.1415f / 2, float(sinp)); // use 90 degrees if out of range
	else
		angles.y = hlslpp::asin(sinp);

	// yaw (z-axis rotation)
	float1 siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float1 cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = hlslpp::atan2(siny_cosp, cosy_cosp);

	return angles;
}

} // namespace hlslpp_helpers
