#include "Renderer.h"
#include "engine.pch.h"

#include "CommandLine.h"
#include "Core/Material.h"
#include "Core/MaterialResource.h"
#include "Graphics/ShaderCache.h"
#include "Logging.h"
#include "RenderWorld.h"

#include "GameEngine.h"

#include "Debug.h"
#include "RendererDebug.h"

#include "CommonStates.h"
#include "Effects.h"
#include "ShaderCache.h"
#include "ShaderType.h"

namespace Graphics
{

static f32 s_box_size = 15.0f;

void Renderer::init(EngineSettings const& settings, GameSettings const& game_settings, cli::CommandLine const& cmdline)
{
	_msaa = settings.d3d_msaa_mode;
	_engine_settings = settings;
	_game_settings = game_settings;

	create_factories(settings, cmdline);

	_debug_tool = std::make_unique<RendererDebugTool>(this);
	GameEngine::instance()->get_overlay_manager()->register_overlay(_debug_tool.get());

	_cb_global = ConstantBuffer::create(_device, sizeof(GlobalCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
	_cb_debug = ConstantBuffer::create(_device, sizeof(DebugCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
	_cb_post = ConstantBuffer::create(_device, sizeof(PostCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);

}

void Renderer::init_for_hwnd(HWND wnd)
{
	assert(!_wnd);
	_wnd = wnd;

	RECT r{};
	if (::GetWindowRect(wnd, &r))
	{
		u32 w = r.right - r.left;
		u32 h = r.bottom - r.top;

		// Initially start out with the window as our viewport. This will most likely get
		// resized when ImGui creates our viewport control
		_viewport_width = w;
		_viewport_height = h;
		_viewport_pos = { 0, 0 };

		resize_swapchain(w, h);
	}

	if (_d2d_rt)
	{
		// set alias mode
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);

		// Create a brush.
		_d2d_rt->CreateSolidColorBrush((D2D1::ColorF)D2D1::ColorF::Black, &_color_brush);
	}
}

void Renderer::deinit()
{
	GameEngine::instance()->get_overlay_manager()->unregister_overlay(_debug_tool.get());

	using helpers::SafeRelease;

	SafeRelease(_color_brush);
	SafeRelease(_d2d_rt);

	SafeRelease(_swapchain);
	SafeRelease(_swapchain_rtv);
	SafeRelease(_swapchain_srv);

	SafeRelease(_output_tex);
	SafeRelease(_output_rtv);
	SafeRelease(_output_srv);
	SafeRelease(_output_depth);
	SafeRelease(_output_depth_srv);
	SafeRelease(_output_depth_srv_copy);
	SafeRelease(_output_depth_copy);
	SafeRelease(_output_dsv);
	SafeRelease(_user_defined_annotation);

	SafeRelease(_device_ctx);
	SafeRelease(_device);
	SafeRelease(_dwrite_factory);
	SafeRelease(_wic_factory);
	SafeRelease(_d2d_factory);
	SafeRelease(_factory);
}

void Renderer::create_factories(EngineSettings const& settings, cli::CommandLine const& cmdline)
{
	// Create Direct3D 11 factory
	{
		ENSURE_HR(CreateDXGIFactory(IID_PPV_ARGS(&_factory)));
		helpers::SetDebugObjectName(_factory, "Main DXGI Factory");

		// Define the ordering of feature levels that Direct3D attempts to create.
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
		};
		D3D_FEATURE_LEVEL featureLevel;

		uint32_t creation_flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		bool debug_layer = cli::has_arg(cmdline, "-enable-d3d-debug");
		if (debug_layer)
		{
			creation_flag |= D3D11_CREATE_DEVICE_DEBUG;
			debug_layer = true;
		}

		ENSURE_HR(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flag, featureLevels, UINT(std::size(featureLevels)), D3D11_SDK_VERSION, &_device, &featureLevel, &_device_ctx));

		if (debug_layer)
		{
			bool do_breaks = cli::has_arg(cmdline, "-d3d-break");
			ComPtr<ID3D11InfoQueue> info_queue;
			_device->QueryInterface(IID_PPV_ARGS(&info_queue));
			if (info_queue)
			{
				if (do_breaks)
				{
					info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
					info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
				}

				// D3D11_INFO_QUEUE_FILTER f{};
				// f.DenyList.NumSeverities = 1;
				// D3D11_MESSAGE_SEVERITY severities[1] = {
				//	D3D11_MESSAGE_SEVERITY_WARNING
				// };
				// f.DenyList.pSeverityList = severities;
				// info_queue->AddStorageFilterEntries(&f);
			}
		}
	}

#if FEATURE_D2D
	create_d2d_factory(settings);
#endif

	create_wic_factory();
	create_write_factory();

	ENSURE_HR(_device_ctx->QueryInterface(IID_PPV_ARGS(&_user_defined_annotation)));
}

#if FEATURE_D2D
void Renderer::create_d2d_factory(EngineSettings const& settings)
{
	if (settings.d2d_use)
	{
		HRESULT hr;
		// Create a Direct2D factory.
		ID2D1Factory* localD2DFactoryPtr = nullptr;
		if (!_d2d_factory)
		{
			D2D1_FACTORY_OPTIONS options;
			options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
			hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, options, &localD2DFactoryPtr);
			if (FAILED(hr))
			{
				FAILMSG("Create D2D Factory Failed");
				exit(-1);
			}
			_d2d_factory = localD2DFactoryPtr;
		}
	}
}
#endif

void Renderer::create_wic_factory()
{
	HRESULT hr;
	// Create a WIC factory if it does not exists
	IWICImagingFactory* localWICFactoryPtr = nullptr;
	if (!_wic_factory)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&localWICFactoryPtr));
		if (FAILED(hr))
		{
			FAILMSG("Create WIC Factory Failed");
			exit(-1);
		}
		_wic_factory = localWICFactoryPtr;
	}
}

