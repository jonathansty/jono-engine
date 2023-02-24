#pragma once

#include "Graphics.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "EngineCfg.h"
#include "GameSettings.h"

#include "DirectXTK/CommonStates.h"
#include "DirectXTK/Effects.h"
#include "DirectXTK/VertexTypes.h"

#include "Debug/OverlayManager.h"
#include "Core/Math.h"
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "Visibility.h"
#include "RendererDebug.h"

#include "Shaders/CommonShared.h"

#if FEATURE_D2D
#include "2DRenderContext.h"
#endif
#include <Source/Engine/graphics/2DRenderContext.h>

#include "Graphics/RenderInterface.h"
#include "Graphics/VertexLayout.h"

#include "Core/TextureResource.h"
#include "Core/ModelResource.h"

class RenderWorld;
class RenderWorldCamera;
class Material;
class MaterialInstance;
using ShaderConstRef = shared_ptr<class Shader>;

namespace cli
{
using CommandLine = std::vector<std::string>;
}

namespace Graphics
{

using FrustumCorners = Math::FrustumCorners;


struct RenderPass
{
	enum Value
	{
		ZPrePass,
		Opaque,
		Shadow_CSM0,
		Shadow_CSM1,
		Shadow_CSM2,
		Shadow_CSM3,
		Post

	};

	static bool IsShadowPass(RenderPass::Value pass) { return pass >= Shadow_CSM0 && pass <= Shadow_CSM3; }

	static std::string ToString(RenderPass::Value pass)
	{
		switch (pass)
		{
			case Value::ZPrePass:
				return "ZPrePass";
			case Value::Opaque:
				return "Opaque";
			case Value::Post:
				return "Post";
			case Value::Shadow_CSM0:
				return "Shadow_CSM0";
			case Value::Shadow_CSM1:
				return "Shadow_CSM1";
			case Value::Shadow_CSM2:
				return "Shadow_CSM2";
			case Value::Shadow_CSM3:
				return "Shadow_CSM3";
			default:
				return "Invalid";
				break;
		}
	}
};

struct ViewParams
{
	float4x4 view;
	float4x4 proj;
	float3 view_position;
	float3 view_direction;
	Viewport viewport;
	RenderPass::Value pass;
	bool reverse_z = true;
};



//__declspec(align(16)) 
//struct DirectionalLightInfo
//{
//	float4   colour;
//	float4   direction;
//	float4x4 light_space;
//
//	float4x4 cascades[MAX_CASCADES];
//	float4   cascade_distances[MAX_CASCADES];
//
//	u32 num_cascades;
//
//};

__declspec(align(16)) 
struct AmbientInfo
{
	float4 ambient;
};

__declspec(align(16))
struct ViewportCB
{
	float vp_half_width;
	float vp_half_height;
	float vp_top_x;
	float vp_top_y;
	float vp_min_depth;
	float vp_max_depth;
	float pad[2];
};


__declspec(align(16)) 
struct GlobalCB
{
	float4x4 view;
	float4x4 inv_view;
	float4x4 proj;
	float4x4 inv_proj;
	float4x4 inv_view_projection;
	
	float4 view_direction;
	float4 view_pos;

	ViewportCB vp;
	AmbientInfo ambient;
	DirectionalLightInfo lights[MAX_DIRECTIONAL_LIGHTS];

	u32 num_directional_lights;
	u32 num_lights; // local lights
	u32 num_tiles_x;
	float padding;
};

__declspec(align(16)) 
struct DebugCB
{
	unsigned int m_VisualizeMode;
	uint8_t pad[12];
};

__declspec(align(16)) 
struct PostCB
{
	float m_ViewportWidth;
	float m_ViewportHeight;
	float padding[2];
};

struct ModelCB
{
	float4x4 world;
	float4x4 wv;
	float4x4 wvp;
};


// information that is needed to represent 1 draw call
struct DrawCall
{
    float4x4 _transform;

    // Mesh index buffer
    GraphicsResourceHandle _index_buffer;

    // Vertex buffers
    GraphicsResourceHandle _vertex_buffer;

    VertexLayoutFlags _input_layout_flags;

