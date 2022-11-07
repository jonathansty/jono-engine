#include "engine.pch.h"
#include "Logging.h"
#include "Graphics.h"
#include "GraphicsThread.h"
#include "Perf.h"
#include "GameEngine.h"
#include "Graphics/Renderer.h"
#include "Graphics/2DRenderContext.h"
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
		m_FrameData.m_Render2DDrawCommands = engine->m_D2DRenderContext->GetCommands();
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
	Graphics::D2DRenderContext& context = *engine->m_D2DRenderContext;
	uint2 size = m_FrameData.m_ViewportSize;

	std::vector<Graphics::DrawCmd> const& commands = m_FrameData.m_Render2DDrawCommands;

	{
		using namespace Graphics;
		GPU_SCOPED_EVENT(renderer->get_raw_annotation(), "D2D:Paint");

		// First construct the necessary caching
		//{
		//	std::vector<SimpleVertex2D> vertices;
		//	std::vector<u32> indices;

		//	vertices.reserve(m_TotalVertices);
		//	indices.reserve(m_TotalIndices);

		//	u32 vtx_offset = 0;
		//	for (DrawCmd const& cmd : commands)
		//	{
		//		cmd.m_IdxOffset = u32(indices.size());
		//		cmd.m_VertexOffset = u32(vertices.size());

		//		for (SimpleVertex2D const& vert : cmd.m_VertexBuffer)
		//		{
		//			vertices.push_back(vert);
		//		}

		//		for (u32 const& index : cmd.m_IdxBuffer)
		//		{
		//			indices.push_back(index);
		//		}
		//	}
		//}


		ID3D11DeviceContext* ctx = renderer->get_ctx()._ctx.Get();

		ID3D11RenderTargetView* currentRTV = nullptr;
		ID3D11DepthStencilView* currentDSV = nullptr;

		for (DrawCmd const& cmd : commands)
		{
			switch(cmd.m_Type)
			{
				case DrawCmd::DC_MESH:
				{
				
				}break;
				case DrawCmd::DC_CLEAR:
				{
					float const* color = reinterpret_cast<float const*>(&cmd.m_Data[0]);
					ctx->ClearRenderTargetView(currentRTV, color);
				} break;
				default:
					break;
			}
			

		}
	}

	// draw Box2D debug rendering
	// http://www.iforce2d.net/b2dtut/debug-draw
	if (m_FrameData.m_DebugPhysicsRendering)
	{
		// dimming rect in screenspace
		context.set_world_matrix(float4x4::identity());
		float4x4 matView = context.get_view_matrix();
		context.set_view_matrix(float4x4::identity());
		context.set_color(MK_COLOR(0, 0, 0, 127));
		context.fill_rect(0, 0, m_FrameData.m_WindowSize.x, m_FrameData.m_WindowSize.y);
		context.set_view_matrix(matView);

		engine->m_Box2DDebugRenderer.set_draw_ctx(&context);
		engine->m_Box2DWorld->DebugDraw();
		engine->m_Box2DDebugRenderer.set_draw_ctx(nullptr);
	}

	bool result = context.end_paint();

	// if drawing failed, terminate the game
	if (!result)
	{
		FAILMSG("Something went wrong drawing D2D elements. Crashing the game...");
		this->Terminate();
		engine->m_IsRunning = false;
	}
}
#endif