void Renderer::create_write_factory()
{
	HRESULT hr;
	// Create a DirectWrite factory.
	IDWriteFactory* localDWriteFactoryPtr = nullptr;
	if (!_dwrite_factory)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(localDWriteFactoryPtr), reinterpret_cast<IUnknown**>(&localDWriteFactoryPtr));
		if (FAILED(hr))
		{
			FAILMSG("Create WRITE Factory Failed");
			exit(-1);
		}
		_dwrite_factory = localDWriteFactoryPtr;
	}
}

void Renderer::resize_swapchain(u32 w, u32 h)
{
	assert(_wnd);

	LOG_VERBOSE(Graphics, "Resizing swapchain to {}x{}", w, h);
	DXGI_FORMAT swapchain_format = DXGI_FORMAT_B8G8R8A8_UNORM;
	_swapchain_format = swapchain_format;

	// Create MSAA render target that resolves to non-msaa swapchain
	DXGI_SAMPLE_DESC aa_desc{};
	switch (_msaa)
	{
		case MSAAMode::Off:
			aa_desc.Count = 1;
			break;
		case MSAAMode::MSAA_2x:
			aa_desc.Count = 2;
			break;
		case MSAAMode::MSAA_4x:
			aa_desc.Count = 4;
			break;
		default:
			break;
	}

	UINT qualityLevels;
	_device->CheckMultisampleQualityLevels(swapchain_format, _aa_desc.Count, &qualityLevels);
	aa_desc.Quality = (_msaa != MSAAMode::Off) ? qualityLevels - 1 : 0;

	// Release the textures before re-creating the swapchain
	if (_swapchain)
	{
		helpers::SafeRelease(_non_msaa_output_tex);
		helpers::SafeRelease(_non_msaa_output_tex_copy);
		helpers::SafeRelease(_non_msaa_output_srv);
		helpers::SafeRelease(_non_msaa_output_srv_copy);
		helpers::SafeRelease(_non_msaa_output_rtv);
		helpers::SafeRelease(_output_tex);
		helpers::SafeRelease(_output_rtv);
		helpers::SafeRelease(_output_srv);

		helpers::SafeRelease(_output_depth);
		helpers::SafeRelease(_output_dsv);
		helpers::SafeRelease(_swapchain_rtv);
		helpers::SafeRelease(_swapchain_srv);
		helpers::SafeRelease(_d2d_rt);
	}

	// Create the 3D output target
	{
		auto output_desc = CD3D11_TEXTURE2D_DESC(
				swapchain_format,
				w,
				h,
				1,
				1,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
		ENSURE_HR(_device->CreateTexture2D(&output_desc, nullptr, &_output_tex));
		ENSURE_HR(_device->CreateRenderTargetView(_output_tex, nullptr, &_output_rtv));
		ENSURE_HR(_device->CreateShaderResourceView(_output_tex, nullptr, &_output_srv));

		// This texture is used to output to a image in imgui
		output_desc.SampleDesc.Count = 1;
		output_desc.SampleDesc.Quality = 0;
		ENSURE_HR(_device->CreateTexture2D(&output_desc, nullptr, &_non_msaa_output_tex));
		ENSURE_HR(_device->CreateTexture2D(&output_desc, nullptr, &_non_msaa_output_tex_copy));
		ENSURE_HR(_device->CreateShaderResourceView(_non_msaa_output_tex, nullptr, &_non_msaa_output_srv));
		ENSURE_HR(_device->CreateShaderResourceView(_non_msaa_output_tex_copy, nullptr, &_non_msaa_output_srv_copy));
		ENSURE_HR(_device->CreateRenderTargetView(_non_msaa_output_tex, nullptr, &_non_msaa_output_rtv));
	}

	// Create the 3D depth target
	auto depth_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16_TYPELESS, w, h, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
	ENSURE_HR(_device->CreateTexture2D(&depth_desc, nullptr, &_output_depth));
	ENSURE_HR(_device->CreateTexture2D(&depth_desc, nullptr, &_output_depth_copy));

	auto dsv_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(_output_depth, D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D16_UNORM);
	ENSURE_HR(_device->CreateDepthStencilView(_output_depth, &dsv_desc, &_output_dsv));
	auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(_output_depth, D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R16_UNORM);
	ENSURE_HR(_device->CreateShaderResourceView(_output_depth, &srv_desc, &_output_depth_srv));
	ENSURE_HR(_device->CreateShaderResourceView(_output_depth_copy, &srv_desc, &_output_depth_srv_copy));

	set_debug_name(_output_tex, "Color Output");
	set_debug_name(_output_depth, "Depth Output");

	// Either create the swapchain or retrieve the existing description
	DXGI_SWAP_CHAIN_DESC desc{};
	if (_swapchain)
	{
		_swapchain->GetDesc(&desc);
		_swapchain->ResizeBuffers(desc.BufferCount, w, h, desc.BufferDesc.Format, desc.Flags);
	}
	else
	{
		desc.BufferDesc.Width = w;
		desc.BufferDesc.Height = h;
		desc.BufferDesc.Format = swapchain_format;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		desc.OutputWindow = _wnd;
		if (_game_settings.m_FullscreenMode == GameSettings::FullScreenMode::Windowed || _game_settings.m_FullscreenMode == GameSettings::FullScreenMode::BorderlessWindowed)
		{
			desc.Windowed = TRUE;
		}
		else
		{
			desc.Windowed = false;
		}
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		ComPtr<IDXGISwapChain> swapchain;
		SUCCEEDED(_factory->CreateSwapChain(_device, &desc, &swapchain));
		swapchain->QueryInterface(IID_PPV_ARGS(&_swapchain));

		set_debug_name(_swapchain, "DXGISwapchain");
		set_debug_name(_factory, "DXGIFactory");
	}

	// Recreate the views
	ComPtr<ID3D11Texture2D> backBuffer;
	_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	assert(backBuffer);
	SUCCEEDED(_device->CreateRenderTargetView(backBuffer.Get(), NULL, &_swapchain_rtv));
	SUCCEEDED(_device->CreateShaderResourceView(backBuffer.Get(), NULL, &_swapchain_srv));
	set_debug_name(backBuffer.Get(), "Swapchain::Output");

	// Create the D2D target for 2D rendering
	UINT dpi = GetDpiForWindow(_wnd);
	D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(swapchain_format, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

	ComPtr<IDXGISurface> surface;
	_output_tex->QueryInterface(surface.GetAddressOf());

	if (_d2d_factory)
	{
		SUCCEEDED(_d2d_factory->CreateDxgiSurfaceRenderTarget(surface.Get(), rtp, &_d2d_rt));
		set_debug_name(surface.Get(), "[D2D] Output");

		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
}

void Renderer::pre_render(shared_ptr<RenderWorld> const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"PreRender");
	extern int g_DebugMode;
	DebugCB* debug_data = (DebugCB*)_cb_debug->map(_device_ctx);
	debug_data->m_VisualizeMode = g_DebugMode;
	_cb_debug->unmap(_device_ctx);

	for (auto& cam : world->get_cameras())
	{
		// Update our view camera to properly match the viewport aspect
		cam->set_aspect((f32)_viewport_width / (f32)_viewport_height);
		cam->update();
	}
}

void Renderer::render_view(shared_ptr<RenderWorld> const& world, RenderPass::Value pass)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Renderer::render_view");

	// Setup global constant buffer
	std::shared_ptr<RenderWorldCamera> camera = world->get_view_camera();

	ViewParams params{};
	D3D11_TEXTURE2D_DESC desc;
	_output_tex->GetDesc(&desc);

	params.proj = camera->get_proj();
	params.view = camera->get_view();
	params.view_position = camera->get_position();

	params.view_direction = camera->get_view_direction().xyz;
	params.pass = pass;

	// Viewport depends on the actual imgui window
	params.viewport = CD3D11_VIEWPORT(_output_tex, _output_rtv);

	// For the world we always render from the top left corner
	params.viewport.TopLeftX = 0.0;
	params.viewport.TopLeftY = 0.0;
	params.viewport.Width = (f32)_viewport_width;
	params.viewport.Height = (f32)_viewport_height;

	render_world(world, params);
}

void Renderer::render_world(shared_ptr<RenderWorld> const& world, ViewParams const& params)
{
	std::string passName = RenderPass::ToString(params.pass);
	GPU_SCOPED_EVENT(_user_defined_annotation, passName);

	// Setup some defaults. At the moment these are applied for each pass. However
	// ideally we would be able to have more detailed logic here to decided based on pass and mesh/material
	{
		DepthStencilState depth_stencil_state = DepthStencilState::Equal;
		if (params.pass == RenderPass::ZPrePass || params.pass == RenderPass::Shadow)
		{
			depth_stencil_state = DepthStencilState::LessEqual;
		}

		ID3D11SamplerState* samplers[2] = {
			Graphics::get_sampler_state(SamplerState::MinMagMip_Linear).Get(),
			Graphics::get_sampler_state(SamplerState::MinMagMip_Point).Get()
		};
		_device_ctx->PSSetSamplers(0, UINT(std::size(samplers)), samplers);

		_device_ctx->OMSetDepthStencilState(Graphics::get_depth_stencil_state(depth_stencil_state).Get(), 0);
		_device_ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullBack).Get());
		_device_ctx->OMSetBlendState(Graphics::get_blend_state(BlendState::Default).Get(), NULL, 0xffffffff);
		_device_ctx->RSSetViewports(1, &params.viewport);
	}

	GlobalCB* global = (GlobalCB*)_cb_global->map(_device_ctx);
	global->ambient.ambient = float4(0.02f, 0.02f, 0.02f, 1.0f);

	global->proj = params.proj;
	global->view = params.view;
	global->inv_view = hlslpp::inverse(params.view);
	global->inv_proj = hlslpp::inverse(params.proj);
	global->inv_view_projection = hlslpp::inverse(hlslpp::mul(params.view, params.proj));
	global->view_direction = float4(params.view_direction.xyz, 0.0f);

	global->view_pos = float4(params.view_position, 1.0f);
	global->vp.vp_top_x = params.viewport.TopLeftX;
	global->vp.vp_top_y = params.viewport.TopLeftY;
	global->vp.vp_half_width = params.viewport.Width / 2.0f;
	global->vp.vp_half_height = params.viewport.Height / 2.0f;

	RenderWorld::LightCollection const& lights = world->get_lights();
	global->num_lights = std::min<u32>(u32(lights.size()), MAX_LIGHTS);
	for (u32 i = 0; i < global->num_lights; ++i)
	{
		LightInfo* info = global->lights + i;
		shared_ptr<RenderWorldLight> l = lights[i];
		if (l->is_directional())
		{
			info->direction = float4(l->get_view_direction().xyz, 0.0f);
			info->colour = float4(l->get_colour(), 1.0f);

			// Each directional light could have N cascades so we need to store orthographic projects for each light
			// CascadeInfo const& cascade_info = l->get_cascade(0);
			info->light_space = float4x4::identity();

			info->num_cascades = MAX_CASCADES;
			for (int j = 0; j < MAX_CASCADES; ++j)
			{
				f32 n = world->get_camera(0)->get_near();
				f32 f = world->get_camera(0)->get_far();

				f32 z0 = n * pow(f / n, f32(j) / f32(MAX_CASCADES));
				f32 z1 = n * pow(f / n, f32(j + 1) / f32(MAX_CASCADES));
				info->cascade_distances[j] = z1;
				info->cascades[j] = l->get_cascade(j).vp;
			}
		}
	}

	_cb_global->unmap(_device_ctx);

	float4x4 vp = hlslpp::mul(params.view, params.proj);

	RenderWorld::InstanceCollection const& instances = world->get_instances();
	for (std::shared_ptr<RenderWorldInstance> const& inst : instances)
	{
		// A render world instance is ready when the whole model (and it's dependencies) has been loaded
		if (!inst->is_ready())
		{
			continue;
		}

		auto ctx = _device_ctx;

		Model const* model = inst->_model->get();
		ConstantBufferRef const& model_cb = inst->_model_cb;

		RenderWorldInstance::ConstantBufferData* data = (RenderWorldInstance::ConstantBufferData*)model_cb->map(_device_ctx);
		data->world = inst->_transform;
		data->wv = hlslpp::mul(data->world, params.view);
		data->wvp = hlslpp::mul(data->world, vp);
		model_cb->unmap(ctx);

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetIndexBuffer(model->get_index_buffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		UINT strides = { sizeof(Model::VertexType) };
		UINT offsets = { 0 };
		ctx->IASetVertexBuffers(0, 1, model->get_vertex_buffer().GetAddressOf(), &strides, &offsets);

		// #TODO: Remove debug handling into a debug lighting system
		extern int g_DebugMode;
		if (g_DebugMode != 0)
		{
			ID3D11Buffer* buffers[3] = {
				_cb_global->Get(),
				_cb_debug->Get(),
				model_cb->Get(),
			};
			ctx->VSSetConstantBuffers(0, 3, buffers);
			ctx->PSSetConstantBuffers(0, 3, buffers);
		}
		else
		{
			ID3D11Buffer* buffers[1] = {
				_cb_global->Get(),
			};
			ctx->VSSetConstantBuffers(0, 1, buffers);
			ctx->PSSetConstantBuffers(0, 1, buffers);

			buffers[0] = model_cb->Get();
			ctx->VSSetConstantBuffers(2, 1, buffers);
			ctx->PSSetConstantBuffers(2, 1, buffers);
		}

		for (Mesh const& mesh : model->get_meshes())
		{
			MaterialRef const& material_res = model->get_material(mesh.material_index);

			ASSERTMSG(material_res->is_loaded(), "Material resource for the model hasn't been loaded yet! Make sure the model load check is correct.");
			Material* material = material_res->get();

			setup_renderstate(params, material);

			ctx->DrawIndexed((UINT)mesh.indexCount, (UINT)mesh.firstIndex, (INT)mesh.firstVertex);

			ID3D11ShaderResourceView* views[] = {
				nullptr,
				nullptr
			};

			_device_ctx->PSSetShaderResources(3, UINT(std::size(views)), views);
		}

		if (params.pass == RenderPass::Opaque)
		{
			// unbind shader resource
			ID3D11ShaderResourceView* views[] = { nullptr };
			_device_ctx->PSSetShaderResources(3, 1, views);
		}
	}
}

