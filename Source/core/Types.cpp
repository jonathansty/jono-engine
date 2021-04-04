#include "core.pch.h"
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

		registration::class_<WrapperQuat>("quaternion")
			.property("X", &WrapperQuat::get_x, &WrapperQuat::set_x)
			.property("Y", &WrapperQuat::get_y, &WrapperQuat::set_y)
			.property("Z", &WrapperQuat::get_z, &WrapperQuat::set_z)
			.property("W", &WrapperQuat::get_w, &WrapperQuat::set_w);


	registration::class_<WrapperFloat3>("float3")
		.property("X", &WrapperFloat3::get_x, &WrapperFloat3::set_x)
		.property("Y", &WrapperFloat3::get_y, &WrapperFloat3::set_y)
		.property("Z", &WrapperFloat3::get_z, &WrapperFloat3::set_z);
}


struct GlobalData {
	std::atomic<bool> initialised = false;
	std::mutex mutex;
	std::unordered_map<u64, rttr::type const*> mappings;
};
GlobalData g_data{};

GUID StringToGuid(const std::string& str) {
	GUID guid;
	sscanf_s(str.c_str(),
			"{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}",
			&guid.Data1, &guid.Data2, &guid.Data3,
			&guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3],
			&guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);

	return guid;
}

namespace helpers {

std::string GuidToString(GUID guid) {
	char guid_cstr[39];
	snprintf(guid_cstr, sizeof(guid_cstr),
			"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return std::string(guid_cstr);
}

void SetDebugObjectName(ID3D11DeviceChild* res, std::string const& name) {
	res->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
}

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

}
