#include "engine.pch.h"

#if FEATURE_D2D
#include "2DRenderContext.h"
#include "Renderer.h"

#include "Bitmap.h"
#include "Core/TextureResource.h"
#include "Font.h"
#include "GameEngine.h"
#include "ShaderCache.h"

using hlslpp::float3x3;

namespace Graphics
{

D2DRenderContext::D2DRenderContext(Renderer* renderer, ID2D1Factory* factory, ID2D1RenderTarget* rt, ID2D1SolidColorBrush* brush, Font* font)
	: m_Renderer(renderer)
	, m_RenderTarget(rt)
	, m_Brush(brush)
	, m_Factory(factory)
	, m_InterpolationMode(D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR)
	, m_DefaultFont(font)
	, m_Font(font)
	, m_ViewMatrix(float3x3::identity())
	, m_WorldMatrix(float3x3::identity())
	, m_TotalVertices(0)
	, m_TotalIndices(0)
	, m_CurrentIndices(0)
	, m_CurrentVertices(0)
	, m_Colour(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_GlobalCB = ConstantBuffer::create(Graphics::get_device().Get(), sizeof(Shaders::float4x4) + sizeof(Shaders::float4), true, BufferUsage::Dynamic);
}

D2DRenderContext::D2DRenderContext()
	: m_InterpolationMode(D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR)
	, m_ViewMatrix(float3x3::identity())
	, m_WorldMatrix(float3x3::identity())
	, m_TotalVertices(0)
	, m_TotalIndices(0)
	, m_CurrentIndices(0)
	, m_CurrentVertices(0)
	, m_Colour(1.0f, 1.0f,1.0f,1.0f)
	, m_Renderer(nullptr)
	, m_RenderTarget(nullptr)
	, m_Brush(nullptr)
	, m_Font(nullptr)
	, m_DefaultFont(nullptr)
	, m_GlobalCB(nullptr)
{

}

bool D2DRenderContext::begin_paint(Renderer* renderer, ID2D1Factory* factory, ID2D1RenderTarget* rt, ID2D1SolidColorBrush* brush, Font* font)
{
	//m_RenderTarget = rt;
	//m_Factory = factory;
	//m_Brush = brush;
	//m_Font = font;
	m_Renderer = renderer;

	CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(m_Renderer->get_raw_output_tex(), m_Renderer->get_raw_output_rtv());
	D3D11_RECT rect{ (LONG)viewport.TopLeftX, (LONG)viewport.TopLeftY, (LONG)viewport.TopLeftX + (LONG)viewport.Width, (LONG)viewport.TopLeftY + (LONG)viewport.Height };
	m_ProjectionMatrix = float4x4::orthographic(hlslpp::projection(hlslpp::frustum(viewport.TopLeftX, viewport.TopLeftX + viewport.Width, viewport.TopLeftY + viewport.Height, viewport.TopLeftY, 0.01f, 1.0f), hlslpp::zclip::zero));

	m_TotalVertices = 0;
	m_TotalIndices = 0;
	m_DrawCommands.clear();
	m_DrawCommands.reserve(1024);

	if (m_RenderTarget)
	{
		m_RenderTarget->BeginDraw();
		m_RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	}
	if(m_Brush)
	{
		// set black as initial brush color
		m_Brush->SetColor(D2D1::ColorF((FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(1.0)));
	}
	return true;
}

bool D2DRenderContext::end_paint()
{
	HRESULT hr = S_OK;
	#if 0

	if (m_RenderTarget)
	{
		hr = m_RenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		printf(" Direct2D error: RenderTarget lost.\nThe GameEngine terminates the game.\n");
		return false; // app should close or re-initialize
	}

	// Create the rendering resources for rendering the 2D commands
	if (!m_DrawCommands.empty())
	{
		if (!m_VertexShader)
		{
			ShaderCreateParams params = ShaderCreateParams::vertex_shader("Source/Engine/Shaders/default_2d.vx.hlsl");
			m_VertexShader = ShaderCache::instance()->find_or_create(params);
			params = ShaderCreateParams::pixel_shader("Source/Engine/Shaders/default_2d.px.hlsl");
			m_PixelShader = ShaderCache::instance()->find_or_create(params);
		}

		{
			std::vector<SimpleVertex2D> vertices;
			std::vector<u32> indices;

			vertices.reserve(m_TotalVertices);
			indices.reserve(m_TotalIndices);

			u32 vtx_offset = 0;
			for (DrawCmd& cmd : m_DrawCommands)
			{
				cmd.m_IdxOffset = u32(indices.size());
				cmd.m_VertexOffset = u32(vertices.size());

				for (SimpleVertex2D const& vert : cmd.m_VertexBuffer)
				{
					vertices.push_back(vert);
				}

				for (u32 const& index : cmd.m_IdxBuffer)
				{
					indices.push_back(index);
				}
			}

			if (m_TotalVertices > m_CurrentVertices)
			{
				m_VertexBuffer.Reset();

				m_CurrentVertices = m_TotalVertices + 5000;

				CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(m_CurrentVertices * sizeof(SimpleVertex2D), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, sizeof(SimpleVertex2D));
				D3D11_SUBRESOURCE_DATA initial_data{};
				initial_data.pSysMem = vertices.data();
				ENSURE_HR(Graphics::get_device()->CreateBuffer(&desc, &initial_data, m_VertexBuffer.GetAddressOf()));
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource{};
				Graphics::get_ctx()->Map(m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

				SimpleVertex2D* data = (SimpleVertex2D*)mapped_resource.pData;
				memcpy(data, vertices.data(), vertices.size() * sizeof(SimpleVertex2D));
				Graphics::get_ctx()->Unmap(m_VertexBuffer.Get(), 0);
			}

			if (m_TotalIndices > m_CurrentIndices)
			{
				m_IndexBuffer.Reset();

				m_CurrentIndices = m_TotalIndices + 5000;

				CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(m_CurrentIndices * sizeof(u32), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0);
				D3D11_SUBRESOURCE_DATA initial_data{};
				initial_data.pSysMem = indices.data();
				ENSURE_HR(Graphics::get_device()->CreateBuffer(&desc, &initial_data, m_IndexBuffer.GetAddressOf()));
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource{};
				Graphics::get_ctx()->Map(m_IndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				u32* data = (u32*)mapped_resource.pData;
				memcpy(data, indices.data(), indices.size() * sizeof(u32));
				Graphics::get_ctx()->Unmap(m_IndexBuffer.Get(), 0);
			}
		}

		// Render the 2D commands
		{
			auto ctx = Graphics::get_ctx();

			CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(m_Renderer->get_raw_output_tex(), m_Renderer->get_raw_output_rtv());
			D3D11_RECT rect{ (LONG)viewport.TopLeftX, (LONG)viewport.TopLeftY, (LONG)viewport.TopLeftX + (LONG)viewport.Width, (LONG)viewport.TopLeftY + (LONG)viewport.Height };
			ctx->RSSetViewports(1, &viewport);
			ctx->RSSetScissorRects(1, &rect);

			ctx->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			auto rtv = m_Renderer->get_raw_output_rtv();
			ctx->OMSetRenderTargets(1, &rtv, nullptr);

			constexpr UINT vertexStride = sizeof(SimpleVertex2D);
			constexpr UINT vertexOffset = 0;
			ctx->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);

			ctx->IASetInputLayout(m_VertexShader->get_input_layout().Get());
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			ctx->VSSetShader(m_VertexShader->as<ID3D11VertexShader>().Get(), nullptr, 0);

			ctx->PSSetShader(m_PixelShader->as<ID3D11PixelShader>().Get(), nullptr, 0);

			auto cb = m_GlobalCB->Get();
			ctx->VSSetConstantBuffers(0, 1, &cb);

			ComPtr<ID3D11RasterizerState> rss = Graphics::get_rasterizer_state(RasterizerState::CullNone);
			ComPtr<ID3D11BlendState> bss = Graphics::get_blend_state(BlendState::AlphaBlend);
			ComPtr<ID3D11DepthStencilState> dss = Graphics::get_depth_stencil_state(DepthStencilState::NoDepth);
			ctx->RSSetState(rss.Get());
			ctx->OMSetBlendState(bss.Get(), nullptr, 0xFFFFFF);
			ctx->OMSetDepthStencilState(dss.Get(), 0);

			ID3D11SamplerState* samplers[] = {
				Graphics::get_sampler_state(SamplerState::MinMagMip_Linear).Get()
			};
			ctx->PSSetSamplers(0, 1, samplers);
			for (DrawCmd const& cmd : m_DrawCommands)
			{
				ID3D11ShaderResourceView* srvs[] = {
					cmd.m_Texture ? cmd.m_Texture : TextureHandle::white()->get_srv()
				};
				ctx->PSSetShaderResources(0, 1, srvs);
				float4x4 wv = cmd.m_WorldViewMatrix;
				float4x4 result = hlslpp::mul(wv, m_ProjectionMatrix);
				result._13 = 0.0f;
				result._23 = 0.0f;
				result._33 = 1.0f;
				result._43 = 0.0f;

				// Copy matrix to shader
				struct DrawData
				{
					Shaders::float4x4 mat;
					Shaders::float4 colour;
				};

				DrawData* dst = (DrawData*)m_GlobalCB->map(ctx.Get());
				dst->mat = Shaders::float4x4(result);
				dst->colour = cmd.m_Colour;
				m_GlobalCB->unmap(ctx.Get());

				ctx->DrawIndexed((UINT)cmd.m_IdxBuffer.size(), (UINT)cmd.m_IdxOffset, (UINT)cmd.m_VertexOffset);
			}
		}
	}

	#endif
	return true;
}

bool D2DRenderContext::draw_background(u32 color)
{
	//m_RenderTarget->Clear(D2D1::ColorF(color >> 8, 1.0));

	DrawCmd cmd{};
	cmd.m_Type = DrawCmd::DC_CLEAR;

	D2D1::ColorF* pData = (D2D1::ColorF*)cmd.m_Data;
	*pData = D2D1::ColorF(color >> 8, 1.0f);
	m_DrawCommands.push_back(std::move(cmd));
	return true;
}

bool D2DRenderContext::draw_line(float2 p1, float2 p2, float strokeWidth /*= 1.0*/)
{
	//_rt->DrawLine(D2D1::Point2F((FLOAT)p1.x, (FLOAT)p1.y), D2D1::Point2F((FLOAT)p2.x, (FLOAT)p2.y), _brush, (FLOAT)strokeWidth);
	// return true;

	float4x4 mat = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);

	f32 width = hlslpp::mul(m_ProjectionMatrix, strokeWidth).x * 10.0f;

	float3 dir = float3(p2 - p1, 0.0f);
	float3 tangent = hlslpp::cross(dir, float3(0.0, 0.0, 1.0f));

	DrawCmd command{};
	command.m_IdxBuffer = {
		// Top
		0, 1, 2,
		1, 3, 2
	};
	command.m_VertexBuffer = {
		SimpleVertex2D( Shaders::float2(p1 + tangent.xy * width * 0.5f), Shaders::float2({ 0.0f, 0.0f }) ),
		SimpleVertex2D( Shaders::float2(p2 + tangent.xy * width * 0.5f), Shaders::float2({ 0.0f, 0.0f }) ),
		SimpleVertex2D( Shaders::float2(p1 - tangent.xy * width * 0.5f), Shaders::float2({ 0.0f, 0.0f }) ),
		SimpleVertex2D( Shaders::float2(p2 - tangent.xy * width * 0.5f), Shaders::float2({ 0.0f, 0.0f }) ),
	};

	command.m_WorldViewMatrix = mat;
	command.m_Colour = float4(m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a);

	// Transform vertices to new
	m_TotalVertices += u32(command.m_VertexBuffer.size());
	m_TotalIndices += u32(command.m_IdxBuffer.size());
	m_DrawCommands.push_back(command);
	return true;
}

bool D2DRenderContext::draw_line(int x1, int y1, int x2, int y2)
{
	return draw_line(float2(x1, y1), float2(x2, y2), 1.0);
}

bool D2DRenderContext::draw_polygon(const std::vector<float2>& ptsArr, unsigned int count, bool close /*= true*/, double strokeWidth /*= 1.0*/)
{
	if (count < 2)
	{
		return false;
	}

	for (unsigned int countLoop = 0; countLoop < count - 1; ++countLoop)
	{
		draw_line((int)ptsArr[countLoop].x, (int)ptsArr[countLoop].y, (int)ptsArr[countLoop + 1].x, (int)ptsArr[countLoop + 1].y);
	}
	if (close)
	{
		draw_line((int)ptsArr[0].x, (int)ptsArr[0].y, (int)ptsArr[count - 1].x, (int)ptsArr[count - 1].y);
	}

	return true;
}

bool D2DRenderContext::draw_polygon(const std::vector<POINT>& ptsArr, unsigned int count, bool close /*= true*/)
{
	if (count < 2)
	{
		return false;
	}

	for (unsigned int countLoop = 0; countLoop < count - 1; ++countLoop)
	{
		draw_line(ptsArr[countLoop].x, ptsArr[countLoop].y, ptsArr[countLoop + 1].x, ptsArr[countLoop + 1].y);
	}
	if (close)
	{
		draw_line(ptsArr[0].x, ptsArr[0].y, ptsArr[count - 1].x, ptsArr[count - 1].y);
	}
	return true;
}

bool D2DRenderContext::fill_polygon(const std::vector<float2>& ptsArr, unsigned int count)
{
	if (count < 2)
	{
		return false;
	}

	HRESULT hr;

	if (m_RenderTarget)
	{
		// Create path geometry
		ID2D1PathGeometry* geometryPtr;
		hr = m_Factory->CreatePathGeometry(&(geometryPtr));
		if (FAILED(hr))
		{
			geometryPtr->Release();
			return false;
		}

		// Write to the path geometry using the geometry sink.
		ID2D1GeometrySink* geometrySinkPtr = nullptr;
		hr = geometryPtr->Open(&geometrySinkPtr);
		if (FAILED(hr))
		{
			geometrySinkPtr->Release();
			geometryPtr->Release();
			return false;
		}

		if (SUCCEEDED(hr))
		{
			geometrySinkPtr->BeginFigure(
					D2D1::Point2F((FLOAT)ptsArr[0].x, (FLOAT)ptsArr[0].y),
					D2D1_FIGURE_BEGIN_FILLED);

			for (unsigned int i = 0; i < count; ++i)
			{
				geometrySinkPtr->AddLine(D2D1::Point2F((FLOAT)ptsArr[i].x, (FLOAT)ptsArr[i].y));
			}

			geometrySinkPtr->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = geometrySinkPtr->Close();
		}

		geometrySinkPtr->Release();

		if (SUCCEEDED(hr))
		{
			m_RenderTarget->FillGeometry(geometryPtr, m_Brush);
			geometryPtr->Release();
			return true;
		}

		geometryPtr->Release();
	}
	return false;
}

bool D2DRenderContext::fill_polygon(const std::vector<POINT>& ptsArr, unsigned int count)
{
	// do not fill an empty polygon
	if (count < 2)
	{
		return false;
	}

	HRESULT hr;

		if (m_RenderTarget)
	{
		// Create path geometry
		ID2D1PathGeometry* geometryPtr;
		hr = m_Factory->CreatePathGeometry(&geometryPtr);
		if (FAILED(hr))
		{
			geometryPtr->Release();
			return false;
		}

		// Write to the path geometry using the geometry sink.
		ID2D1GeometrySink* geometrySinkPtr = nullptr;
		hr = geometryPtr->Open(&geometrySinkPtr);
		if (FAILED(hr))
		{
			geometrySinkPtr->Release();
			geometryPtr->Release();
			return false;
		}
		if (SUCCEEDED(hr))
		{
			geometrySinkPtr->BeginFigure(
					D2D1::Point2F((FLOAT)ptsArr[0].x, (FLOAT)ptsArr[0].y),
					D2D1_FIGURE_BEGIN_FILLED);

			for (unsigned int i = 0; i < count; ++i)
			{
				geometrySinkPtr->AddLine(D2D1::Point2F((FLOAT)ptsArr[i].x, (FLOAT)ptsArr[i].y));
			}

			geometrySinkPtr->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = geometrySinkPtr->Close();
			geometrySinkPtr->Release();
		}
		if (SUCCEEDED(hr))
		{
			m_RenderTarget->FillGeometry(geometryPtr, m_Brush);
			geometryPtr->Release();
			return true;
		}

		geometryPtr->Release();
	}
	return false;
}

bool D2DRenderContext::draw_rect(Rect rect, float strokeWidth /*= 1*/)
{
	if ((rect.right < rect.left) || (rect.bottom < rect.top))
	{
		return false;
	}

	DrawCmd command{};
	command.m_IdxBuffer = {
		// Top
		0, 1, 4,
		4, 1, 5,

		// Right
		1, 6, 5,
		1, 2, 6,

		// Bottom
		3, 7, 2,
		7, 6, 2,

		// Left
		0, 4, 7,
		3, 0, 7
	};
	command.m_VertexBuffer = {
		SimpleVertex2D( Shaders::float2(rect.left, rect.top), Shaders::float2(0.0f, 0.0f) ),
		SimpleVertex2D( Shaders::float2(rect.right, rect.top), Shaders::float2(0.0f, 0.0f) ),
		SimpleVertex2D( Shaders::float2(rect.right, rect.bottom), Shaders::float2(0.0f, 0.0f) ),
		SimpleVertex2D( Shaders::float2(rect.left, rect.bottom), Shaders::float2(0.0f, 0.0f) ),

		SimpleVertex2D( Shaders::float2(rect.left + strokeWidth, rect.top + strokeWidth), Shaders::float2(0.0f, 0.0f) ),
		SimpleVertex2D( Shaders::float2(rect.right - strokeWidth, rect.top + strokeWidth), Shaders::float2(0.0f, 0.0f) ),
		SimpleVertex2D( Shaders::float2(rect.right - strokeWidth, rect.bottom - strokeWidth), Shaders::float2(0.0f, 0.0f) ),
		SimpleVertex2D( Shaders::float2(rect.left + strokeWidth, rect.bottom - strokeWidth), Shaders::float2(0.0f, 0.0f) ),

	};

	command.m_WorldViewMatrix = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);
	command.m_Colour = float4(m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a);

	// Transform vertices to new
	m_TotalVertices += u32(command.m_VertexBuffer.size());
	m_TotalIndices += u32(command.m_IdxBuffer.size());
	m_DrawCommands.push_back(command);
	return true;
}

bool D2DRenderContext::draw_rect(float2 topLeft, float2 rightbottom, float strokeWidth /*= 1.0*/)
{
	Rect rect2{ float(topLeft.x), float(topLeft.y), rightbottom.x, rightbottom.y };
	return draw_rect(rect2, strokeWidth);
}

bool D2DRenderContext::draw_rect(RECT rect)
{
	Rect rect2{ float(rect.left), float(rect.top), float(rect.right), float(rect.bottom) };
	return draw_rect(rect2, 1.0);
}

bool D2DRenderContext::draw_rect(int left, int top, int right, int bottom)
{
	Rect rect2{ float(left), float(top), float(right), float(bottom) };
	return draw_rect(rect2, 1.0);
}

bool D2DRenderContext::fill_rect(Rect rect)
{
	if ((rect.right < rect.left) || (rect.bottom < rect.top))
	{
		return false;
	}

	DrawCmd command{};
	command.m_IdxBuffer = {
		0, 1, 2,
		0, 2, 3
	};
	command.m_VertexBuffer = {
		SimpleVertex2D{ Shaders::float2({ rect.left, rect.top }), Shaders::float2({ 0.0f, 0.0f }) },
		SimpleVertex2D{ Shaders::float2({ rect.right, rect.top }), Shaders::float2({ 0.0f, 0.0f }) },
		SimpleVertex2D{ Shaders::float2({ rect.right, rect.bottom }), Shaders::float2({ 0.0f, 0.0f }) },
		SimpleVertex2D{ Shaders::float2({ rect.left, rect.bottom }), Shaders::float2({ 0.0f, 0.0f }) },
	};

	command.m_WorldViewMatrix = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);
	command.m_Colour = float4(m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a);