void Renderer::prepare_shadow_pass()
{
	if (!_shadow_map)
	{
		u32 num_cascades = MAX_CASCADES;
		auto res_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, 2048, 2048, num_cascades);
		res_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		res_desc.MipLevels = 1;
		if (FAILED(_device->CreateTexture2D(&res_desc, NULL, _shadow_map.ReleaseAndGetAddressOf())))
		{
			ASSERTMSG(false, "Failed to create the shadowmap texture");
		}

		for (u32 i = 0; i < num_cascades; ++i)
		{
			auto view_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(_shadow_map.Get(), D3D11_DSV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_D32_FLOAT, 0, i);
			if (FAILED(_device->CreateDepthStencilView(_shadow_map.Get(), &view_desc, _shadow_map_dsv[i].ReleaseAndGetAddressOf())))
			{
				ASSERTMSG(false, "Failed to create the shadowmap DSV");
			}

			auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(_shadow_map.Get(), D3D11_SRV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32_FLOAT, 0, UINT(-1), i, 1);
			if (FAILED(_device->CreateShaderResourceView(_shadow_map.Get(), &srv_desc, _debug_shadow_map_srv[i].ReleaseAndGetAddressOf())))
			{
				ASSERTMSG(false, "Failed to create the shadowmap SRV");
			}
		}

		auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(_shadow_map.Get(), D3D11_SRV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32_FLOAT);
		if (FAILED(_device->CreateShaderResourceView(_shadow_map.Get(), &srv_desc, _shadow_map_srv.ReleaseAndGetAddressOf())))
		{
			ASSERTMSG(false, "Failed to create the shadowmap SRV");
		}
	}
}

