#include "engine.pch.h"
#include "Debug.h"
namespace Graphics
{

namespace Debug
{
using namespace DirectX;

inline void DrawCube(PrimitiveBatch<VertexPositionColor>* batch,
		CXMMATRIX matWorld,
		FXMVECTOR color)
{
	static const XMVECTORF32 s_verts[8] = {
		{ { { -1.f, -1.f, -1.f, 0.f } } },
		{ { { 1.f, -1.f, -1.f, 0.f } } },
		{ { { 1.f, -1.f, 1.f, 0.f } } },
		{ { { -1.f, -1.f, 1.f, 0.f } } },
		{ { { -1.f, 1.f, -1.f, 0.f } } },
		{ { { 1.f, 1.f, -1.f, 0.f } } },
		{ { { 1.f, 1.f, 1.f, 0.f } } },
		{ { { -1.f, 1.f, 1.f, 0.f } } }
	};

	static const WORD s_indices[] = {
		0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	VertexPositionColor verts[8];
	for (size_t i = 0; i < 8; ++i)
	{
		XMVECTOR v = XMVector3Transform(s_verts[i], matWorld);
		XMStoreFloat3(&verts[i].position, v);
		XMStoreFloat4(&verts[i].color, color);
	}

	batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, static_cast<UINT>(std::size(s_indices)), verts, 8);
}

void Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingSphere& sphere,
		FXMVECTOR color)
{
	XMVECTOR origin = XMLoadFloat3(&sphere.Center);

	const float radius = sphere.Radius;

	XMVECTOR xaxis = g_XMIdentityR0 * radius;
	XMVECTOR yaxis = g_XMIdentityR1 * radius;
	XMVECTOR zaxis = g_XMIdentityR2 * radius;

	DrawRing(batch, origin, xaxis, zaxis, color);
	DrawRing(batch, origin, xaxis, yaxis, color);
	DrawRing(batch, origin, yaxis, zaxis, color);
}

void Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingBox& box,
		FXMVECTOR color)
{
	XMMATRIX matWorld = XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
	XMVECTOR position = XMLoadFloat3(&box.Center);
	matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

	DrawCube(batch, matWorld, color);
}

void  Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingOrientedBox& obb,
		FXMVECTOR color)
{
	XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
	XMMATRIX matScale = XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
	matWorld = XMMatrixMultiply(matScale, matWorld);
	XMVECTOR position = XMLoadFloat3(&obb.Center);
	matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

	DrawCube(batch, matWorld, color);
}

void Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingFrustum& frustum,
		FXMVECTOR color)
{
	XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
	frustum.GetCorners(corners);

	VertexPositionColor verts[24] = {};
	verts[0].position = corners[0];
	verts[1].position = corners[1];
	verts[2].position = corners[1];
	verts[3].position = corners[2];
	verts[4].position = corners[2];
	verts[5].position = corners[3];
	verts[6].position = corners[3];
	verts[7].position = corners[0];

	verts[8].position = corners[0];
	verts[9].position = corners[4];
	verts[10].position = corners[1];
	verts[11].position = corners[5];
	verts[12].position = corners[2];
	verts[13].position = corners[6];
	verts[14].position = corners[3];
	verts[15].position = corners[7];

	verts[16].position = corners[4];
	verts[17].position = corners[5];
	verts[18].position = corners[5];
	verts[19].position = corners[6];
	verts[20].position = corners[6];
	verts[21].position = corners[7];
	verts[22].position = corners[7];
	verts[23].position = corners[4];

	for (size_t j = 0; j < std::size(verts); ++j)
	{
		XMStoreFloat4(&verts[j].color, color);
	}

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, static_cast<UINT>(std::size(verts)));
}

void DrawGrid(PrimitiveBatch<VertexPositionColor>* batch,
		float4 xAxis,
		float4 yAxis,
		float4 origin,
		size_t xdivs,
		size_t ydivs,
		float4 color)
{
	xdivs = std::max<size_t>(1, xdivs);
	ydivs = std::max<size_t>(1, ydivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float percent = float(i) / float(xdivs);
		percent = (percent * 2.f) - 1.f;
		float4 scale = xAxis * percent;
		scale += origin;

		XMFLOAT4 col;
		XMFLOAT3 pos0, pos1;
		hlslpp::store(color, (float*)&col);
		hlslpp::store((scale - yAxis), (float*)&pos0);
		hlslpp::store((scale + yAxis), (float*)&pos1);

		VertexPositionColor v1(pos0, col);
		VertexPositionColor v2(pos1, col);
		batch->DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; i++)
	{
		FLOAT percent = float(i) / float(ydivs);
		percent = (percent * 2.f) - 1.f;
		float4 scale = yAxis * percent;
		scale += origin;

		XMFLOAT4 col;
		XMFLOAT3 pos0, pos1;
		hlslpp::store(color, (float*)&col);
		hlslpp::store((scale - xAxis), (float*)&pos0);
		hlslpp::store((scale + xAxis), (float*)&pos1);

		VertexPositionColor v1(pos0, col);
		VertexPositionColor v2(pos1, col);
		batch->DrawLine(v1, v2);
	}
}

