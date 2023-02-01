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
class Shader;
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

	void Present(RenderContext& ctx);
    void Render(RenderContext& ctx);

	void RenderD2D(RenderContext& ctx);

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

		struct Render2DData
		{
			std::vector<Graphics::DrawCmd> m_DrawCommands;
			u32 m_TotalVertices = 0;
			u32 m_TotalIndices = 0;

			u32 m_CurrentVertices = 0;
			u32 m_CurrentIndices = 0;

			ComPtr<ID3D11Buffer> m_VertexBuffer;
			ComPtr<ID3D11Buffer> m_IndexBuffer;
			ComPtr<ID3D11InputLayout> m_InputLayout;

			ConstantBufferRef m_GlobalCB;

			float4x4 m_ProjectionMatrix;
		};
		Render2DData m_Render2DData;
	};

	FrameData m_FrameData;


	std::mutex m_StageChangedCS;
	std::condition_variable m_StageChangedCV;

	Stage m_Stage;

	ConstantBufferRef m_GlobalCB;
	shared_ptr<Graphics::Shader> m_VertexShader;
	shared_ptr<Graphics::Shader> m_PixelShader;
};