	// Transform vertices to new
	m_TotalVertices += 4;
	m_TotalIndices += 6;
	m_DrawCommands.push_back(command);

	return true;
}

bool D2DRenderContext::fill_rect(float2 topLeft, float2 rightbottom)
{
	Rect rect2{ topLeft.x, topLeft.y, rightbottom.x, rightbottom.y };
	return fill_rect(rect2);
}

bool D2DRenderContext::fill_rect(RECT rect)
{
	Rect rect2{ float(rect.left), float(rect.top), float(rect.right), float(rect.bottom) };
	return fill_rect(rect2);
}

bool D2DRenderContext::fill_rect(int left, int top, int right, int bottom)
{
	Rect rect2{ float(left), float(top), float(right), float(bottom) };
	return fill_rect(rect2);
}

bool D2DRenderContext::draw_rounded_rect(Rect rect, int radiusX, int radiusY, float strokeWidth /*= 1.0*/)
{
	FAILMSG("Method not implemented.");
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	if(m_RenderTarget)
	{
		m_RenderTarget->DrawRoundedRectangle(d2dRoundedRect, m_Brush, 1.0);
	}
	return true;
}

bool D2DRenderContext::draw_rounded_rect(float2 topLeft, float2 rightbottom, int radiusX, int radiusY, float strokeWidth /*= 1.0*/)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	if (m_RenderTarget)
	{
		m_RenderTarget->DrawRoundedRectangle(d2dRoundedRect, m_Brush, (FLOAT)strokeWidth);
	}
	return true;
}

