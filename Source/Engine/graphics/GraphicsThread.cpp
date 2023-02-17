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

		m_GlobalCB = ConstantBuffer::create(GetRI(), sizeof(Shaders::float4x4) + sizeof(Shaders::float4), true, BufferUsage::Dynamic);
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
		renderer->ResizeSwapchain(engine->m_WindowWidth, engine->m_WindowHeight);
	}

	RenderContext& ctx = GetRI()->BeginContext();
	this->Render(ctx);
	this->Present(ctx);

	engine->m_SignalGraphicsToMain.release();
}

void GraphicsThread::Present(RenderContext& ctx)
{
	GameEngine* engine = GetGlobalContext()->m_Engine;
	Graphics::Renderer* renderer = GetGlobalContext()->m_Engine->m_Renderer.get();
	RenderWorld& world = m_FrameData.m_RenderWorld;

	PrecisionTimer present_timer{};
	present_timer.reset();
	present_timer.start();

	JONO_EVENT();
	auto d3d_ctx = renderer->get_raw_device_context();
	auto d3d_swapchain = renderer->get_raw_swapchain();

	size_t idx = Perf::get_current_frame_resource_index();

	// Present,
	GPU_MARKER(&ctx, "DrawEnd");
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

void GraphicsThread::Render(RenderContext& ctx)
{
	// JonS: When recreating swapchain imgui is a bit broken at the moment. Therefore skip imgui rendering till next frame
	bool doImgui = !m_FrameData.m_RecreateSwapchain;
	RenderWorld& world = m_FrameData.m_RenderWorld;

	GameEngine* engine = GetGlobalContext()->m_Engine;
	Graphics::Renderer* renderer = engine->m_Renderer.get();

	JONO_EVENT();
	GPU_SCOPED_EVENT(&ctx, "Frame");

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

	renderer->BeginFrame(ctx);

	// Render 3D before 2D
	EngineCfg const& cfg = m_FrameData.m_EngineCfg;
	if (cfg.m_UseD3D)
	{
		renderer->PreRender(ctx, world);

		GPU_SCOPED_EVENT(&ctx, "Render");
		// Render the shadows
		if (Graphics::s_EnableShadowRendering)
		{
			renderer->DrawShadowPass(ctx, world);
		}

		{
			GPU_SCOPED_EVENT(&ctx, "Main");
			renderer->DrawZPrePass(ctx, world);
			renderer->DrawOpaquePass(ctx, world);
		}
	}

	// Render Direct2D to the swapchain
	if (cfg.m_UseD2D)
	{
#if FEATURE_D2D
		this->RenderD2D(ctx);
#else
		LOG_ERROR(System, "Trying to use D2D but the build isn't compiled with D2D enabled!");
		DebugBreak();
#endif
	}

	shared_ptr<OverlayManager> overlayManager = nullptr;
	if(cfg.m_UseD3D)
	{
		overlayManager = engine->m_OverlayManager;
	}
	renderer->DrawPost(ctx, world, overlayManager, doImgui);
	renderer->EndFrame(ctx);
}

#if FEATURE_D2D
void GraphicsThread::RenderD2D(RenderContext& ctx)
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
		GPU_SCOPED_EVENT(&ctx, "D2D:Paint");

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
				if(renderData.m_VertexBuffer.IsValid())
                {
                    GetRI()->ReleaseResource(renderData.m_VertexBuffer);
				}
				renderData.m_CurrentVertices = renderData.m_TotalVertices;

				CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(renderData.m_CurrentVertices * sizeof(SimpleVertex2D), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, sizeof(SimpleVertex2D));
				D3D11_SUBRESOURCE_DATA initial_data{};
				initial_data.pSysMem = vertices.data();
                renderData.m_VertexBuffer = GetRI()->CreateBuffer(desc, &initial_data, "2D Dynamic Vertex Buffer");
			}
			else if(renderData.m_TotalVertices > 0)
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource{};
                ctx.Map(renderData.m_VertexBuffer);

				SimpleVertex2D* data = (SimpleVertex2D*)mapped_resource.pData;
				memcpy(data, vertices.data(), vertices.size() * sizeof(SimpleVertex2D));
                ctx.Unmap(renderData.m_VertexBuffer);
			}

			if (renderData.m_TotalIndices > renderData.m_CurrentIndices)
			{
				if(renderData.m_IndexBuffer.IsValid())
                {
                    GetRI()->ReleaseResource(renderData.m_IndexBuffer);
				}
				renderData.m_CurrentIndices = renderData.m_TotalIndices;

				CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(renderData.m_CurrentIndices * sizeof(u32), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0);
				D3D11_SUBRESOURCE_DATA initial_data{};
				initial_data.pSysMem = indices.data();
                renderData.m_IndexBuffer = GetRI()->CreateBuffer(desc, &initial_data);
			}
			else if(renderData.m_TotalIndices > 0)
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource{};
                ctx.Map(renderData.m_IndexBuffer);
				u32* data = (u32*)mapped_resource.pData;
				memcpy(data, indices.data(), indices.size() * sizeof(u32));
                ctx.Unmap(renderData.m_IndexBuffer);
			}
		}

		{
			Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (FLOAT)renderer->GetDrawableWidth();
            viewport.height = (FLOAT)renderer->GetDrawableHeight();
            viewport.minZ = 0.0f;
            viewport.maxZ = 1.0f;

			TRect<u32> rect = TRect<u32>{ (u32)viewport.x, (u32)viewport.y, (u32)viewport.x + (u32)viewport.width, (u32)viewport.y + (u32)viewport.height };
            ctx.SetViewports({ viewport });
            ctx.SetScissorRects({ rect });

			ctx.IASetIndexBuffer(renderData.m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			GraphicsResourceHandle rtv = renderer->get_raw_output_rtv();
            ctx.SetTarget(rtv, GraphicsResourceHandle::Invalid());

			constexpr UINT vertexStride = sizeof(SimpleVertex2D);
			constexpr UINT vertexOffset = 0;
            ctx.IASetVertexBuffers(0, { renderData.m_VertexBuffer }, { sizeof(SimpleVertex2D) }, { 0 });

			ctx.IASetInputLayout(m_VertexShader->GetInputLayout());
            ctx.IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

			ctx.VSSetShader(m_VertexShader->as<ID3D11VertexShader>().Get());
			ctx.PSSetShader(m_PixelShader->as<ID3D11PixelShader>().Get());

            ctx.SetConstantBuffers(ShaderStage::Vertex, 0, { m_GlobalCB->get_buffer() });

			GraphicsResourceHandle rss = Graphics::GetRasterizerState(RasterizerState::CullNone);
            GraphicsResourceHandle bss = (Graphics::GetBlendState(BlendState::AlphaBlend));
            GraphicsResourceHandle dss = (Graphics::GetDepthStencilState(DepthStencilState::NoDepth));
            ctx.RSSetState(rss);
            ctx.OMSetBlendState(bss, {}, 0xFFFFFF);
            ctx.OMSetDepthStencilState(dss, 0);

			GraphicsResourceHandle samplers = {
				Graphics::GetSamplerState(SamplerState::MinMagMip_Linear)
			};
            ctx.SetSamplers(ShaderStage::Pixel, 0, { samplers });
			for (DrawCmd const& cmd : renderData.m_DrawCommands)
			{
				if(cmd.m_Type == DrawCmd::DC_MESH)
				{
                    GraphicsResourceHandle srv = cmd.m_TextureSRV ? cmd.m_TextureSRV : TextureHandle::white()->GetSRV();
                    ctx.SetShaderResources(ShaderStage::Pixel, 0, { srv });

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

					DrawData* dst = (DrawData*)m_GlobalCB->map(ctx);
					dst->mat = Shaders::float4x4(result);
					dst->colour = cmd.m_Colour;
					m_GlobalCB->unmap(ctx);

					ctx.DrawIndexed((UINT)cmd.m_IdxBuffer.size(), (UINT)cmd.m_IdxOffset, (UINT)cmd.m_VertexOffset);
				}
				else if(cmd.m_Type == DrawCmd::DC_CLEAR)
				{
					float const* color = reinterpret_cast<float const*>(&cmd.m_Data[0]);
                    ctx.ClearRenderTarget(rtv, float4(color[0], color[1], color[2], color[3]));
				}
			}
		}
	}
}
#endif
