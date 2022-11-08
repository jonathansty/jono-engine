#include "engine.pch.h"
#include "Logging.h"
#include "Graphics.h"
#include "GraphicsThread.h"
#include "Perf.h"
#include "GameEngine.h"
#include "Graphics/Renderer.h"
#include "Graphics/2DRenderContext.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/Shader.h"
#include "Core/TextureResource.h"
#include "AbstractGame.h"

 GraphicsThread::GraphicsThread()
		: Thread("Graphics")
		, m_Stage(Stage::Initialization)
{
	 ASSERT(!GetGlobalContext()->m_GraphicsThread);
	 GetGlobalContext()->m_GraphicsThread = this;
 }

void GraphicsThread::WaitForStage(Stage stage)
{
	if (stage == m_Stage)
	{
		return;
	}

	std::unique_lock _lock(m_StageChangedCS);
	m_StageChangedCV.wait(_lock);
}

void GraphicsThread::ChangeStage(Stage targetStage)
{
	std::lock_guard stageGuard(m_StageChangedCS);
	m_Stage = targetStage;
	m_StageChangedCV.notify_all();
}

void GraphicsThread::Run()
{
	::SetThreadDescription(GetCurrentThread(), L"GraphicsThread");
	JONO_THREAD("GraphicsThread");

	m_Running = true;

	ChangeStage(Stage::Initialization);

	LOG_INFO(Graphics, "Launching graphics thread...");

	using namespace Graphics;
	GameEngine* engine = GetGlobalContext()->m_Engine;
	Renderer* renderer = engine->m_Renderer.get();

	ChangeStage(Stage::Running);

	u64 _frame = 0;
	while (IsRunning())
	{
		// Wait until the engine has signalled a new frame is ready
		engine->m_SignalMainToGraphics.acquire();
		++_frame;
		OPTICK_FRAME_EVENT(Optick::FrameType::GPU);
		OPTICK_TAG("Frame", _frame);
		DoFrame();
	}
	ChangeStage(Stage::Cleanup);
	LOG_INFO(Graphics, "Shutting down render thread...");
	ChangeStage(Stage::Terminated);
	m_Running = false;
}

void GraphicsThread::Sync()
{
	if (!m_VertexShader)
	{
		using namespace Graphics;
		ShaderCreateParams params = ShaderCreateParams::vertex_shader("Source/Engine/Shaders/default_2d.vx.hlsl");
		m_VertexShader = ShaderCache::instance()->find_or_create(params);
		params = ShaderCreateParams::pixel_shader("Source/Engine/Shaders/default_2d.px.hlsl");
		m_PixelShader = ShaderCache::instance()->find_or_create(params);

		m_GlobalCB = ConstantBuffer::create(Graphics::get_device().Get(), sizeof(Shaders::float4x4) + sizeof(Shaders::float4), true, BufferUsage::Dynamic);
	}

	// Setup the graphics thread parameters
	GlobalContext* context = GetGlobalContext();
	GameEngine* engine = context->m_Engine;
	ASSERT(std::this_thread::get_id() == GameEngine::s_MainThreadID);

	m_FrameData.m_VSyncEnabled = engine->m_VSyncEnabled;
	m_FrameData.m_RecreateSwapchain = engine->m_RecreateSwapchainRequested;
	m_FrameData.m_RenderWorld = *engine->get_render_world();
	m_FrameData.m_EngineCfg = engine->m_EngineCfg;
	m_FrameData.m_DebugPhysicsRendering = engine->m_DebugPhysicsRendering;

	m_FrameData.m_ViewportSize = uint2(engine->GetViewportSize().x, engine->GetViewportSize().y);
	m_FrameData.m_WindowSize = uint2(engine->GetWindowSize().x, engine->GetWindowSize().y);

	if(engine->m_D2DRenderContext.IsValid())
	{
		m_FrameData.m_Render2DData.m_DrawCommands = engine->m_D2DRenderContext->GetCommands();
		m_FrameData.m_Render2DData.m_TotalIndices = engine->m_D2DRenderContext->m_TotalIndices;
		m_FrameData.m_Render2DData.m_TotalVertices = engine->m_D2DRenderContext->m_TotalVertices;
		m_FrameData.m_Render2DData.m_ProjectionMatrix = engine->m_D2DRenderContext->m_ProjectionMatrix;
	}

	// Copy over ImDrawData for next frame
	ImDrawData* drawData = ImGui::GetDrawData();
	ImDrawData* gtDrawData = &m_FrameData.m_DrawData;
	if (gtDrawData->CmdLists != nullptr)
	{
		for (int i = 0; i < gtDrawData->CmdListsCount; i++)
		{
			delete gtDrawData->CmdLists[i];
		}
		delete gtDrawData->CmdLists;
		gtDrawData->CmdLists = nullptr;
	}
	gtDrawData->Clear();
	{
		*gtDrawData = *drawData;
		gtDrawData->CmdLists = new ImDrawList*[drawData->CmdListsCount];
		for (int i = 0; i < drawData->CmdListsCount; i++)
		{
			gtDrawData->CmdLists[i] = drawData->CmdLists[i]->CloneOutput();
		}
	}
}

