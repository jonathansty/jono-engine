#pragma once

#include "DirectXColors.h"
#include <DirectXTK/VertexTypes.h>
#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/PrimitiveBatch.h>

namespace Graphics
{

using FrustumCorners = std::array<float4, 8>;

namespace Debug
{

void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingSphere& sphere,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingBox& box,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingOrientedBox& obb,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingFrustum& frustum,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void DrawGrid(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		float4 xAxis, float4 yAxis,
		float4 origin, size_t xdivs, size_t ydivs,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void DrawRing(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		float4 const& origin, float4 const& majorAxis, float4 const& minorAxis,
		float4 const& color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void DrawRay(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		float4 origin, float4 direction, bool normalize = true,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void DrawTriangle(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		DirectX::XMVECTOR pointA, DirectX::XMVECTOR pointB, DirectX::XMVECTOR pointC,
		DirectX::GXMVECTOR color = DirectX::Colors::White);

void DrawQuad(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		float4 pointA, float4 pointB,
		float4 pointC, float4 pointD,
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

void DrawFrustum(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, FrustumCorners const& corners, float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f));

} // namespace Debug

}