void DrawRing(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR origin,
		FXMVECTOR majorAxis,
		FXMVECTOR minorAxis,
		GXMVECTOR color)
{
	static const size_t c_ringSegments = 32;

	VertexPositionColor verts[c_ringSegments + 1];

	FLOAT fAngleDelta = XM_2PI / float(c_ringSegments);
	// Instead of calling cos/sin for each segment we calculate
	// the sign of the angle delta and then incrementally calculate sin
	// and cosine from then on.
	XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
	XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
	XMVECTOR incrementalSin = XMVectorZero();
	static const XMVECTORF32 s_initialCos = {
		1.f, 1.f, 1.f, 1.f
	};
	XMVECTOR incrementalCos = s_initialCos.v;
	for (size_t i = 0; i < c_ringSegments; i++)
	{
		XMVECTOR pos = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
		pos = XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
		XMStoreFloat3(&verts[i].position, pos);
		XMStoreFloat4(&verts[i].color, color);
		// Standard formula to rotate a vector.
		XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
		XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
		incrementalCos = newCos;
		incrementalSin = newSin;
	}
	verts[c_ringSegments] = verts[0];

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
}

void DrawRay(PrimitiveBatch<VertexPositionColor>* batch,
		float4 origin,
		float4 direction,
		bool normalize,
		float4 color)
{
	VertexPositionColor verts[3];
	hlslpp::store(origin, (float*)&verts[0].position);

	float3 normDirection = hlslpp::normalize(direction.xyz);
	float3 rayDirection = (normalize) ? normDirection : direction.xyz;

	float3 perpVector = hlslpp::cross(normDirection.xyz, float3(0.0f,1.0f,0.0f));

	if (hlslpp::length(perpVector) == float1(0.0f))
	{
		perpVector = hlslpp::cross(normDirection.xyz, float3(0.0f,0.0f,1.0f));
	}
	perpVector = hlslpp::normalize(perpVector);

	hlslpp::store(rayDirection + origin.xyz, (float*)&verts[1].position);
	perpVector = perpVector * 0.0625f;
	normDirection = normDirection * -0.25f;
	rayDirection = perpVector + rayDirection;
	rayDirection = normDirection+ rayDirection;
	hlslpp::store(rayDirection + origin.xyz, (float*)&verts[2].position);

	hlslpp::store(color,(float*)&verts[0].color);
	hlslpp::store(color,(float*)&verts[1].color);
	hlslpp::store(color,(float*)&verts[2].color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
}

void DrawTriangle(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR pointA,
		FXMVECTOR pointB,
		FXMVECTOR pointC,
		GXMVECTOR color)
{
	VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, pointA);
	XMStoreFloat3(&verts[1].position, pointB);
	XMStoreFloat3(&verts[2].position, pointC);
	XMStoreFloat3(&verts[3].position, pointA);

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);
	XMStoreFloat4(&verts[3].color, color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
}

void DrawQuad(PrimitiveBatch<VertexPositionColor>* batch,
		float4 pointA,
		float4 pointB,
		float4 pointC,
		float4 pointD,
		float4 color)
{
	VertexPositionColor verts[5];
	hlslpp::store(pointA, (float*)&verts[0].position);
	hlslpp::store(pointB, (float*)&verts[1].position);
	hlslpp::store(pointC, (float*)&verts[2].position);
	hlslpp::store(pointD, (float*)&verts[3].position);
	hlslpp::store(pointA, (float*)&verts[4].position);

	hlslpp::store(color, (float*)&verts[0].color);
	hlslpp::store(color, (float*)&verts[1].color);
	hlslpp::store(color, (float*)&verts[2].color);
	hlslpp::store(color, (float*)&verts[3].color);
	hlslpp::store(color, (float*)&verts[4].color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 5);
}

void DrawFrustum(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, FrustumCorners const& frustum, float4 color)
{
	// Near face
	Debug::DrawQuad(batch, frustum[0], frustum[1], frustum[3], frustum[2], color);

	// Left
	Debug::DrawQuad(batch, frustum[1], frustum[5], frustum[7], frustum[3], color);

	// Right
	Debug::DrawQuad(batch, frustum[0], frustum[4], frustum[6], frustum[2], color);

	// Top
	Debug::DrawQuad(batch, frustum[0], frustum[4], frustum[5], frustum[1], color);

	// Bottom
	Debug::DrawQuad(batch, frustum[2], frustum[3], frustum[7], frustum[6], color);

	// Far face
	Debug::DrawQuad(batch, frustum[4], frustum[5], frustum[7], frustum[6], color);
}

} // namespace Debug
} // namespace Graphics