bool D2DRenderContext::draw_rounded_rect(RECT rect, int radiusX, int radiusY)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	if (m_RenderTarget)
	{
		m_RenderTarget->DrawRoundedRectangle(d2dRoundedRect, m_Brush, 1.0);
	}
	return true;
}

bool D2DRenderContext::draw_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);

	if (m_RenderTarget)
	{
		m_RenderTarget->DrawRoundedRectangle(d2dRoundedRect, m_Brush, 1.0);
	}
	return true;
}

bool D2DRenderContext::fill_rounded_rect(Rect rect, int radiusX, int radiusY)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);

	if (m_RenderTarget)
	{
		m_RenderTarget->FillRoundedRectangle(d2dRoundedRect, m_Brush);
	}
	return true;
}

bool D2DRenderContext::fill_rounded_rect(float2 topLeft, float2 rightbottom, int radiusX, int radiusY)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);

	if (m_RenderTarget)
	{
		m_RenderTarget->FillRoundedRectangle(d2dRoundedRect, m_Brush);
	}
	return true;
}

bool D2DRenderContext::fill_rounded_rect(RECT rect, int radiusX, int radiusY)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	if (m_RenderTarget)
	{
		m_RenderTarget->FillRoundedRectangle(d2dRoundedRect, m_Brush);
	}
	return true;
}

