#pragma once
#include <hlsl++.h>
#include <DirectXMath.h>


namespace hlslpp_helpers {

	using DirectX::XMMATRIX;
	using DirectX::XMFLOAT4X4;

	using hlslpp::float4x4;

	XMMATRIX XMLoadFloat4x4(float4x4 matrix) {
		XMFLOAT4X4 i{};
		hlslpp::store(matrix, reinterpret_cast<float*>(&i));

		XMMATRIX result = DirectX::XMLoadFloat4x4(&i);
		return result;
	}
	
	void XMStoreFloat4x4(XMMATRIX src, float4x4& result) {

		XMFLOAT4X4 data{};
		DirectX::XMStoreFloat4x4(&data, src);
		hlslpp::load(result, reinterpret_cast<float *>(&data));
	}

}