    // First vertex offset this mesh starts at in the vertex buffer
    u64 _first_vertex;

    // First index location offset this mesh starts at in the index buffer
    u64 _first_index;

    // The amount of indices associated with this mesh
    u64 _index_count;

    // Material (e.g. shaders, textures, constant buffer, input layouts)
    MaterialInstance const* _material;

	#ifdef _DEBUG
    Model const* _model;
	#endif
};



class Renderer final
{
	friend class RendererDebugTool;	

	static constexpr u32 c_MaxLights = 2048;

public:
	void Init(EngineCfg const& settings, GameCfg const& game_settings, cli::CommandLine const& cmdline);
	void InitForWindow(SDL_Window* wnd);

	u32 GetDrawableWidth() const { return m_DrawableAreaWidth; };
	u32 GetDrawableHeight() const { return m_DrawableAreaHeight; };

	void DeInit();

	ID3D11Device*              get_raw_device() const { return _device; };
	//ID3D11DeviceContext*       get_raw_device_context() const { return m_DeviceCtx; };
	GraphicsResourceHandle const& get_raw_output_rtv() const { return m_OutputTexture.GetRTV(); };
	GraphicsResourceHandle const& get_raw_output_tex() const { return m_OutputTexture.GetResource(); };
	GraphicsResourceHandle const& get_raw_output_non_msaa_tex() const { return _non_msaa_output_tex; };
	GraphicsResourceHandle const& get_raw_output_non_msaa_srv() const { return _non_msaa_output_srv; };

	SwapchainHandle				  get_raw_swapchain() const { return _swapchain; };

	IWICImagingFactory*        get_raw_wic() const { return _wic_factory; };
#if FEATURE_D2D
	ID2D1Factory*              get_raw_d2d_factory() const { return _d2d_factory; }
	ID2D1RenderTarget*         get_2d_draw_ctx() const { return _d2d_rt; }
	ID2D1SolidColorBrush*      get_2d_color_brush() const { return _color_brush; }
#endif
	IDWriteFactory*            get_raw_dwrite_factory() const { return _dwrite_factory; }
	DXGI_FORMAT get_swapchain_format() const { return _swapchain_format; }


	void UpdateViewport(u32 x, u32 y, u32 w, u32 h)
	{
		m_ViewportPos.x = float(x);
		m_ViewportPos.y = float(y);
		m_ViewportWidth = w;
		m_ViewportHeight = h;
	}
	// Respond to external swapchain change requests
	void ResizeSwapchain(u32 w, u32 h);

	void PreRender(RenderContext& ctx, RenderWorld const& world);

	// Rendering passes and commands
    void DrawView(RenderContext& ctx, RenderWorld const& world, RenderPass::Value pass);
    void DrawWorld(RenderContext& ctx, RenderWorld const& world, ViewParams const& params);

	void PrepareShadowPass();

	// Call at start of frame to setup default state
	void BeginFrame(RenderContext& ctx);

	// Call at end to make sure the required systems are flushed
	void EndFrame(RenderContext& ctx);

	// Renders the shadow pass for this world. (Currently just CSM)
    void DrawShadowPass(RenderContext& ctx, RenderWorld const& world);

	// Renders a zprepass for the world 
	void DrawZPrePass(RenderContext& ctx, RenderWorld const& world);

	// Renders the opaque pass 
	void DrawOpaquePass(RenderContext& ctx, RenderWorld const& world);

	// Renders the final post processing pass. For now we also render ImGui and other debug code in here.
    void DrawPost(RenderContext& ctx, RenderWorld const& world, shared_ptr<OverlayManager> const& overlays, bool doImgui = true);

	// Copies the last rendered main frustum depth to be used as input during the main pass
	void CopyDepth(RenderContext& ctx);

private:
	void release_frame_resources();
	void release_device_resources();

	Math::Frustum get_frustum_world(RenderWorld const& world, u32 cam) const;

	Math::Frustum get_cascade_frustum(shared_ptr<RenderWorldCamera> const& camera, u32 cascade, u32 num_cascades) const;

	// Helper to setup the render state based on material
    void setup_renderstate(RenderContext& ctx, VertexLayoutFlags flags, MaterialInstance const* material, ViewParams const& params);

