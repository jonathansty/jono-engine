#pragma once

namespace hlslpp_helpers {

	hlslpp::float3 to_euler(hlslpp::quaternion q);

	DirectX::XMMATRIX XMLoadFloat4x4(hlslpp::float4x4 matrix);
	
	void XMStoreFloat4x4(DirectX::XMMATRIX src, hlslpp::float4x4& result);

}