bool D2DRenderContext::fill_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	FAILMSG("Method not implemented.");

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	if (m_RenderTarget)
	{
		m_RenderTarget->FillRoundedRectangle(d2dRoundedRect, m_Brush);
	}
	return true;
}

bool D2DRenderContext::draw_ellipse(float2 centerPt, double radiusX, double radiusY, float strokeWidth /*= 1.0*/)
{
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);

	constexpr u32 c_ellipse_resolution = 32;

	DrawCmd command{};
	u32 expected_idx_count = c_ellipse_resolution * 6;
	u32 expected_vtx_count = c_ellipse_resolution * 4;
	command.m_VertexBuffer.reserve(expected_vtx_count);
	command.m_IdxBuffer.reserve(expected_idx_count);

	float2 center = float2(centerPt.x, centerPt.y);

	f32 step = 2.0f * M_PI / c_ellipse_resolution;
	for (f32 dt = 0.0f; dt <= 2.0f * M_PI; dt += step)
	{
		f32 dt_a = dt - step;
		f32 dt_b = dt;
		f32 dt_c = dt + step;
		f32 dt_d = dt + step + step;

		float2 a_pt = center + float2(cos(dt_a) * radiusX, sin(dt_a) * radiusY);
		float2 b_pt = center + float2(cos(dt_b) * radiusX, sin(dt_b) * radiusY);
		float2 c_pt = center + float2(cos(dt_c) * radiusX, sin(dt_c) * radiusY);
		float2 d_pt = center + float2(cos(dt_d) * radiusX, sin(dt_d) * radiusY);

		float2 tangent_ab = hlslpp::cross(float3(a_pt - b_pt, 0.0f), float3(0.0f, 0.0f, 1.0f)).xy;
		float2 tangent_bc = hlslpp::cross(float3(b_pt - c_pt, 0.0f), float3(0.0f, 0.0f, 1.0f)).xy;
		float2 tangent_cd = hlslpp::cross(float3(c_pt - d_pt, 0.0f), float3(0.0f, 0.0f, 1.0f)).xy;

		float2 tangent_b = hlslpp::normalize(tangent_ab + tangent_bc);
		float2 tangent_c = hlslpp::normalize(tangent_bc + tangent_cd);

		// Push the plane of this line segment
		u32 start_index = u32(command.m_VertexBuffer.size());
		f32 half_stroke_width = (f32)strokeWidth / 2.0f;
		command.m_VertexBuffer.push_back(SimpleVertex2D{ Shaders::float2(b_pt - tangent_b * half_stroke_width), Shaders::float2{} });
		command.m_VertexBuffer.push_back(SimpleVertex2D{ Shaders::float2(c_pt - tangent_c * half_stroke_width), Shaders::float2{} });
		command.m_VertexBuffer.push_back(SimpleVertex2D{ Shaders::float2(c_pt + tangent_c * half_stroke_width), Shaders::float2{} });
		command.m_VertexBuffer.push_back(SimpleVertex2D{ Shaders::float2(b_pt + tangent_b * half_stroke_width), Shaders::float2{} });

		// Push the indices of this line segment
		command.m_IdxBuffer.push_back(start_index);
		command.m_IdxBuffer.push_back(start_index + 1);
		command.m_IdxBuffer.push_back(start_index + 2);
		command.m_IdxBuffer.push_back(start_index);
		command.m_IdxBuffer.push_back(start_index + 2);
		command.m_IdxBuffer.push_back(start_index + 3);
	}

	command.m_WorldViewMatrix = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);
	command.m_Colour = float4(m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a);

	// Transform vertices to new
	m_TotalVertices += u32(command.m_VertexBuffer.size());
	m_TotalIndices += u32(command.m_IdxBuffer.size());
	m_DrawCommands.push_back(command);

	return true;
}

