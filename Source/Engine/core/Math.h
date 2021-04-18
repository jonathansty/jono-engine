#pragma once

#include <hlsl++.h>

namespace hlslpp_helpers {

	hlslpp::float3 to_euler(hlslpp::quaternion q);
}