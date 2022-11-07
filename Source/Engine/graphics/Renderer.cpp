#include "engine.pch.h"
#include "Renderer.h"

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

#include "Memory.h"

#include "Engine/Shaders/CommonShared.h"
#include "Graphics/StructuredBuffer.h"

namespace Graphics
{
using Helpers::SafeRelease;

static f32 s_box_size = 15.0f;


void Renderer::Init(EngineCfg const& settings, GameCfg const& game_settings, cli::CommandLine const& cmdline)
{
	MEMORY_TAG(MemoryCategory::Graphics);

	_visibility = std::make_unique<class VisibilityManager>();

	_stats = {};

	_msaa = settings.d3d_msaa_mode;
	_engine_settings = settings;
	_game_settings = game_settings;

	create_factories(settings, cmdline);

	_debug_tool = std::make_unique<RendererDebugTool>(this);
	GameEngine::instance()->get_overlay_manager()->register_overlay(_debug_tool.get());

	_cb_global = ConstantBuffer::create(_device, sizeof(GlobalCB), true, BufferUsage::Dynamic, nullptr);
	_cb_model = ConstantBuffer::create(_device, sizeof(ModelCB), true, BufferUsage::Dynamic, nullptr);
	_cb_debug = ConstantBuffer::create(_device, sizeof(DebugCB), true, BufferUsage::Dynamic, nullptr);
	_cb_post = ConstantBuffer::create(_device, sizeof(PostCB), true, BufferUsage::Dynamic, nullptr);


	// Setup Light buffer
	{
		_light_buffer = GPUStructuredBuffer::create(_device, sizeof(ProcessedLight), c_max_lights,true, BufferUsage::Dynamic);
	}

	// Create our cubemap 
	std::array<std::string_view, 6> faces = {
		"Resources/skybox/right.jpg",
		"Resources/skybox/left.jpg",
		"Resources/skybox/top.jpg",
		"Resources/skybox/bottom.jpg",
		"Resources/skybox/back.jpg",
		"Resources/skybox/front.jpg"
	};

	std::array<ComPtr<ID3D11Texture2D>, 6> intermediate{};

	int width = 0, height = 0;
	int mip_levels = 1;
	for (size_t i = 0; i < faces.size(); ++i)
	{
		int nrChannels;
		stbi_uc* data = stbi_load(faces[i].data(), &width, &height, &nrChannels, 4);

		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1,0);
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		D3D11_SUBRESOURCE_DATA subresource{};
		subresource.pSysMem = data; 
		subresource.SysMemPitch = width * sizeof(stbi_uc) * 4;
		ComPtr<ID3D11Texture2D> tex{};
		ENSURE_HR(_device->CreateTexture2D(&desc, &subresource, tex.GetAddressOf()));


		intermediate[i] = tex;

		stbi_image_free(data);
	}
	int tmp = width;
	while (tmp > 4)
	{
		tmp = tmp >> 1;
		++mip_levels;
	}

	ComPtr<ID3D11Texture2D> cubemap{};
	CD3D11_TEXTURE2D_DESC cubemap_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 6, mip_levels);
	cubemap_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubemap_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	ENSURE_HR(_device->CreateTexture2D(&cubemap_desc, nullptr, &cubemap));

	for (size_t i = 0; i < faces.size(); ++i)
	{
		_device_ctx->CopySubresourceRegion(cubemap.Get(), D3D11CalcSubresource(0, (UINT)i, mip_levels), 0, 0, 0, intermediate[i].Get(), D3D11CalcSubresource(0, 0, mip_levels), nullptr);
	}
	_cubemap = cubemap;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	srv_desc.TextureCube.MipLevels = mip_levels;
	srv_desc.TextureCube.MostDetailedMip = 0;
	srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	ENSURE_HR(_device->CreateShaderResourceView(_cubemap.Get(), &srv_desc, _cubemap_srv.GetAddressOf()));
	_device_ctx->GenerateMips(_cubemap_srv.Get());

}

void Renderer::InitForWindow(SDL_Window* window)
{
	MEMORY_TAG(MemoryCategory::Graphics);

	assert(!m_Window);
	m_Window = window;
	SDL_GetWindowSize(window, (int*)&m_DrawableAreaWidth, (int*)&m_DrawableAreaHeight);


	{
		// Initially start out with the window as our viewport. This will most likely get
		// resized when ImGui creates our viewport control
		_viewport_pos = { 0, 0 };
		_viewport_width = m_DrawableAreaWidth;
		_viewport_height = m_DrawableAreaHeight;

		resize_swapchain(_viewport_width, _viewport_height);
	}

	if (_d2d_rt)
	{
		// set alias mode
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);

		// Create a brush.
		_d2d_rt->CreateSolidColorBrush((D2D1::ColorF)D2D1::ColorF::Black, &_color_brush);
	}
}