void Renderer::begin_frame()
{
	GameEngine* engine = GameEngine::instance();
	D3D11_VIEWPORT vp{};
	vp.Width = static_cast<float>(engine->get_viewport_size().x);
	vp.Height = static_cast<float>(engine->get_viewport_size().y);
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	_device_ctx->RSSetViewports(1, &vp);
}

void Renderer::end_frame()
{
}

FrustumCorners Renderer::get_frustum_world(shared_ptr<RenderWorld> const& world, u32 cam) const
{
	shared_ptr<RenderWorldCamera> camera = world->get_camera(cam);

	// Calculates the frustum straight from camera view proj
	FrustumCorners world_corners = {};
	Math::calculate_frustum(world_corners, hlslpp::inverse(camera->get_vp()));
	return world_corners;
}

Graphics::FrustumCorners Renderer::get_cascade_frustum(shared_ptr<RenderWorldCamera> const& camera, u32 cascade, u32 num_cascades) const
{
	f32 n = camera->get_near();
	f32 f = camera->get_far();

	f32 z0 = n * pow(f / n, f32(cascade) / f32(num_cascades));
	f32 z1 = n * pow(f / n, f32(cascade + 1) / f32(num_cascades));
	FrustumCorners world_corners = {};
	Math::calculate_frustum(world_corners, z0, z1, camera->get_fov(), camera->get_vertical_fov());

	float4x4 view = camera->get_view();
	Math::transform_frustum(world_corners, hlslpp::inverse(view));
	return world_corners;
}