bool D2DRenderContext::draw_ellipse(int centerX, int centerY, int radiusX, int radiusY)
{
	draw_ellipse(float2(centerX, centerY), radiusX, radiusY);
	return true;
}

bool D2DRenderContext::fill_ellipse(float2 centerPt, double radiusX, double radiusY)
{
	constexpr u32 c_ellipse_resolution = 32;

	DrawCmd command{};
	u32 expected_idx_count = c_ellipse_resolution * 6;
	u32 expected_vtx_count = c_ellipse_resolution * 4;
	command.m_VertexBuffer.reserve(expected_vtx_count);
	command.m_IdxBuffer.reserve(expected_idx_count);

	float2 center = float2(centerPt.x, centerPt.y);

	f32 step = 2.0f * M_PI / c_ellipse_resolution;

	// Add the center vertex
	command.m_VertexBuffer.push_back(SimpleVertex2D{ Shaders::float2(center), Shaders::float2{} });

	// Populate all the vertices
	u32 n_points = c_ellipse_resolution;
	for (f32 dt = 0.0f; dt <= 2.0f * M_PI; dt += step)
	{
		f32 dt_a = dt;
		float2 start_pt = center + float2(cos(dt_a) * radiusX, sin(dt_a) * radiusY);
		command.m_VertexBuffer.push_back(SimpleVertex2D{ Shaders::float2(start_pt), Shaders::float2{} });
	}

	// Add the indices
	for (u32 i = 0; i < n_points; ++i)
	{
		command.m_IdxBuffer.push_back(0);
		command.m_IdxBuffer.push_back(i);
		command.m_IdxBuffer.push_back(std::max<u32>((i + 1) % n_points, 1));
	}

	command.m_WorldViewMatrix = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);
	command.m_Colour = float4(m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a);

	// Transform vertices to new
	m_TotalVertices += u32(command.m_VertexBuffer.size());
	m_TotalIndices += u32(command.m_IdxBuffer.size());
	m_DrawCommands.push_back(command);

	return true;
}