	// Rendering
	void render_post_predebug(RenderContext& ctx);
	void render_post_postdebug(RenderContext& ctx);

	struct PerfStats
	{
		u32 n_draws;
		u32 n_primitives;
	};
	PerfStats const& get_stats() const { return m_FrameStats; }

private:
	PerfStats m_FrameStats;

	// 0: Game camera | 1: Debug camera
	u32 _active_cam = 0;
	EngineCfg _engine_settings;
	GameCfg _game_settings;

	MSAAMode _msaa;
	SDL_Window* m_Window;
	RenderInterface* m_RI;

	DXGI_FORMAT   _swapchain_format;

	SwapchainHandle          _swapchain;
	GraphicsResourceHandle   _swapchain_rtv;
	GraphicsResourceHandle   _swapchain_srv;

	ID3D11Device*        _device;
	//ID3D11DeviceContext* m_DeviceCtx;

	// Intermediate MSAA game output.
	// these textures get resolved to the swapchain before presenting
    Texture m_OutputTexture;
	GraphicsResourceHandle _non_msaa_output_tex;
	GraphicsResourceHandle _non_msaa_output_srv;
	GraphicsResourceHandle _non_msaa_output_tex_copy;
	GraphicsResourceHandle _non_msaa_output_srv_copy;

	GraphicsResourceHandle _non_msaa_output_rtv;
	GraphicsResourceHandle _output_depth;
	GraphicsResourceHandle _output_depth_copy;
	GraphicsResourceHandle _output_depth_srv;
	GraphicsResourceHandle _output_depth_srv_copy;
	GraphicsResourceHandle _output_dsv;

	// D2D resources to support D2D1 on D3D11
	ID2D1Factory* _d2d_factory;
	IWICImagingFactory* _wic_factory;
	ID2D1RenderTarget* _d2d_rt;
	IDWriteFactory* _dwrite_factory;
	ID2D1SolidColorBrush* _color_brush;
	DXGI_SAMPLE_DESC _aa_desc;
	D2D1_ANTIALIAS_MODE _d2d_aa_mode;

	// Shadow mapping
	GraphicsResourceHandle _shadow_map;
	GraphicsResourceHandle _shadow_map_dsv[MAX_CASCADES];
	GraphicsResourceHandle _shadow_map_srv;
	GraphicsResourceHandle _debug_shadow_map_srv[MAX_CASCADES];

	std::unique_ptr<GPUStructuredBuffer> _light_buffer;
	std::unique_ptr<GPUByteBuffer> _tile_light_index_buffer;
	std::unique_ptr<GPUByteBuffer> _per_tile_info_buffer;
	std::unique_ptr<ConstantBuffer> _fplus_cb;
	std::shared_ptr<Shader> _fplus_cull_shader;

	// Constant buffers
	ConstantBufferRef m_CBGlobal;
	ConstantBufferRef m_CBModel;
	ConstantBufferRef m_CBDebug;
	ConstantBufferRef m_CBPost;

	u32 _num_lights;
	u32 _num_directional_lights;
	u32 _num_tiles_x;

	// Cubemap
	ComPtr<ID3D11Texture2D> _cubemap;
	GraphicsResourceHandle _cubemap_srv;

	std::unique_ptr<class RendererDebugTool> _debug_tool;

	// Rendering parameters
	u32		m_ViewportWidth;
	u32		m_ViewportHeight;
	float2  m_ViewportPos;

	u32 m_DrawableAreaWidth;
	u32 m_DrawableAreaHeight;

	std::unique_ptr<DirectX::DX11::CommonStates> _states = nullptr;
	std::unique_ptr<DirectX::DX11::BasicEffect> _common_effect = nullptr;
	ComPtr<ID3D11InputLayout> _layout = nullptr;

	std::unique_ptr<class VisibilityManager> m_Visibility;

	std::vector<DrawCall> m_DrawCalls;

	// State tracking
	struct 
	{
		shared_ptr<Shader const> VS;
		shared_ptr<Shader const> PS;
		ID3D11InputLayout* InputLayout;
		ID3D11RasterizerState* RS;
	} m_PrevRenderState;

};

}