void Renderer::setup_renderstate(ViewParams const& params, Material* const material)
{
	auto ctx = _device_ctx;

	// Bind anything coming from the material
	{
		if (material->is_double_sided())
		{
			ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullNone).Get());
		}
		else
		{
			ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullBack).Get());
		}

		auto vertex_shader = material->get_vertex_shader();
		auto pixel_shader = material->get_pixel_shader();
		auto debug_shader = material->get_debug_pixel_shader();
		if (!vertex_shader->is_valid())
		{
			vertex_shader = Graphics::get_error_shader_vx();
		}

		if (!pixel_shader->is_valid())
		{
			pixel_shader = Graphics::get_error_shader_px();
		}

		if (!debug_shader->is_valid())
		{
			debug_shader = Graphics::get_error_shader_px();
		}

		ctx->VSSetShader(vertex_shader->as<ID3D11VertexShader>().Get(), nullptr, 0);
		ctx->IASetInputLayout(vertex_shader->get_input_layout().Get());

		if (params.pass == RenderPass::Opaque)
		{
			ctx->PSSetShader(pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
			extern int g_DebugMode;
			if (g_DebugMode)
			{
				ctx->PSSetShader(debug_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
			}

			// Bind material parameters
			std::vector<ID3D11ShaderResourceView const*> views{};
			material->get_texture_views(views);
			ctx->PSSetShaderResources(0, (UINT)views.size(), (ID3D11ShaderResourceView**)views.data());
		}
		else
		{
			ctx->PSSetShader(nullptr, nullptr, 0);
		}
	}

	// Bind the global textures coming from rendering systems
	if (params.pass == RenderPass::Opaque)
	{
		ID3D11ShaderResourceView* views[] = {
			_shadow_map_srv.Get(),
			_output_depth_srv_copy
		};
		_device_ctx->PSSetShaderResources(3, UINT(std::size(views)), views);
	}
}