bool D2DRenderContext::fill_ellipse(int centerX, int centerY, int radiusX, int radiusY)
{
	// D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	//_rt->FillEllipse(ellipse, _brush);
	fill_ellipse(float2(centerX, centerY), radiusX, radiusY);
	return true;
}

bool D2DRenderContext::draw_string(std::string const& text, float2 topLeft, double right /*= -1*/, double bottom /*= -1*/)
{
	FAILMSG("Method not implemented.");

	if(m_RenderTarget)
	{
		D2D1_SIZE_F dstSize_f = m_RenderTarget->GetSize();
		D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
		if (right == -1 || bottom == -1) // ignore the right and bottom edge to enable drawing in entire Level
		{
			options = D2D1_DRAW_TEXT_OPTIONS_NONE;
			right = bottom = FLT_MAX;
		}
		D2D1_RECT_F layoutRect = (D2D1::RectF)((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(right), (FLOAT)(bottom));

		tstring wText(text.begin(), text.end());
		m_RenderTarget->DrawText(wText.c_str(), (UINT32)wText.length(), m_Font->GetTextFormat(), layoutRect, m_Brush, options);
	}

	return true;
}

bool D2DRenderContext::draw_string(const string& textRef, RECT boundingRect)
{
	return draw_string(textRef, boundingRect.left, boundingRect.top, boundingRect.right, boundingRect.bottom);
}

bool D2DRenderContext::draw_string(const string& textRef, Rect boundingRect)
{
	return draw_string(textRef, (int)boundingRect.left, (int)boundingRect.top, (int)boundingRect.right, (int)boundingRect.bottom);
}

bool D2DRenderContext::draw_string(const string& textRef, int xPos, int yPos, int right /*= -1*/, int bottom /*= -1*/)
{
	return draw_string(textRef, float2(xPos, yPos), right, bottom);
}

bool D2DRenderContext::draw_bitmap(Bitmap* imagePtr, float2 position, Rect srcRect)
{
	if (imagePtr == nullptr)
	{
		return false;
	}
	f32 width = (f32)imagePtr->get_width();
	f32 height = (f32)imagePtr->get_height();
	// The size and position, in device-independent pixels in the bitmap's coordinate space, of the area within the bitmap to draw.
	D2D1_RECT_F srcRect_f;
	srcRect_f.left = (FLOAT)srcRect.left / width;
	srcRect_f.right = (FLOAT)srcRect.right / width;
	srcRect_f.top = (FLOAT)srcRect.top / height;
	srcRect_f.bottom = (FLOAT)srcRect.bottom / height;

	// http://msdn.microsoft.com/en-us/library/dd371880(v=VS.85).aspx
	// The size and position, in device-independent pixels in the render target's coordinate space,
	// of the area to which the bitmap is drawn. If the rectangle is not well-ordered, nothing is drawn,
	// but the render target does not enter an error state.
	D2D1_RECT_F dstRect_f;
	dstRect_f.left = (FLOAT)position.x;
	dstRect_f.right = dstRect_f.left + (FLOAT)(srcRect.right - srcRect.left);
	dstRect_f.top = (FLOAT)position.y;
	dstRect_f.bottom = dstRect_f.top + (FLOAT)(srcRect.bottom - srcRect.top);

	//_rt->DrawBitmap(imagePtr->GetBitmapPtr(), dstRect_f, (FLOAT)imagePtr->GetOpacity(), _interpolation_mode, srcRect_f);

	DrawCmd command{};
	command.m_IdxBuffer = {
		0, 1, 2,
		0, 2, 3
	};
	command.m_VertexBuffer = {
		SimpleVertex2D{ Shaders::float2({ dstRect_f.left, dstRect_f.top }), Shaders::float2({ srcRect_f.left, srcRect_f.top }) },
		SimpleVertex2D{ Shaders::float2({ dstRect_f.right, dstRect_f.top }), Shaders::float2({ srcRect_f.right, srcRect_f.top }) },
		SimpleVertex2D{ Shaders::float2({ dstRect_f.right, dstRect_f.bottom }), Shaders::float2({ srcRect_f.right, srcRect_f.bottom }) },
		SimpleVertex2D{ Shaders::float2({ dstRect_f.left, dstRect_f.bottom }), Shaders::float2({ srcRect_f.left, srcRect_f.bottom }) },
	};

	command.m_WorldViewMatrix = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);
	command.m_Colour = float4(m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a);
	command.m_Colour = float4(1.0f, 1.0f, 1.0f, 1.0f);
	command.m_Texture = imagePtr->get_srv();

	// Transform vertices to new
	m_TotalVertices += 4;
	m_TotalIndices += 6;
	m_DrawCommands.push_back(command);
	return true;
}