void GraphicsThread::DoFrame()
{
	ASSERT(std::this_thread::get_id() == GetThread().get_id());

	GameEngine* engine = GetGlobalContext()->m_Engine;
	Graphics::Renderer* renderer = GetGlobalContext()->m_Engine->m_Renderer.get();
	RenderWorld& world = m_FrameData.m_RenderWorld;

	Perf::begin_frame(renderer->get_raw_device_context());

	if (m_FrameData.m_RecreateSwapchain)
	{
		renderer->resize_swapchain(engine->m_WindowWidth, engine->m_WindowHeight);
	}

	this->Render();
	this->Present();

	engine->m_SignalGraphicsToMain.release();
}

void GraphicsThread::Present()
{
	GameEngine* engine = GetGlobalContext()->m_Engine;
	Graphics::Renderer* renderer = GetGlobalContext()->m_Engine->m_Renderer.get();
	RenderWorld& world = m_FrameData.m_RenderWorld;

	PrecisionTimer present_timer{};
	present_timer.reset();
	present_timer.start();

	JONO_EVENT();
	auto d3d_ctx = renderer->get_raw_device_context();
	auto d3d_annotation = renderer->get_raw_annotation();
	auto d3d_swapchain = renderer->get_raw_swapchain();

	size_t idx = Perf::get_current_frame_resource_index();

	// Present,
	GPU_MARKER(d3d_annotation, L"DrawEnd");
	u32 flags = 0;
	if (!m_FrameData.m_VSyncEnabled)
	{
		flags |= DXGI_PRESENT_ALLOW_TEARING;
	}
	d3d_ctx->OMSetRenderTargets(0, nullptr, nullptr);
	d3d_swapchain->Present(m_FrameData.m_VSyncEnabled ? 1 : 0, flags);

	auto& timer = engine->m_GpuTimings[idx];
	timer.end(d3d_ctx);
	Perf::end_frame(d3d_ctx);

	// Render all other imgui windows
	ImGui::RenderPlatformWindowsDefault();

	present_timer.stop();
	engine->m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::PresentCPU, present_timer.get_delta_time() * 1000.0);
}

void GraphicsThread::Render()
{
	// JonS: When recreating swapchain imgui is a bit broken at the moment. Therefore skip imgui rendering till next frame
	bool doImgui = !m_FrameData.m_RecreateSwapchain;
	RenderWorld& world = m_FrameData.m_RenderWorld;

	GameEngine* engine = GetGlobalContext()->m_Engine;
	Graphics::Renderer* renderer = engine->m_Renderer.get();

	JONO_EVENT();
	auto d3d_annotation = renderer->get_raw_annotation();
	GPU_SCOPED_EVENT(d3d_annotation, "Frame");

	// Process the previous frame gpu timers here to allow our update thread to run first
	// #TODO: Move to graphics thread
	if (Perf::can_collect())
	{
		JONO_EVENT("PerfCollect");
		size_t current = Perf::get_previous_frame_resource_index();

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
		UINT64 start;
		UINT64 end;

		if (Perf::collect_disjoint(renderer->get_raw_device_context(), timestampDisjoint))
		{
			auto& timing_data = engine->m_GpuTimings[current];
			f64 cpuTime;
			timing_data.flush(renderer->get_raw_device_context(), start, end, cpuTime);

			double diff = (double)(end - start) / (double)timestampDisjoint.Frequency;
			engine->m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::RenderGPU, (float)(diff * 1000.0));
			engine->m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::RenderCPU, (float)(cpuTime * 1000.0));
		}
	}

	size_t idx = Perf::get_current_frame_resource_index();

	// Begin frame gpu timer
	auto& timer = engine->m_GpuTimings[idx];
	timer.begin(renderer->get_raw_device_context());

	renderer->begin_frame();

	// Render 3D before 2D
	EngineCfg const& cfg = m_FrameData.m_EngineCfg;
	if (cfg.d3d_use)
	{
		renderer->pre_render(world);

		GPU_SCOPED_EVENT(d3d_annotation, "Render");
		// Render the shadows
		if (Graphics::s_EnableShadowRendering)
		{
			renderer->render_shadow_pass(world);
		}

		{
			GPU_SCOPED_EVENT(d3d_annotation, "Main");
			renderer->render_zprepass(world);
			renderer->render_opaque_pass(world);
		}
	}

	// Render Direct2D to the swapchain
	if (cfg.d2d_use)
	{
#if FEATURE_D2D
		this->RenderD2D();
#else
		LOG_ERROR(System, "Trying to use D2D but the build isn't compiled with D2D enabled!");
		DebugBreak();
#endif
	}

	renderer->render_post(world, engine->m_OverlayManager, doImgui);
	renderer->end_frame();
}