void Renderer::VSSetShader(ShaderRef const& vertex_shader)
{
	ASSERT(vertex_shader->get_type() == ShaderType::Vertex);
	_device_ctx->VSSetShader(vertex_shader->as<ID3D11VertexShader>().Get(), nullptr, 0);
}

void Renderer::PSSetShader(ShaderRef const& pixel_shader)
{
	ASSERT(pixel_shader->get_type() == ShaderType::Pixel);
	_device_ctx->PSSetShader(pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
}

void Renderer::render_post_predebug()
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Post:PreDebug");

	ShaderCreateParams params = ShaderCreateParams::pixel_shader("Source/Engine/Shaders/default_post_px.hlsl");
	ShaderRef post_shader = ShaderCache::instance()->find_or_create(params);

	params = ShaderCreateParams::vertex_shader("Source/Engine/Shaders/default_post_vx.hlsl");
	ShaderRef post_vs_shader = ShaderCache::instance()->find_or_create(params);

	_device_ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN,0);
	_device_ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	_device_ctx->IASetInputLayout(post_vs_shader->get_input_layout().Get());
	_device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	VSSetShader(post_vs_shader);
	PSSetShader(post_shader);
	_device_ctx->PSSetShaderResources(0, 1, &_non_msaa_output_srv_copy);
	ID3D11Buffer* buffer = _cb_post->Get();
	_device_ctx->PSSetConstantBuffers(0, 1, &buffer);

	ID3D11SamplerState* sampler_states[] = {
		Graphics::get_sampler_state(SamplerState::MinMagMip_Linear).Get(),
		Graphics::get_sampler_state(SamplerState::MinMagMip_Point).Get()
	};
	_device_ctx->PSSetSamplers(0, 2, sampler_states);
	_device_ctx->Draw(3, 0);

}