bool D2DRenderContext::draw_bitmap(Bitmap* imagePtr, float2 position)
{
	assert(imagePtr);

	Rect srcRect2{ 0, 0, (f32)imagePtr->get_width(), (f32)imagePtr->get_height() };
	return draw_bitmap(imagePtr, position, srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap* imagePtr, int x, int y, RECT srcRect)
{
	Rect srcRect2{ (f32)srcRect.left, (f32)srcRect.top, (f32)srcRect.right, (f32)srcRect.bottom };
	return draw_bitmap(imagePtr, float2(x, y), srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap* imagePtr, int x, int y)
{
	assert(imagePtr);

	Rect srcRect2{ 0, 0, (f32)imagePtr->get_width(), (f32)imagePtr->get_height() };
	return draw_bitmap(imagePtr, float2(x, y), srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap* imagePtr, RECT srcRect)
{
	Rect srcRect2{ (f32)srcRect.left, (f32)srcRect.top, (f32)srcRect.right, (f32)srcRect.bottom };
	return draw_bitmap(imagePtr, float2(0, 0), srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap* imagePtr)
{
	assert(imagePtr);

	Rect srcRect2{ 0, 0, (f32)imagePtr->get_width(), (f32)imagePtr->get_height() };
	return draw_bitmap(imagePtr, float2(0, 0), srcRect2);
}

void D2DRenderContext::set_color(u32 color)
{
	u8 a = color & 0xff;
	u32 fshift = color >> 8;
	m_Colour = D2D1::ColorF(color >> 8, a / 255.0f);

	if(m_Brush)
	{
		m_Brush->SetColor(m_Colour);
	}
}

u32 D2DRenderContext::get_color() const
{
	D2D1_COLOR_F dColor = m_Brush->GetColor();
	return MK_COLOR((unsigned char)(dColor.r * 255), (unsigned char)(dColor.g * 255), (unsigned char)(dColor.b * 255), (unsigned char)(dColor.a * 255));
}

void D2DRenderContext::set_world_matrix(float4x4 const& mat)
{
	m_WorldMatrix = mat;
	update_transforms();
}

float4x4 D2DRenderContext::get_world_matrix() const
{
	return m_WorldMatrix;
}
void D2DRenderContext::set_view_matrix(float4x4 const& mat)
{
	m_ViewMatrix = mat;
	update_transforms();
}

float4x4 D2DRenderContext::get_view_matrix() const
{
	return m_ViewMatrix;
}

void D2DRenderContext::set_bitmap_interpolation_mode(bitmap_interpolation_mode mode)
{
	switch (mode)
	{
		case bitmap_interpolation_mode::linear:
			m_InterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
			break;
		case bitmap_interpolation_mode::nearest_neighbor:
			m_InterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
			break;
		default:
			assert("Case not supported");
			break;
	}
}

void D2DRenderContext::update_transforms()
{
	//FAILMSG("Method not implemented.");

	float4x4 result = hlslpp::mul(m_WorldMatrix, m_ViewMatrix);
	auto r = D2D1::Matrix3x2F(result._11, result._12, result._21, result._22, result._41, result._42);
	if(m_RenderTarget)
	{
		m_RenderTarget->SetTransform(r);
	}
}

} // namespace Graphics
#endif