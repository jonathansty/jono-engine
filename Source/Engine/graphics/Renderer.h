#pragma once

#include "Graphics.h"
#include "ConstantBuffer.h"
#include "EngineSettings.h"
#include "GameSettings.h"

#include "debug_overlays/OverlayManager.h"
#include "Core/Math.h"
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "RendererDebug.h"

#if FEATURE_D2D
#include "2DRenderContext.h"
#endif
#include <Source/Engine/graphics/2DRenderContext.h>

class RenderWorld;
class RenderWorldCamera;
class Material;
using ShaderRef = shared_ptr<class Shader>;

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
		Shadow,
		Post

	};

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
			case Value::Shadow:
				return "Shadow";
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
	D3D11_VIEWPORT viewport;
	RenderPass::Value pass;
	bool reverse_z = true;
};



// The max amount of lights available in the shaders
static constexpr u32 MAX_LIGHTS = 4;
static constexpr u32 MAX_CASCADES = 4;

__declspec(align(16)) 
struct LightInfo
{
	float4 colour;
	float4 direction;
	float4x4 light_space;

	u32 num_cascades;
	f32 padding0[3];

	float4x4 cascades[MAX_CASCADES];
	float4 cascade_distances[MAX_CASCADES];
};

__declspec(align(16)) 
struct AmbientInfo
{
	float4 ambient;
};

__declspec(align(16))
struct Viewport
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

	Viewport vp;
	AmbientInfo ambient;
	LightInfo lights[MAX_LIGHTS];

	u32 num_lights;
	float padding[3];
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

struct DeviceContext
{
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _ctx;
};


class Renderer
{
public:

	void init(EngineSettings const& settings, GameSettings const& game_settings, cli::CommandLine const& cmdline);

	void init_for_hwnd(HWND wnd);

	void deinit();

	DeviceContext get_ctx() const { return DeviceContext{ _device, _device_ctx }; }

	ID3D11Device*              get_raw_device() const { return _device; };
	ID3D11DeviceContext*       get_raw_device_context() const { return _device_ctx; };
	ID3D11RenderTargetView*    get_raw_swapchain_rtv() const { return _swapchain_rtv; };
	ID3D11RenderTargetView*    get_raw_output_rtv() const { return _output_rtv; };
	ID3D11ShaderResourceView*  get_raw_output_srv() const { return _output_srv; };
	ID3D11DepthStencilView*    get_raw_output_dsv() const { return _output_dsv; };
	ID3D11Texture2D*           get_raw_output_tex() const { return _output_tex; };
	ID3D11Texture2D*           get_raw_output_non_msaa_tex() const { return _non_msaa_output_tex; };
	ID3D11ShaderResourceView*  get_raw_output_non_msaa_srv() const { return _non_msaa_output_srv; };
	IDXGISwapChain3*           get_raw_swapchain() const { return _swapchain; };

	ID3DUserDefinedAnnotation* get_raw_annotation() const { return _user_defined_annotation; };
	IWICImagingFactory*        get_raw_wic() const { return _wic_factory; };
#if FEATURE_D2D
	ID2D1Factory*              get_raw_d2d_factory() const { return _d2d_factory; }
	ID2D1RenderTarget*         get_2d_draw_ctx() const { return _d2d_rt; }
	ID2D1SolidColorBrush*      get_2d_color_brush() const { return _color_brush; }
#endif
	IDWriteFactory*            get_raw_dwrite_factory() const { return _dwrite_factory; }
	DXGI_FORMAT get_swapchain_format() const { return _swapchain_format; }


	void update_viewport(u32 x, u32 y, u32 w, u32 h)
	{
		_viewport_pos.x = float(x);
		_viewport_pos.y = float(y);
		_viewport_width = w;
		_viewport_height = h;
	}
	// Respond to external swapchain change requests
	void resize_swapchain(u32 w, u32 h);

	void pre_render(shared_ptr<RenderWorld> const& world);

	// Rendering passes and commands
	void render_view(shared_ptr<RenderWorld> const& world, RenderPass::Value pass);
	void render_world(shared_ptr<RenderWorld> const& world, ViewParams const& params);


	void prepare_shadow_pass();