void Renderer::render_post_postdebug()
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Post:PostDebug");


}

void Renderer::render_shadow_pass(shared_ptr<RenderWorld> const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Shadows");

	prepare_shadow_pass();

	shared_ptr<RenderWorldLight> light = nullptr;

	// Retrieve the first directional light in the render world
	auto it = std::find_if(world->get_lights().begin(), world->get_lights().end(), [](auto const& light){ return light->is_directional() && light->get_casts_shadow(); });
	if(it != world->get_lights().end())
	{
		light = *it;
	}

	float4x4 light_space = light->get_view();
	if (light->get_casts_shadow())
	{
		float4x4 view = world->get_light(0)->get_view();
		float3 direction = world->get_light(0)->get_view_direction().xyz;
		float3 position = world->get_light(0)->get_position().xyz;

		std::vector<CascadeInfo> matrices;
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{
			FrustumCorners box_light = get_cascade_frustum(world->get_camera(0), i, MAX_CASCADES);

			// Transform to light view space
			Math::transform_frustum(box_light, light_space);

			// Calculate the extents(in light  space)
			float4 min_extents = box_light[0];
			float4 max_extents = box_light[0];

			for (u32 j = 1; j < box_light.size(); ++j)
			{
				float4 pos_light = box_light[j];
				min_extents = hlslpp::min(pos_light, min_extents);
				max_extents = hlslpp::max(pos_light, max_extents);
			}

			float3 center = ((min_extents + max_extents) / 2.0f).xyz;
			float3 extents = ((max_extents - min_extents) / 2.0f).xyz;

			XMFLOAT3 xm_center;
			XMFLOAT3 xm_extents;
			hlslpp::store(center, (float*)&xm_center);
			hlslpp::store(extents, (float*)&xm_extents);
			auto box = DirectX::BoundingBox(xm_center, xm_extents);

			f32 z_range = max_extents.z - min_extents.z;

			center = hlslpp::mul(float4(center, 1.0), hlslpp::inverse(light_space)).xyz;
			float3 new_center = center - direction * z_range;
			float4x4 new_projection = float4x4::orthographic(hlslpp::projection(hlslpp::frustum(-box.Extents.x, box.Extents.x, -box.Extents.y, box.Extents.y, 0.0f, z_range * 1.5f), hlslpp::zclip::zero));
			float4x4 new_view = float4x4::look_at(new_center, center, float3{ 0.0f, 1.0f, 0.0f });

			matrices.push_back({ center, new_view, new_projection, hlslpp::mul(new_view, new_projection) });
		}

		light->update_cascades(matrices);

		// Render out the cascades shadow map for the directional light
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{
			GPU_SCOPED_EVENT(_user_defined_annotation, fmt::format("Cascade {}", i));
			CascadeInfo const& info = light->get_cascade(i);

			_device_ctx->OMSetRenderTargets(0, nullptr, _shadow_map_dsv[i].Get());
			_device_ctx->ClearDepthStencilView(_shadow_map_dsv[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

			ViewParams params{};
#if 0 
			params.proj = proj;
			params.view = light_view;
#else
			params.view = info.view;
			params.proj = info.proj;
#endif
			params.view_position = position;
			params.view_direction = direction;
			params.pass = RenderPass::Shadow;
			params.viewport = CD3D11_VIEWPORT(0.0f, 0.0f, 2048.0f, 2048.0f);
			render_world(world, params);
		}
	}
}

void Renderer::render_zprepass(shared_ptr<RenderWorld> const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"zprepass");

	// Clear the output targets
	FLOAT color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	_device_ctx->ClearRenderTargetView(_output_rtv, color);
	_device_ctx->ClearDepthStencilView(_output_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_device_ctx->OMSetRenderTargets(0, NULL, _output_dsv);

	render_view(world, RenderPass::ZPrePass);
}

void Renderer::render_opaque_pass(shared_ptr<RenderWorld> const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"opaque");

	// Do a copy to our dsv tex for sampling during the opaque pass
	{
		copy_depth();
	}

	_device_ctx->OMSetRenderTargets(1, &_output_rtv, _output_dsv);
	render_view(world, Graphics::RenderPass::Opaque);
}

void Renderer::render_post(shared_ptr<RenderWorld> const& world, shared_ptr<OverlayManager> const& overlays)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Post");

	PostCB* data = (PostCB*)_cb_post->map(_device_ctx);
	data->m_ViewportWidth = f32(GameEngine::instance()->get_width());
	data->m_ViewportHeight = f32(GameEngine::instance()->get_height());
	_cb_post->unmap(_device_ctx);

	static std::unique_ptr<DirectX::CommonStates> s_states = nullptr;
	static std::unique_ptr<DirectX::BasicEffect> s_effect = nullptr;
	static ComPtr<ID3D11InputLayout> s_layout = nullptr;
	if (!s_states)
	{
		s_states = std::make_unique<DirectX::CommonStates>(_device);
		s_effect = std::make_unique<DirectX::BasicEffect>(_device);
		s_effect->SetVertexColorEnabled(true);

		void const* shader_byte_code;
		size_t byte_code_length;
		s_effect->GetVertexShaderBytecode(&shader_byte_code, &byte_code_length);
		ENSURE_HR(_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements, DirectX::VertexPositionColor::InputElementCount, shader_byte_code, byte_code_length, s_layout.ReleaseAndGetAddressOf()));
	}

	_device_ctx->OMSetBlendState(s_states->Opaque(), nullptr, 0xFFFFFFFF);
	_device_ctx->OMSetDepthStencilState(s_states->DepthRead(), 0);
	_device_ctx->RSSetState(s_states->CullNone());

	_device_ctx->IASetInputLayout(s_layout.Get());

	float4x4 view = float4x4::identity();
	float4x4 proj = float4x4::identity();
	if(world->get_view_camera())
	{
		view = world->get_view_camera()->get_view();
		proj = world->get_view_camera()->get_proj();
	}
	XMMATRIX xm_view = DirectX::XMLoadFloat4x4((XMFLOAT4X4*)&view);
	XMMATRIX xm_proj = DirectX::XMLoadFloat4x4((XMFLOAT4X4*)&proj);
	s_effect->SetView(xm_view);
	s_effect->SetProjection(xm_proj);
	s_effect->Apply(_device_ctx);

	{
		GPU_SCOPED_EVENT(_user_defined_annotation, L"Post:RenderOverlays");
		overlays->render_3d(_device_ctx);
	}

		// Resolve msaa to non msaa for imgui render
	_device_ctx->ResolveSubresource(_non_msaa_output_tex, 0, _output_tex, 0, _swapchain_format);

	// Copy the non-msaa world render so we can sample from it in the post pass
	_device_ctx->CopyResource(_non_msaa_output_tex_copy, _non_msaa_output_tex);
	_device_ctx->OMSetRenderTargets(1, &_non_msaa_output_rtv, nullptr);
	render_post_predebug();

	render_post_postdebug();

	// Render main viewport ImGui
	{
		GPU_SCOPED_EVENT(_user_defined_annotation, L"ImGui");
		_device_ctx->OMSetRenderTargets(1, &_swapchain_rtv, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

}

void Renderer::copy_depth()
{
	_device_ctx->CopyResource(_output_depth_copy, _output_depth);
}

} // namespace Graphics
