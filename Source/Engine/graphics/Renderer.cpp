#include "engine.pch.h"
#include "Renderer.h"

#include "Core/Material.h"
#include "Core/MaterialResource.h"
#include "RenderWorld.h"
#include "Logging.h"
#include "CommandLine.h"

#include "GameEngine.h"

#include "Debug.h"

namespace Graphics
{

void Renderer::init(EngineSettings const& settings, GameSettings const& game_settings, cli::CommandLine const& cmdline)
{
	_msaa = settings.d3d_msaa_mode;
	_engine_settings = settings;
	_game_settings = game_settings;

	create_factories(settings, cmdline);

	_debug_tool = std::make_unique<class RendererDebugTool>(this);
	GameEngine::instance()->get_overlay_manager()->register_overlay(_debug_tool.get());

	_cb_global = ConstantBuffer::create(_device, sizeof(GlobalCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
	_cb_debug = ConstantBuffer::create(_device, sizeof(DebugCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
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
		}
#if defined(_DEBUG)
		else
		{
			creation_flag |= D3D11_CREATE_DEVICE_DEBUG;
			debug_layer = true;
		}
#endif

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
		helpers::SafeRelease(_non_msaa_output_srv);
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
		ENSURE_HR(_device->CreateShaderResourceView(_non_msaa_output_tex, nullptr, &_non_msaa_output_srv));
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

	for(auto& cam : world->get_cameras())
	{
		// Update our view camera to properly match the viewport aspect
		cam->set_aspect((f32)_viewport_width / (f32)_viewport_height);
		cam->update();
	}
}

void Renderer::render_view(shared_ptr<RenderWorld> const& world, RenderPass::Value pass)
{
	// Setup global constant buffer
	std::shared_ptr<RenderWorldCamera> camera = world->get_view_camera();

	ViewParams params{};
	D3D11_TEXTURE2D_DESC desc;
	_output_tex->GetDesc(&desc);

	params.proj = camera->get_proj();
	params.view = camera->get_view();

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
#ifdef _DEBUG
	std::string passName = RenderPass::ToString(params.pass);
	GPU_SCOPED_EVENT(_user_defined_annotation, passName);
#endif

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
			//CascadeInfo const& cascade_info = l->get_cascade(0);
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
		if (!inst->is_ready())
			continue;

		auto ctx = _device_ctx;

		RenderWorldInstance::ConstantBufferData* data = (RenderWorldInstance::ConstantBufferData*)inst->_model_cb->map(_device_ctx);
		data->world = inst->_transform;
		data->wv = hlslpp::mul(data->world, params.view);
		data->wvp = hlslpp::mul(data->world, vp);
		inst->_model_cb->unmap(ctx);

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetIndexBuffer(inst->_mesh->_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		UINT strides = { sizeof(ModelResource::VertexType) };
		UINT offsets = { 0 };
		ctx->IASetVertexBuffers(0, 1, inst->_mesh->_vert_buffer.GetAddressOf(), &strides, &offsets);

		// #Hacky debug mode handling
		extern int g_DebugMode;
		if (g_DebugMode != 0)
		{
			ID3D11Buffer* buffers[3] = {
				_cb_global->Get(),
				inst->_model_cb->Get(),
				_cb_debug->Get(),
			};
			ctx->VSSetConstantBuffers(0, 3, buffers);
			ctx->PSSetConstantBuffers(0, 3, buffers);
		}
		else
		{
			ID3D11Buffer* buffers[2] = {
				_cb_global->Get(),
				inst->_model_cb->Get()
			};
			ctx->VSSetConstantBuffers(0, 2, buffers);
			ctx->PSSetConstantBuffers(0, 2, buffers);
		}

		for (Mesh const& m : inst->_mesh->_meshes)
		{
			std::shared_ptr<MaterialResource> res = inst->_mesh->_materials[m.materialID];

			Material* material = res->get();
			if (material->is_double_sided())
			{
				ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullNone).Get());
			}
			else
			{
				ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullBack).Get());
			}

			material->apply();
			if (params.pass != RenderPass::Opaque)
			{
				_device_ctx->PSSetShader(nullptr, nullptr, 0);
			}

			if (params.pass == RenderPass::Opaque)
			{
				ID3D11ShaderResourceView* views[] = { 
					_shadow_map_srv.Get(),
					_output_depth_srv_copy
				};
				_device_ctx->PSSetShaderResources(3, UINT(std::size(views)), views);
			}

			ctx->DrawIndexed((UINT)m.indexCount, (UINT)m.firstIndex, (INT)m.firstVertex);

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
	D3D11_TEXTURE2D_DESC desc{};
	if (_shadow_map)
	{
		_shadow_map->GetDesc(&desc);
	}
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

		for(u32 i = 0; i < num_cascades; ++i)
		{
			auto view_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(_shadow_map.Get(), D3D11_DSV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_D32_FLOAT,0, i);
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


void calculate_frustum(FrustumCorners& out_corners, f32 n, f32 f, f32 fov, f32 vFov)
{
	f32 tan_half_fov = tanf(fov / 2.0f);
	f32 tan_half_vfov = tanf(vFov / 2.0f);

	f32 x1 = n * tan_half_fov;
	f32 x2 = f * tan_half_fov;
	f32 y1 = n * tan_half_vfov;
	f32 y2 = f * tan_half_vfov;

	// Calculate the frustum in view space
	out_corners[0] = { x1, y1, n, 1.0 };
	out_corners[1] = { -x1, y1, n, 1.0 };
	out_corners[2] = { x1, -y1, n, 1.0 };
	out_corners[3] = { -x1, -y1, n, 1.0 };
	out_corners[4] = { x2, y2, f, 1.0 };
	out_corners[5] = { -x2, y2, f, 1.0 };
	out_corners[6] = { x2, -y2, f, 1.0 };
	out_corners[7] = { -x2, -y2, f, 1.0 };
}

void transform_frustum(FrustumCorners& corners, float4x4 matrix)
{
	for (u32 i = 0; i < corners.size(); ++i)
	{
		corners[i] = hlslpp::mul(corners[i], matrix);
	}
}


void calculate_frustum(FrustumCorners& out_corners, float4x4 cam_vp)
{
	f32 n = -1.0f;
	f32 f = 1.0f;
	f32 x1 = 1.0f;
	f32 x2 = 1.0f;
	f32 y1 = 1.0f;
	f32 y2 = 1.0f;

	// Calculate the frustum in view space
	out_corners[0] = {  1.0,  1.0, n, 1.0 };
	out_corners[1] = { -1.0,  1.0, n, 1.0 };
	out_corners[2] = {  1.0, -1.0, n, 1.0 };
	out_corners[3] = { -1.0, -1.0, n, 1.0 };
	out_corners[4] = {  1.0,  1.0, f, 1.0 };
	out_corners[5] = { -1.0,  1.0, f, 1.0 };
	out_corners[6] = {  1.0, -1.0, f, 1.0 };
	out_corners[7] = { -1.0, -1.0, f, 1.0 };

	float4x4 inv_m = hlslpp::inverse(cam_vp);
	transform_frustum(out_corners, inv_m);
	for(u32 i = 0; i < 8; ++i)
	{
		// Perspective divide
		out_corners[i].xyz = out_corners[i].xyz / out_corners[i].w;
		out_corners[i].w = 1.0f;
	}
}



FrustumCorners Renderer::get_frustum_world(shared_ptr<RenderWorld> const& world, u32 cam) const
{
	shared_ptr<RenderWorldCamera> camera = world->get_camera(cam);

	// Calculates the frustum straight from camera view proj
	FrustumCorners world_corners = {};
	calculate_frustum(world_corners, hlslpp::inverse(camera->get_vp()));
	return world_corners;
}

Graphics::FrustumCorners Renderer::get_cascade_frustum(shared_ptr<RenderWorldCamera> const& camera, u32 cascade, u32 num_cascades) const
{
	f32 n = camera->get_near();
	f32 f = camera->get_far();

	f32 z0 = n * pow(f / n, f32(cascade) / f32(num_cascades));
	f32 z1 = n * pow(f / n, f32(cascade+1) / f32(num_cascades));
	FrustumCorners world_corners = {};
	calculate_frustum(world_corners, z0, z1,camera->get_fov(), camera->get_vertical_fov());

	float4x4 view = camera->get_view();
	transform_frustum(world_corners, hlslpp::inverse(view));
	return world_corners;
}

static f32 s_box_size = 15.0f;

static float3 s_center[MAX_CASCADES];

void Renderer::render_shadow_pass(shared_ptr<RenderWorld> const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Shadows");

	prepare_shadow_pass();


	shared_ptr<RenderWorldLight> light = world->get_light(0);
	float4x4 light_space = light->get_view();

	if (light->get_casts_shadow())
	{
		float4x4 view = world->get_light(0)->get_view();
		float3 direction = world->get_light(0)->get_view_direction().xyz;

		std::vector<CascadeInfo> matrices;
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{

			FrustumCorners box_light = get_cascade_frustum(world->get_camera(0), i, MAX_CASCADES);

			// Transform to light view space
			transform_frustum(box_light, light_space);

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
			//DirectX::BoundingBox result(xm_center, xm_extents);
			//XMMATRIX mat;
			//hlslpp::store(light_view, (float*)&mat);
			//box.Transform(result, mat);

			f32 z_range = max_extents.z - min_extents.z;

			center = hlslpp::mul(float4(center,1.0), hlslpp::inverse(light_space)).xyz;
			s_center[i] = center;
			float3 new_center = center - direction * z_range;
			float4x4 new_projection = float4x4::orthographic(hlslpp::projection(hlslpp::frustum(-box.Extents.x, box.Extents.x, -box.Extents.y, box.Extents.y, 0.0f, z_range * 1.5f), hlslpp::zclip::zero));
			float4x4 new_view = float4x4::look_at(new_center, center, float3{ 0.0f, 1.0f, 0.0f });

			matrices.push_back({ new_view, new_projection, hlslpp::mul(new_view, new_projection) });
		}

		light->update_cascades(matrices);

		for(u32 i = 0; i < MAX_CASCADES; ++i)
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
			params.view_direction = direction;
			params.pass = RenderPass::Shadow;
			params.viewport = CD3D11_VIEWPORT(0.0f, 0.0f, 2048.0f, 2048.0f);
			render_world(world, params);
		}
	}
}



void Renderer::copy_depth()
{
	_device_ctx->CopyResource(_output_depth_copy, _output_depth);
}

RendererDebugTool::RendererDebugTool(Renderer* owner)
		: DebugOverlay(true, "RendererDebug")
	    , _renderer(owner)
		, _show_shadow_debug(false)
{

}


void RendererDebugTool::render_overlay()
{
	render_debug_tool();
	render_shader_tool();
}

void RendererDebugTool::render_3d(ID3D11DeviceContext* ctx)
{

	if(!_batch)
	{
		_batch = std::make_shared<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(ctx);
	}

	_batch->Begin();

	Debug::DrawGrid(_batch.get(), float4(100.0, 0.0, 0.0, 0.0), float4(0.0, 0.0, 100.0, 0.0), float4(0.0, 0.0, 0.0, 0.0), 10, 10, float4(1.0f,1.0f,1.0f,0.5f));

	if(_show_shadow_debug)
	{
		float4 frustum_color = float4(0.7f, 0.0f, 0.0f, 1.0f);
		auto world = GameEngine::instance()->get_render_world();
		if (world->get_camera(0) != world->get_view_camera())
		{
			std::vector<float4> colors = {
				float4(1.0f, 0.0f, 0.0f, 1.0f),
				float4(0.0f, 1.0f, 0.0f, 1.0f),
				float4(0.0f, 0.0f, 1.0f, 1.0f),
				float4(1.0f, 0.0f, 1.0f, 1.0f),
				float4(1.0f, 1.0f, 0.0f, 1.0f),
				float4(0.0f, 1.0f, 1.0f, 1.0f)
			};
			u32 num_cascades = MAX_CASCADES;
			for (u32 i = 0; i < num_cascades; ++i)
			{
				FrustumCorners frustum = _renderer->get_cascade_frustum(world->get_camera(0), i, num_cascades);
				Debug::DrawFrustum(_batch.get(), frustum, colors[i % colors.size()]);

				// Find the world space min/ max for each cascade
				float4 min = frustum[0];
				float4 max = frustum[0];
				for (u32 j = 1; j < frustum.size(); ++j)
				{
					min = hlslpp::min(frustum[j], min);
					max = hlslpp::max(frustum[j], max);
				}

				XMVECTOR xm_color;
				hlslpp::store(colors[(i) % colors.size()], (float*)&xm_color);

				// Visualize the bounding box in world space of our cascades
				float3 center = ((max + min) / 2.0f).xyz;
				float3 extents = ((max - min) / 2.0f).xyz;
				XMFLOAT3 xm_center;
				hlslpp::store(center, (float*)&xm_center);

				XMFLOAT3 xm_extents;
				hlslpp::store(extents, (float*)&xm_extents);
				auto box = DirectX::BoundingBox(xm_center, xm_extents);
				Debug::Draw(_batch.get(), box, xm_color);
			}
		}

		if (world->get_light(0))
		{
			static const float4 c_basic_cascade = float4(1.0f, 1.0f, 0.0f, 1.0f);
			static const float4 c_transformed = float4(0.0f, 1.0f, 0.0f, 1.0f);

			static u32 s_visualize_cascade = 0;
			{
				CascadeInfo const& info = world->get_light(0)->get_cascade(s_visualize_cascade);
				FrustumCorners corners;
				calculate_frustum(corners, info.vp);
				Debug::DrawFrustum(_batch.get(), corners, c_basic_cascade);

				float3 center = hlslpp::mul(float4(0.0, 0.0, 0.0, 1.0f), hlslpp::inverse(info.vp)).xyz;
				Debug::DrawRay(_batch.get(), float4(center.xyz, 1.0f), world->get_light(0)->get_view_direction(), true, c_basic_cascade);
			}

			auto box = DirectX::BoundingBox(XMFLOAT3{ s_center[s_visualize_cascade].x, s_center[s_visualize_cascade].y, s_center[s_visualize_cascade].z }, XMFLOAT3{ 1.0, 1.0, 1.0 });
			Debug::Draw(_batch.get(), box);
		}
	}

	_batch->End();
}

void RendererDebugTool::render_shader_tool()
{
	static bool s_open = true;
	if (ImGui::Begin("Shaders", &s_open))
	{
		ImGui::BeginTable("##ShaderTable", 2);
		ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
		ImGui::TableSetupColumn("Path");
		ImGui::TableSetupColumn("Button");
		ImGui::TableHeadersRow();


		auto shaders = ShaderCache::instance()->_shaders;
		for(auto it : shaders)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("%s", it.first.path.c_str());

			ImGui::TableNextColumn();
			ImGui::PushID(it.first.path.c_str());
			if (ImGui::Button("Build"))
			{
				ShaderCache::instance()->reload(it.first);
			}
			ImGui::PopID();
		}
		ImGui::EndTable();



	}
	ImGui::End();
}

void RendererDebugTool::render_debug_tool()
{
	if (ImGui::Begin("RendererDebug"), _isOpen)
	{
		ImGui::Checkbox("Show Shadow Debug", &_show_shadow_debug);

		if (ImGui::Button("Toggle Debug Cam"))
		{
			if (_renderer->_active_cam == 0)
			{
				_renderer->_active_cam = 1;
			}
			else
			{
				_renderer->_active_cam = 0;
			}
			auto world = GameEngine::instance()->get_render_world();
			world->set_active_camera(_renderer->_active_cam);
		}
		ImGui::Text("Active Camera: %d", _renderer->_active_cam);

		ImVec2 current_size = ImGui::GetContentRegionAvail();
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{
			ImGui::Image(_renderer->_debug_shadow_map_srv[i].Get(), { 150, 150 });
			if (i != MAX_CASCADES - 1)
				ImGui::SameLine();
		}

		ImGui::Image(_renderer->_output_depth_srv, { 150, 150 });

	}
	ImGui::End();
}

}