#if FEATURE_D2D
void GraphicsThread::RenderD2D()
{
	//#TODO: Revisit thread safety here. Remove engine->m_DefaultFont
	GameEngine* engine = GetGlobalContext()->m_Engine;
	Graphics::Renderer* renderer = engine->m_Renderer.get();

	// Plan to modernize the 2D and deprecate Direct2D
	// 1. Collect all the draw commands in buffers and capture the required data
	// 2. during end_paint 'flush' draw commands and create required vertex buffers
	// 3. Execute each draw command binding the right buffers and views
	uint2 size = m_FrameData.m_ViewportSize;

	FrameData::Render2DData& renderData = m_FrameData.m_Render2DData;
	std::vector<Graphics::DrawCmd>& commands =renderData.m_DrawCommands;

	// Execute all the 2D draw commands 
	{
		using namespace Graphics;
		GPU_SCOPED_EVENT(renderer->get_raw_annotation(), "D2D:Paint");

		// First construct the necessary caching
		{
			std::vector<SimpleVertex2D> vertices;
			std::vector<u32> indices;

			vertices.reserve(renderData.m_TotalVertices);
			indices.reserve(renderData.m_TotalIndices);

			u32 vtx_offset = 0;
			for (DrawCmd& cmd : commands)
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

			if (renderData.m_TotalVertices > renderData.m_CurrentVertices)
			{
				renderData.m_VertexBuffer.Reset();

				renderData.m_CurrentVertices = renderData.m_TotalVertices;

				CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(renderData.m_CurrentVertices * sizeof(SimpleVertex2D), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, sizeof(SimpleVertex2D));
				D3D11_SUBRESOURCE_DATA initial_data{};
				initial_data.pSysMem = vertices.data();
				ENSURE_HR(Graphics::get_device()->CreateBuffer(&desc, &initial_data, renderData.m_VertexBuffer.GetAddressOf()));
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource{};
				Graphics::get_ctx()->Map(renderData.m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

				SimpleVertex2D* data = (SimpleVertex2D*)mapped_resource.pData;
				memcpy(data, vertices.data(), vertices.size() * sizeof(SimpleVertex2D));
				Graphics::get_ctx()->Unmap(renderData.m_VertexBuffer.Get(), 0);
			}

			if (renderData.m_TotalIndices > renderData.m_CurrentIndices)
			{
				renderData.m_IndexBuffer.Reset();
				renderData.m_CurrentIndices = renderData.m_TotalIndices;

				CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(renderData.m_CurrentIndices * sizeof(u32), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0);
				D3D11_SUBRESOURCE_DATA initial_data{};
				initial_data.pSysMem = indices.data();
				ENSURE_HR(Graphics::get_device()->CreateBuffer(&desc, &initial_data, renderData.m_IndexBuffer.GetAddressOf()));
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource{};
				Graphics::get_ctx()->Map(renderData.m_IndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				u32* data = (u32*)mapped_resource.pData;
				memcpy(data, indices.data(), indices.size() * sizeof(u32));
				Graphics::get_ctx()->Unmap(renderData.m_IndexBuffer.Get(), 0);
			}
		}

		{
			auto ctx = Graphics::get_ctx();

			CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(renderer->get_raw_output_tex(), renderer->get_raw_output_rtv());
			D3D11_RECT rect{ (LONG)viewport.TopLeftX, (LONG)viewport.TopLeftY, (LONG)viewport.TopLeftX + (LONG)viewport.Width, (LONG)viewport.TopLeftY + (LONG)viewport.Height };
			ctx->RSSetViewports(1, &viewport);
			ctx->RSSetScissorRects(1, &rect);

			ctx->IASetIndexBuffer(renderData.m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			ID3D11RenderTargetView* rtv = renderer->get_raw_output_rtv();
			ctx->OMSetRenderTargets(1, &rtv, nullptr);

			constexpr UINT vertexStride = sizeof(SimpleVertex2D);
			constexpr UINT vertexOffset = 0;
			ctx->IASetVertexBuffers(0, 1, renderData.m_VertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);

			ctx->IASetInputLayout(m_VertexShader->get_input_layout().Get());
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			ctx->VSSetShader(m_VertexShader->as<ID3D11VertexShader>().Get(), nullptr, 0);

			ctx->PSSetShader(m_PixelShader->as<ID3D11PixelShader>().Get(), nullptr, 0);

			ID3D11Buffer* cb = m_GlobalCB->Get();
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
			for (DrawCmd const& cmd : renderData.m_DrawCommands)
			{
				if(cmd.m_Type == DrawCmd::DC_MESH)
				{
					ID3D11ShaderResourceView* srvs[] = {
						cmd.m_Texture ? cmd.m_Texture : TextureHandle::white()->get_srv()
					};
					ctx->PSSetShaderResources(0, 1, srvs);
					float4x4 wv = cmd.m_WorldViewMatrix;
					float4x4 result = hlslpp::mul(wv, renderData.m_ProjectionMatrix);
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
				else if(cmd.m_Type == DrawCmd::DC_CLEAR)
				{
					float const* color = reinterpret_cast<float const*>(&cmd.m_Data[0]);
					ctx->ClearRenderTargetView(rtv, color);
				}
			}
		}
	}
}
#endif