	// Call at start of frame to setup default state
	void begin_frame();

	// Call at end to make sure the required systems are flushed
	void end_frame();

	// Renders the shadow pass for this world. (Currently just CSM)
	void render_shadow_pass(shared_ptr<RenderWorld> const& world);

	// Renders a zprepass for the world 
	void render_zprepass(shared_ptr<RenderWorld> const& world);

	// Renders the opaque pass 
	void render_opaque_pass(shared_ptr<RenderWorld> const& world);

	// Renders the final post processing pass. For now we also render ImGui and other debug code in here.
	void render_post(shared_ptr<RenderWorld> const& world, shared_ptr<OverlayManager> const& overlays);

	// Copies the last rendered main frustum depth to be used as input during the main pass
	void copy_depth();

private:
	void create_factories(EngineSettings const& settings, cli::CommandLine const& cmdline);
	void create_d2d_factory(EngineSettings const& settings);
	void create_wic_factory();
	void create_write_factory();

	FrustumCorners get_frustum_world(shared_ptr<RenderWorld> const& world, u32 cam) const;

	FrustumCorners get_cascade_frustum(shared_ptr<RenderWorldCamera> const& camera, u32 cascade, u32 num_cascades) const;

	void setup_renderstate(ViewParams const& params, Material* const material);

	void VSSetShader(ShaderRef const& vertex_shader);
	void PSSetShader(ShaderRef const& pixel_hader);

	void render_post_predebug();
	void render_post_postdebug();

private:
	// 0: Game camera | 1: Debug camera
	u32 _active_cam = 0;
	EngineSettings _engine_settings;
	GameSettings _game_settings;

	MSAAMode _msaa;
	HWND _wnd;

	DXGI_FORMAT   _swapchain_format;
	IDXGIFactory* _factory;

	IDXGISwapChain3*          _swapchain;
	ID3D11RenderTargetView*   _swapchain_rtv;
	ID3D11ShaderResourceView* _swapchain_srv;

	ID3D11Device*        _device;
	ID3D11DeviceContext* _device_ctx;

	ID3DUserDefinedAnnotation* _user_defined_annotation;

	// Intermediate MSAA game output.
	// these textures get resolved to the swapchain before presenting
	ID3D11Texture2D*          _output_tex;
	ID3D11ShaderResourceView* _output_srv;
	ID3D11Texture2D*          _non_msaa_output_tex;
	ID3D11ShaderResourceView* _non_msaa_output_srv;
	ID3D11Texture2D*	      _non_msaa_output_tex_copy;
	ID3D11ShaderResourceView* _non_msaa_output_srv_copy;

	ID3D11RenderTargetView*	  _non_msaa_output_rtv;
	ID3D11RenderTargetView*   _output_rtv;
	ID3D11Texture2D*          _output_depth;
	ID3D11Texture2D*          _output_depth_copy;
	ID3D11ShaderResourceView* _output_depth_srv;
	ID3D11ShaderResourceView* _output_depth_srv_copy;
	ID3D11DepthStencilView*   _output_dsv;

	// D2D resources to support D2D1 on D3D11
	ID2D1Factory* _d2d_factory;
	IWICImagingFactory* _wic_factory;
	ID2D1RenderTarget* _d2d_rt;
	IDWriteFactory* _dwrite_factory;
	ID2D1SolidColorBrush* _color_brush;
	DXGI_SAMPLE_DESC _aa_desc;
	D2D1_ANTIALIAS_MODE _d2d_aa_mode;

	ComPtr<ID3D11Texture2D> _shadow_map;
	ComPtr<ID3D11DepthStencilView> _shadow_map_dsv[MAX_CASCADES];
	ComPtr<ID3D11ShaderResourceView> _shadow_map_srv;
	ComPtr<ID3D11ShaderResourceView> _debug_shadow_map_srv[MAX_CASCADES];

	ConstantBufferRef _cb_global;
	ConstantBufferRef _cb_debug;
	ConstantBufferRef _cb_post;

	std::unique_ptr<class RendererDebugTool> _debug_tool;
	friend class RendererDebugTool;	

	// Rendering parameters
	u32 _viewport_width;
	u32 _viewport_height;
	float2 _viewport_pos;
};

}
