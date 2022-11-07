#pragma once
#include "Core/Thread.h" 
#include "Graphics/RenderWorld.h"

#include "SmartPtr.h"
#include "EngineCfg.h"
#include "2DRenderContext.h"

namespace Graphics
{
class Renderer;
class RendererDebugTool;
class D2DRenderContext;
}

class GraphicsThread : public Threading::Thread
{
	friend class Graphics::Renderer;
	friend class Graphics::RendererDebugTool;

public:
	GraphicsThread();
	virtual ~GraphicsThread() {}

	enum class Stage
	{
		Initialization, // During this stage all D3D objects are being setup
		Running, // During this stage we are processing frames and running the game
		Cleanup, // During this stage we are doing cleanup
		Terminated
	};

	void WaitForStage(Stage stage);
	void ChangeStage(Stage targetStage);

	void Run() override;

	void Sync();

private:

	void DoFrame();

	void Present();
	void Render();

	void RenderD2D();

	struct FrameData
	{
		bool m_VSyncEnabled = false;
		bool m_RecreateSwapchain = false;
		bool m_DebugPhysicsRendering = false;

		EngineCfg m_EngineCfg;

		uint2 m_ViewportSize;
		uint2 m_WindowSize;

		RenderWorld m_RenderWorld;
		ImDrawData m_DrawData;

		std::vector<Graphics::DrawCmd> m_Render2DDrawCommands;
	};

	FrameData m_FrameData;


	std::mutex m_StageChangedCS;
	std::condition_variable m_StageChangedCV;

	Stage m_Stage;
};