void Renderer::DeInit()
{
	MEMORY_TAG(MemoryCategory::Graphics);

	GameEngine::instance()->get_overlay_manager()->unregister_overlay(_debug_tool.get());


	_device_ctx->ClearState();
	_device_ctx->Flush();

	release_frame_resources();
	release_device_resources();
}

void Renderer::create_factories(EngineCfg const& settings, cli::CommandLine const& cmdline)
{
	MEMORY_TAG(MemoryCategory::Graphics);

	// Create Direct3D 11 factory
	{
		ENSURE_HR(CreateDXGIFactory(IID_PPV_ARGS(&_factory)));
		Helpers::SetDebugObjectName(_factory, "Main DXGI Factory");

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
void Renderer::create_d2d_factory(EngineCfg const& settings)
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

void Renderer::release_device_resources()
{
	SafeRelease(_color_brush);
	SafeRelease(_user_defined_annotation);

	SafeRelease(_device_ctx);
	SafeRelease(_device);
	SafeRelease(_dwrite_factory);
	SafeRelease(_wic_factory);
	SafeRelease(_d2d_factory);
	SafeRelease(_factory);
}

void Renderer::release_frame_resources()
{
	// Release all the MSAA copies
	SafeRelease(_non_msaa_output_tex);
	SafeRelease(_non_msaa_output_tex_copy);
	SafeRelease(_non_msaa_output_srv);
	SafeRelease(_non_msaa_output_srv_copy);
	SafeRelease(_non_msaa_output_rtv);

	SafeRelease(_d2d_rt);

	SafeRelease(_swapchain);
	SafeRelease(_swapchain_rtv);
	SafeRelease(_swapchain_srv);

	SafeRelease(_output_tex);
	SafeRelease(_output_rtv);
	SafeRelease(_output_srv);
	SafeRelease(_output_depth);
	SafeRelease(_output_depth_copy);
	SafeRelease(_output_depth_srv);
	SafeRelease(_output_depth_srv_copy);
	SafeRelease(_output_dsv);
}

void Renderer::resize_swapchain(u32 w, u32 h)
{
	assert(m_Window);

	m_DrawableAreaWidth = w;
	m_DrawableAreaHeight = h;

	LOG_VERBOSE(Graphics, "Resizing swapchain to {}x{}", w, h);
	DXGI_FORMAT swapchain_format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
		release_frame_resources();
	}

	// Create the 3D output target
	{
		auto output_desc = CD3D11_TEXTURE2D_DESC(
				DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
				w,
				h,
				1,
				1,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
		ENSURE_HR(_device->CreateTexture2D(&output_desc, nullptr, &_output_tex));
		ENSURE_HR(_device->CreateRenderTargetView(_output_tex, nullptr, &_output_rtv));
		ENSURE_HR(_device->CreateShaderResourceView(_output_tex, nullptr, &_output_srv));

		Helpers::SetDebugObjectName(_output_tex, "Renderer::Output (MSAA)");
		Helpers::SetDebugObjectName(_output_rtv, "Renderer::Output (MSAA)");
		Helpers::SetDebugObjectName(_output_srv, "Renderer::Output (MSAA)");

		// This texture is used to output to a image in imgui
		output_desc.SampleDesc.Count = 1;
		output_desc.SampleDesc.Quality = 0;
		ENSURE_HR(_device->CreateTexture2D(&output_desc, nullptr, &_non_msaa_output_tex));
		ENSURE_HR(_device->CreateTexture2D(&output_desc, nullptr, &_non_msaa_output_tex_copy));
		ENSURE_HR(_device->CreateShaderResourceView(_non_msaa_output_tex, nullptr, &_non_msaa_output_srv));
		ENSURE_HR(_device->CreateShaderResourceView(_non_msaa_output_tex_copy, nullptr, &_non_msaa_output_srv_copy));
		ENSURE_HR(_device->CreateRenderTargetView(_non_msaa_output_tex, nullptr, &_non_msaa_output_rtv));

		Helpers::SetDebugObjectName(_non_msaa_output_tex, "Renderer::Output (MSAA)");
		Helpers::SetDebugObjectName(_non_msaa_output_srv, "Renderer::Output (MSAA)");
		Helpers::SetDebugObjectName(_non_msaa_output_rtv, "Renderer::Output (MSAA)");

		Helpers::SetDebugObjectName(_non_msaa_output_tex_copy, "Renderer::Output (MSAA) (COPY)");
		Helpers::SetDebugObjectName(_non_msaa_output_srv_copy, "Renderer::Output (MSAA) (COPY)");

	}

	// Create the 3D depth target
	auto depth_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16_TYPELESS, w, h, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
	ENSURE_HR(_device->CreateTexture2D(&depth_desc, nullptr, &_output_depth));
	ENSURE_HR(_device->CreateTexture2D(&depth_desc, nullptr, &_output_depth_copy));

	Helpers::SetDebugObjectName(_output_depth, "Renderer::Output Depth");
	Helpers::SetDebugObjectName(_output_depth_copy, "Renderer::Output Depth (COPY)");


	auto view_dim = D3D11_DSV_DIMENSION_TEXTURE2D;
	auto srv_view_dim = D3D11_SRV_DIMENSION_TEXTURE2D;
	if(_msaa != MSAAMode::Off)
	{
		view_dim = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		srv_view_dim = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	}
	auto dsv_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(_output_depth, view_dim, DXGI_FORMAT_D16_UNORM);
	ENSURE_HR(_device->CreateDepthStencilView(_output_depth, &dsv_desc, &_output_dsv));
	Helpers::SetDebugObjectName(_output_dsv, "Renderer::Output Depth");

	auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(_output_depth, srv_view_dim, DXGI_FORMAT_R16_UNORM);
	ENSURE_HR(_device->CreateShaderResourceView(_output_depth, &srv_desc, &_output_depth_srv));
	Helpers::SetDebugObjectName(_output_depth_srv, "Renderer::Output Depth");

	ENSURE_HR(_device->CreateShaderResourceView(_output_depth_copy, &srv_desc, &_output_depth_srv_copy));
	Helpers::SetDebugObjectName(_output_depth_srv_copy, "Renderer::Output Depth (COPY)");

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

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if(SDL_GetWindowWMInfo(m_Window, &info))
		{
			desc.OutputWindow = info.info.win.window;
		}

		SDL_PixelFormatEnum pixelFormat = (SDL_PixelFormatEnum)SDL_GetWindowPixelFormat(m_Window);

		if (_game_settings.m_FullscreenMode == GameCfg::FullScreenMode::Windowed || _game_settings.m_FullscreenMode == GameCfg::FullScreenMode::BorderlessWindowed)
		{
			desc.Windowed = TRUE;
		}
		else
		{
			desc.Windowed = false;
		}
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
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
	int display = SDL_GetWindowDisplayIndex(m_Window);

	float dpi, hdpi, vdpi;
	SDL_GetDisplayDPI(display, &dpi, &hdpi, &vdpi);

	D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)hdpi, (FLOAT)vdpi);

	ComPtr<IDXGISurface> surface;
	_output_tex->QueryInterface(surface.GetAddressOf());

	if (_d2d_factory)
	{
		SUCCEEDED(_d2d_factory->CreateDxgiSurfaceRenderTarget(surface.Get(), rtp, &_d2d_rt));
		set_debug_name(surface.Get(), "[D2D] Output");

		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
}

void Renderer::pre_render(RenderWorld const& world)
{
	JONO_EVENT();
	GPU_SCOPED_EVENT(_user_defined_annotation, "PreRender");

	// Update the debug buffers
	{
		extern int g_DebugMode;
		DebugCB* debug_data = (DebugCB*)_cb_debug->map(_device_ctx);
		debug_data->m_VisualizeMode = g_DebugMode;
		_cb_debug->unmap(_device_ctx);
	}

	// Update the cameras
	for (auto& cam : world.get_cameras())
	{
		// Update our view camera to properly match the viewport aspect
		cam->set_aspect((f32)_viewport_width / (f32)_viewport_height);
		cam->update();
	}

	// Update all the render world instances
	for (std::shared_ptr<RenderWorldInstance> const& inst : world.get_instances())
	{
		// Finalise a render instance after it's done loading
		if (inst->_model->is_loaded() && !inst->is_finalised())
		{
			inst->finalise();
		}

		// Update all material instances and other dynamic data
		inst->update();
	}

	// Update directional light frustums, NEEDS to happen before visibility
	shared_ptr<RenderWorldLight> directional_light = nullptr;
	{

		// Retrieve the first directional light in the render world
		auto it = std::find_if(world.get_lights().begin(), world.get_lights().end(), [](auto const& light)
				{ return light->is_directional() && light->get_casts_shadow(); });
		if (it != world.get_lights().end())
		{
			directional_light = *it;
		}

		if (!directional_light)
		{
			return;
		}

		float4x4 light_space = directional_light->get_view();
		if (directional_light->get_casts_shadow())
		{
			float4x4 view = world.get_light(0)->get_view();
			float3 direction = world.get_light(0)->get_view_direction().xyz;
			float3 position = world.get_light(0)->get_position().xyz;

			std::vector<CascadeInfo> matrices;
			for (u32 i = 0; i < MAX_CASCADES; ++i)
			{
				Math::Frustum box_light = get_cascade_frustum(world.get_camera(0), i, MAX_CASCADES);

				// Transform to light view space
				box_light.transform(light_space);

				// Calculate the extents(in light  space)
				float4 min_extents = box_light._corners[0];
				float4 max_extents = box_light._corners[0];

				for (u32 j = 1; j < box_light._corners.size(); ++j)
				{
					float4 pos_light = box_light._corners[j];
					min_extents = hlslpp::min(pos_light, min_extents);
					max_extents = hlslpp::max(pos_light, max_extents);
				}

				float3 center = ((min_extents + max_extents) / 2.0f).xyz;
				float3 extents = ((max_extents - min_extents) / 2.0f).xyz;

				using namespace DirectX;
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

			directional_light->update_cascades(matrices);
		}
	}

	// Register our visible instances and calculate visibilities for main view?
	{
		JONO_EVENT("Visibility");
		_visibility->reset();

		for (std::shared_ptr<RenderWorldInstance> const& inst : world.get_instances())
		{
			if (inst->is_ready())
			{
				_visibility->add_instance(inst.get());
			}
		}

		VisibilityParams params{};
		params.frustum[VisiblityFrustum_Main] = get_frustum_world(world, 0);
		params.frustum[VisiblityFrustum_CSM0] = Math::Frustum::from_vp(directional_light->get_cascade(0).vp);
		params.frustum[VisiblityFrustum_CSM1] = Math::Frustum::from_vp(directional_light->get_cascade(1).vp);
		params.frustum[VisiblityFrustum_CSM2] = Math::Frustum::from_vp(directional_light->get_cascade(2).vp);
		params.frustum[VisiblityFrustum_CSM3] = Math::Frustum::from_vp(directional_light->get_cascade(3).vp);
		_visibility->run(params);
	}

	// Light culling step is scheduled here.
	// 1. CPU frustum culling
	// 2. F+ tiled culling

	// 1. CPU Frustum culling
	{
		// Collect all local lights and update our light buffer
		ScopedBufferAccess access{ _device_ctx, _light_buffer.get() };
		ProcessedLight* light_data = static_cast<ProcessedLight*>(access.get_ptr());
		u32 n_lights = 0;
		u32 n_directional_lights = 0;
		for (std::shared_ptr<RenderWorldLight> const& light : world.get_lights())
		{
			if (!light->is_directional())
			{
				ProcessedLight& data = *light_data;
				data.position = light->get_position();
				data.direction = Shaders::float3(hlslpp::normalize(light->get_view_direction().xyz));
				data.range = light->get_range();
				data.color = light->get_colour();
				data.cone = hlslpp::cos(float1(light->get_cone_angle()));
				data.outer_cone = hlslpp::cos(float1(light->get_outer_cone_angle()));

				using LightType = RenderWorldLight::LightType;
				switch (light->get_type())
				{
					case LightType::Point:
						data.flags |= LIGHT_TYPE_POINT;
						break;
					case LightType::Spot:
						data.flags |= LIGHT_TYPE_SPOT;
						break;
				}

				++n_lights;
				++light_data;
			}
			else
			{
				++n_directional_lights;
			}
		}
		_num_lights = n_lights;
		_num_directional_lights = n_directional_lights;
	}

	// 2. F+ GPU Tiled culling
	{

		constexpr u32 tile_res = FPLUS_TILE_RES;
		float width = GameEngine::instance()->GetWindowSize().x;
		float height = GameEngine::instance()->GetWindowSize().y;

		u32 tiles_x = (u32)ceilf(width / tile_res);
		u32 tiles_y = (u32)ceilf(height / tile_res);

		_num_tiles_x = tiles_x;

		struct FPlusCB
		{
			Shaders::float4x4 proj_inv;
			Shaders::float4x4 view;

			int num_tiles_x;
			int num_tiles_y;
			int number_of_lights;
		};

		// #TODO: Resize buffers to match screen size
		if(!_per_tile_info_buffer)
		{
			_per_tile_info_buffer = GPUByteBuffer::create(_device, tiles_x * tiles_y * sizeof(u32), false, BufferUsage::Default);
			_tile_light_index_buffer = GPUByteBuffer::create(_device, tiles_x * tiles_y * FPLUS_MAX_NUM_LIGHTS_PER_TILE * sizeof(u32), false, BufferUsage::Default);

			ShaderCreateParams cs_params = ShaderCreateParams::compute_shader("Source/Engine/shaders/ForwardPlus_Cull.hlsl");
			cs_params.params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
			_fplus_cull_shader = ShaderCache::instance()->find_or_create(cs_params);

			_fplus_cb = ConstantBuffer::create(_device, sizeof(FPlusCB), true, BufferUsage::Dynamic);
		}


		ID3D11DeviceContext* ctx = _device_ctx;

		shared_ptr<RenderWorldCamera> camera = world.get_camera(0);

		// Update CB
		{
			ScopedBufferAccess fplus_cb_lock = ScopedBufferAccess(ctx, _fplus_cb.get());
			FPlusCB& cb = *(FPlusCB*)fplus_cb_lock.get_ptr();
			cb.num_tiles_x = tiles_x;
			cb.num_tiles_y = tiles_y;
			cb.number_of_lights = _num_lights;

			cb.view = camera->get_view();
			cb.proj_inv = hlslpp::inverse(camera->get_proj());
		}

		ctx->OMSetRenderTargets(0, nullptr, nullptr);

		std::array<ID3D11ShaderResourceView*, 2> srvs = {
			_light_buffer->get_srv().Get(),
			_output_depth_srv
		};
		ctx->CSSetShaderResources(0, 2, srvs.data());

		std::array<ID3D11UnorderedAccessView*, 2> uavs = {
			_tile_light_index_buffer->get_uav().Get(),
			_per_tile_info_buffer->get_uav().Get()
		};

		ctx->CSSetUnorderedAccessViews(0, 2, uavs.data(), nullptr);

		std::array<ID3D11Buffer*, 1> cbs = {
			_fplus_cb->Get()
		};
		ctx->CSSetConstantBuffers(0, 1, cbs.data());
		ctx->CSSetShader(_fplus_cull_shader->as<ID3D11ComputeShader>().Get(), nullptr, 0);

		u32 dispatch_x = tiles_x;
		u32 dispatch_y = tiles_y;
		ctx->Dispatch(dispatch_x, dispatch_y, 1);

		uavs[0] = nullptr;
		uavs[1] = nullptr;
		ctx->CSSetUnorderedAccessViews(0, 2, uavs.data(), nullptr);

		srvs[0] = nullptr;
		srvs[1] = nullptr;
		ctx->CSSetShaderResources(0, 2, srvs.data());
	}

}

void Renderer::render_view(RenderWorld const& world, RenderPass::Value pass)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, "Renderer::render_view");

	// Setup global constant buffer
	std::shared_ptr<RenderWorldCamera> camera  = world.get_view_camera();

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

void Renderer::render_world(RenderWorld const& world, ViewParams const& params)
{
	std::string passName = RenderPass::ToString(params.pass);
	GPU_SCOPED_EVENT(_user_defined_annotation, passName.c_str());

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

	global->num_directional_lights = std::min<u32>(u32(_num_directional_lights), MAX_LIGHTS);
	global->num_lights = _num_lights;
	global->num_tiles_x = _num_tiles_x;

	// Process all the lights
	RenderWorld::LightCollection const& lights = world.get_lights();
	for (u32 i = 0; i < lights.size(); ++i)
	{
		DirectionalLightInfo* info = global->lights + i;
		shared_ptr<RenderWorldLight> l = lights[i];
		if (l->is_directional())
		{
			info->direction = float4(l->get_view_direction().xyz, 0.0f);
			info->colour = float4(l->get_colour(), 1.0f);

			// Each directional light could have N cascades so we need to store orthographic projects for each light
			// CascadeInfo const& cascade_info = l->get_cascade(0);
			info->light_space = float4x4::identity();

			if(s_EnableShadowRendering)
			{
				info->num_cascades = MAX_CASCADES;
				for (int j = 0; j < MAX_CASCADES; ++j)
				{
					f32 n = world.get_camera(0)->get_near();
					f32 f = world.get_camera(0)->get_far();

					f32 z0 = n * pow(f / n, f32(j) / f32(MAX_CASCADES));
					f32 z1 = n * pow(f / n, f32(j + 1) / f32(MAX_CASCADES));
					info->cascade_distances[j] = z1;
					info->cascades[j] = l->get_cascade(j).vp;
				}
			}
			else
			{
				info->num_cascades = 0;
				for (int j = 0; j < MAX_CASCADES; ++j)
				{
					info->cascade_distances[j] = {};
					info->cascades[j] = {};
				}
			}
		}
	}
	_cb_global->unmap(_device_ctx);

	float4x4 vp = hlslpp::mul(params.view, params.proj);

	// information that is needed to represent 1 draw call
	struct DrawCall
	{
		float4x4 _transform;

		// Mesh index buffer
		ID3D11Buffer* _index_buffer;

		// Vertex buffers
		ID3D11Buffer* _vertex_buffer;

			// First vertex offset this mesh starts at in the vertex buffer
		u64 _first_vertex;

		// First index location offset this mesh starts at in the index buffer
		u64 _first_index;

		// The amount of indices associated with this mesh
		u64 _index_count;

		// Material (e.g. shaders, textures, constant buffer, input layouts)
		MaterialInstance const* _material;
	
	};
	static std::vector<DrawCall> m_DrawCalls;


	{
		JONO_EVENT("GenerateDrawCalls");

		std::vector<RenderWorldInstance*> const& instances = _visibility->get_visible_instances();
		m_DrawCalls.reserve(instances.size());
		m_DrawCalls.clear();

		// #TODO: Pull this information from visibile lists
		for (RenderWorldInstance const* inst : instances)
		{
			// If the instance is ready we can consider adding it to the draw list
			if (inst->is_ready())
			{
				Model const* model = inst->_model->get();
				for (Mesh const& mesh : model->get_meshes())
				{
					DrawCall dc{};
					dc._transform = inst->_transform;
					dc._index_buffer = model->get_index_buffer().Get();
					dc._vertex_buffer = model->get_vertex_buffer().Get();
					dc._material = inst->get_material_instance(mesh.material_index);
					dc._first_index = mesh.firstIndex;
					dc._first_vertex = mesh.firstVertex;
					dc._index_count = mesh.indexCount;
					m_DrawCalls.push_back(dc);
				}
			}
		}
	}

	JONO_EVENT("Submit");

	ID3D11DeviceContext* ctx = _device_ctx;
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// #TODO: Remove debug handling into a debug lighting system
	extern int g_DebugMode;
	if (g_DebugMode != 0)
	{
		ID3D11Buffer* buffers[3] = {
			_cb_global->Get(),
			_cb_debug->Get()
		};
		ctx->VSSetConstantBuffers(0, 2, buffers);
		ctx->PSSetConstantBuffers(0, 2, buffers);
	}
	else
	{
		ID3D11Buffer* buffers[1] = {
			_cb_global->Get(),
		};
		ctx->VSSetConstantBuffers(0, 1, buffers);
		ctx->PSSetConstantBuffers(0, 1, buffers);
	}

	ID3D11Buffer* prev_index = nullptr;
	ID3D11Buffer* prev_vertex = nullptr;

	for (DrawCall const& dc : m_DrawCalls) 
	{
		ConstantBufferRef const& model_cb = _cb_model;
		ModelCB* data = (ModelCB*)model_cb->map(_device_ctx);
		data->world = dc._transform;
		data->wv = hlslpp::mul(data->world, params.view);
		data->wvp = hlslpp::mul(data->world, vp);
		model_cb->unmap(ctx);

		if(prev_index != dc._index_buffer)
		{
			ctx->IASetIndexBuffer(dc._index_buffer, DXGI_FORMAT_R32_UINT, 0);
			prev_index = dc._index_buffer;
		}

		if (prev_vertex != dc._vertex_buffer)
		{
			UINT strides = { sizeof(Model::VertexType) };
			UINT offsets = { 0 };
			ctx->IASetVertexBuffers(0, 1, &dc._vertex_buffer, &strides, &offsets);

			prev_vertex = dc._vertex_buffer;
		}

		ID3D11Buffer* buffers[1] = 
		{
			model_cb->Get()
		};
		ctx->VSSetConstantBuffers(2, 1, buffers);
		ctx->PSSetConstantBuffers(2, 1, buffers);

		// Setup the material render state
		{
			setup_renderstate(dc._material, params);

			++_stats.n_draws;
			_stats.n_primitives += u32(dc._index_count / 3);
			ctx->DrawIndexed((UINT)dc._index_count, (UINT)dc._first_index, (INT)dc._first_vertex);

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
	JONO_EVENT();

	_stats = {};

	// Reset all the state tracking
	_device_ctx->ClearState();
	_last_vs = nullptr;
	_last_ps = nullptr;
	_last_input_layout = nullptr;
	_last_rs = nullptr;


	GameEngine* engine = GameEngine::instance();
	D3D11_VIEWPORT vp{};
	vp.Width = static_cast<float>(engine->GetViewportSize().x);
	vp.Height = static_cast<float>(engine->GetViewportSize().y);
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	_device_ctx->RSSetViewports(1, &vp);

	// Clear the output targets
	FLOAT color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	_device_ctx->ClearRenderTargetView(_output_rtv, color);
	_device_ctx->ClearDepthStencilView(_output_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::end_frame()
{
}

Math::Frustum Renderer::get_frustum_world(RenderWorld const& world, u32 cam) const
{
	shared_ptr<RenderWorldCamera> camera = world.get_camera(cam);
	return Math::Frustum::from_vp(camera->get_vp());
}

Math::Frustum Renderer::get_cascade_frustum(shared_ptr<RenderWorldCamera> const& camera, u32 cascade, u32 num_cascades) const
{
	f32 n = camera->get_near();
	f32 f = camera->get_far();

	f32 z0 = n * pow(f / n, f32(cascade) / f32(num_cascades));
	f32 z1 = n * pow(f / n, f32(cascade + 1) / f32(num_cascades));

	Math::Frustum frustum = Math::Frustum::from_fov(z0, z1, camera->get_horizontal_fov(), camera->get_vertical_fov());

	float4x4 view = camera->get_view();
	frustum.transform(hlslpp::inverse(view));
	return frustum;
}

void Renderer::setup_renderstate(MaterialInstance const* mat_instance, ViewParams const& params)
{
	auto ctx = _device_ctx;

	// Bind anything coming from the material
	mat_instance->apply(this, params);

	// Bind the global textures coming from rendering systems
	if (params.pass == RenderPass::Opaque)
	{
		ID3D11ShaderResourceView* views[] = {
			s_EnableShadowRendering ? _shadow_map_srv.Get() : nullptr,
			_output_depth_srv_copy,
			_cubemap_srv.Get(),
			_light_buffer->get_srv().Get(),
			_tile_light_index_buffer->get_srv().Get(),
			_per_tile_info_buffer->get_srv().Get()
		};
		_device_ctx->PSSetShaderResources(Texture_CSM, UINT(std::size(views)), views);
	}
}

void Renderer::VSSetShader(ShaderConstRef const& vertex_shader)
{
	if(_last_vs != vertex_shader)
	{
		if (vertex_shader == nullptr)
		{
			_device_ctx->VSSetShader(nullptr, nullptr, 0);
		}
		else
		{
			ASSERT(vertex_shader->get_type() == ShaderType::Vertex);
			_device_ctx->VSSetShader(vertex_shader->as<ID3D11VertexShader>().Get(), nullptr, 0);
		}
		_last_vs = vertex_shader;
	}

}

void Renderer::PSSetShader(ShaderConstRef const& pixel_shader)
{
	if(_last_ps != pixel_shader)
	{
		if(pixel_shader == nullptr)
		{
			_device_ctx->PSSetShader(nullptr, nullptr, 0);
		}
		else
		{
			ASSERT(pixel_shader->get_type() == ShaderType::Pixel);
			_device_ctx->PSSetShader(pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
		}

		_last_ps = pixel_shader;
	}
}

void Renderer::IASetInputLayout(ID3D11InputLayout* layout)
{
	if(_last_input_layout != layout)
	{
		_device_ctx->IASetInputLayout(layout);
		_last_input_layout = layout;	
	}
}

void Renderer::RSSetState(ID3D11RasterizerState* state)
{
	if(_last_rs != state)
	{
		_device_ctx->RSSetState(state);
		_last_rs = state;
	}
}

void Renderer::render_post_predebug()
{
	GPU_SCOPED_EVENT(_user_defined_annotation, "Post:PreDebug");

	ShaderCreateParams params = ShaderCreateParams::pixel_shader("Source/Engine/Shaders/default_post_px.hlsl");
	ShaderRef post_shader = ShaderCache::instance()->find_or_create(params);

	params = ShaderCreateParams::vertex_shader("Source/Engine/Shaders/default_post_vx.hlsl");
	ShaderRef post_vs_shader = ShaderCache::instance()->find_or_create(params);

	_device_ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN,0);
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
	GPU_SCOPED_EVENT(_user_defined_annotation, "Post:PostDebug");


}

void Renderer::render_shadow_pass(RenderWorld const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, "Shadows");

	prepare_shadow_pass();

	shared_ptr<RenderWorldLight> light = nullptr;

	// Retrieve the first directional light in the render world
	auto it = std::find_if(world.get_lights().begin(), world.get_lights().end(), [](auto const& light){ return light->is_directional() && light->get_casts_shadow(); });
	if(it != world.get_lights().end())
	{
		light = *it;
	}

	if(!light)
	{
		return;
	}

	float4x4 light_space = light->get_view();
	if (light->get_casts_shadow())
	{
		float4x4 view = world.get_light(0)->get_view();
		float3 direction = world.get_light(0)->get_view_direction().xyz;
		float3 position = world.get_light(0)->get_position().xyz;

		// Render out the cascades shadow map for the directional light
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{
			GPU_SCOPED_EVENT(_user_defined_annotation, fmt::format("Cascade {}", i).c_str());
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

void Renderer::render_zprepass(RenderWorld const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, "zprepass");


	_device_ctx->OMSetRenderTargets(0, NULL, _output_dsv);

	render_view(world, RenderPass::ZPrePass);
}

void Renderer::render_opaque_pass(RenderWorld const& world)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, "opaque");

	// Do a copy to our dsv tex for sampling during the opaque pass
	copy_depth();

	_device_ctx->OMSetRenderTargets(1, &_output_rtv, _output_dsv);
	render_view(world, Graphics::RenderPass::Opaque);
}

void Renderer::render_post(RenderWorld const& world, shared_ptr<OverlayManager> const& overlays, bool doImgui)
{
	GPU_SCOPED_EVENT(_user_defined_annotation, "Post");

	PostCB* data = (PostCB*)_cb_post->map(_device_ctx);
	data->m_ViewportWidth = (f32)(m_DrawableAreaWidth);
	data->m_ViewportHeight = (f32)(m_DrawableAreaHeight);
	_cb_post->unmap(_device_ctx);

	if (!_states)
	{
		_states = std::make_unique<DirectX::CommonStates>(_device);
		_common_effect = std::make_unique<DirectX::BasicEffect>(_device);
		_common_effect->SetVertexColorEnabled(true);

		void const* shader_byte_code;
		size_t byte_code_length;
		_common_effect->GetVertexShaderBytecode(&shader_byte_code, &byte_code_length);
		ENSURE_HR(_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements, DirectX::VertexPositionColor::InputElementCount, shader_byte_code, byte_code_length, _layout.ReleaseAndGetAddressOf()));

	}

	_device_ctx->OMSetBlendState(_states->Opaque(), nullptr, 0xFFFFFFFF);
	_device_ctx->OMSetDepthStencilState(_states->DepthRead(), 0);
	_device_ctx->RSSetState(_states->CullNone());

	_device_ctx->IASetInputLayout(_layout.Get());

	float4x4 view = float4x4::identity();
	float4x4 proj = float4x4::identity();
	if(world.get_view_camera())
	{
		view = world.get_view_camera()->get_view();
		proj = world.get_view_camera()->get_proj();
	}

	using namespace DirectX;
	Shaders::float4x4 view4x4 = Shaders::float4x4(view);
	Shaders::float4x4 proj4x4 = Shaders::float4x4(proj);
	XMMATRIX xm_view = view4x4;
	XMMATRIX xm_proj = proj4x4;
	_common_effect->SetView(xm_view);
	_common_effect->SetProjection(xm_proj);
	_common_effect->Apply(_device_ctx);

	{
		GPU_SCOPED_EVENT(_user_defined_annotation, "Post:RenderOverlays");
		overlays->Render3D(_device_ctx);
	}

	D3D11_VIEWPORT vp{};
	vp.Width = static_cast<float>(_viewport_width);
	vp.Height = static_cast<float>(_viewport_height);
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	_device_ctx->RSSetViewports(1, &vp);

		// Resolve msaa to non msaa for imgui render
	_device_ctx->ResolveSubresource(_non_msaa_output_tex, 0, _output_tex, 0, _swapchain_format);

	// Copy the non-msaa world render so we can sample from it in the post pass
	_device_ctx->CopyResource(_non_msaa_output_tex_copy, _non_msaa_output_tex);
	_device_ctx->OMSetRenderTargets(1, &_non_msaa_output_rtv, nullptr);
	render_post_predebug();

	render_post_postdebug();

	vp.Width = static_cast<float>(m_DrawableAreaWidth);
	vp.Height = static_cast<float>(m_DrawableAreaHeight);
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	_device_ctx->RSSetViewports(1, &vp);


	// Render main viewport ImGui
	if(doImgui)
	{
		GPU_SCOPED_EVENT(_user_defined_annotation, "ImGui");
		_device_ctx->OMSetRenderTargets(1, &_swapchain_rtv, nullptr);
		ImDrawData* imguiData = &GetGlobalContext()->m_GraphicsThread->m_FrameData.m_DrawData;
		ImGui_ImplDX11_RenderDrawData(imguiData);
	}

}

void Renderer::copy_depth()
{
	_device_ctx->CopyResource(_output_depth_copy, _output_depth);
}

} // namespace Graphics
