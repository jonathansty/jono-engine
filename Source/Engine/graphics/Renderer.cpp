#include "engine.pch.h"
#include "Renderer.h"

#include "Core/Material.h"
#include "Core/MaterialResource.h"
#include "RenderWorld.h"
#include "Logging.h"
#include "CommandLine.h"

namespace Graphics
{

void Renderer::init(EngineSettings const& settings, GameSettings const& game_settings, cli::CommandLine const& cmdline)
{
	_msaa = settings.d3d_msaa_mode;
	_engine_settings = settings;
	_game_settings = game_settings;

	create_factories(settings, cmdline);

	_cb_global = ConstantBuffer::create(_device, sizeof(GlobalCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
	_cb_debug = ConstantBuffer::create(_device, sizeof(DebugCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
}

void Renderer::init_for_hwnd(HWND wnd)
{
	assert(!_wnd);
	_wnd = wnd;

	RECT r{};
	if(::GetWindowRect(wnd, &r))
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
		SUCCEEDED(_device->CreateTexture2D(&output_desc, nullptr, &_output_tex));
		SUCCEEDED(_device->CreateRenderTargetView(_output_tex, nullptr, &_output_rtv));
		SUCCEEDED(_device->CreateShaderResourceView(_output_tex, nullptr, &_output_srv));

		// This texture is used to output to a image in imgui
		output_desc.SampleDesc.Count = 1;
		output_desc.SampleDesc.Quality = 0;
		SUCCEEDED(_device->CreateTexture2D(&output_desc, nullptr, &_non_msaa_output_tex));
		SUCCEEDED(_device->CreateShaderResourceView(_non_msaa_output_tex, nullptr, &_non_msaa_output_srv));
	}

	// Create the 3D depth target
	auto dsv_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, w, h, 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
	SUCCEEDED(_device->CreateTexture2D(&dsv_desc, nullptr, &_output_depth));
	SUCCEEDED(_device->CreateDepthStencilView(_output_depth, NULL, &_output_dsv));

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

void Renderer::pre_render()
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"PreRender");
	extern int g_DebugMode;
	DebugCB* debug_data = (DebugCB*)_cb_debug->map(_device_ctx);
	debug_data->m_VisualizeMode = g_DebugMode;
	_cb_debug->unmap(_device_ctx);
}

void Renderer::render(shared_ptr<RenderWorld> const& world)
{
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
		DepthStencilState::Value depth_stencil_state = DepthStencilState::GreaterEqual;
		if (params.pass == RenderPass::ZPrePass || params.pass == RenderPass::Shadow)
		{
			depth_stencil_state = DepthStencilState::GreaterEqual;
		}

		ID3D11SamplerState* samplers[1] = {
			Graphics::get_sampler_state(SamplerState::MinMagMip_Linear).Get(),
		};
		_device_ctx->PSSetSamplers(0, 1, samplers);

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
	global->view_direction = float4(params.view_direction.xyz, 0.0f);

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
			info->light_space = l->get_vp();
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
				ID3D11ShaderResourceView* views[] = { _shadow_map_srv.Get() };
				_device_ctx->PSSetShaderResources(3, 1, views);
			}

			ctx->DrawIndexed((UINT)m.indexCount, (UINT)m.firstIndex, (INT)m.firstVertex);
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
		auto res_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, 2048, 2048);
		res_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		res_desc.MipLevels = 1;
		if (FAILED(_device->CreateTexture2D(&res_desc, NULL, _shadow_map.ReleaseAndGetAddressOf())))
		{
			ASSERTMSG(false, "Failed to create the shadowmap texture");
		}

		auto view_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(_shadow_map.Get(), D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT);
		if (FAILED(_device->CreateDepthStencilView(_shadow_map.Get(), &view_desc, _shadow_map_dsv.ReleaseAndGetAddressOf())))
		{
			ASSERTMSG(false, "Failed to create the shadowmap DSV");
		}

		auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(_shadow_map.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT);
		if (FAILED(_device->CreateShaderResourceView(_shadow_map.Get(), &srv_desc, _shadow_map_srv.ReleaseAndGetAddressOf())))
		{
			ASSERTMSG(false, "Failed to create the shadowmap SRV");
		}
	}
}

void Renderer::render_shadow_pass(shared_ptr<RenderWorld> const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, L"Shadows");

	prepare_shadow_pass();

	_device_ctx->OMSetRenderTargets(0, nullptr, _shadow_map_dsv.Get());
	_device_ctx->ClearDepthStencilView(_shadow_map_dsv.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0);

	shared_ptr<RenderWorldLight> light = world->get_light(0);
	if (light->get_casts_shadow())
	{
		ViewParams params{};
		params.view = light->get_view();
		params.proj = light->get_proj();
		params.view_direction = light->get_view_direction().xyz;
		params.pass = RenderPass::Shadow;
		params.viewport = CD3D11_VIEWPORT(0.0f, 0.0f, 2048, 2048);
		render_world(world, params);
	}
}

}
