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

#include "ShaderCache.h"
#include "Graphics/ShaderStage.h"

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

	m_Visibility = std::make_unique<class VisibilityManager>();

	_stats = {};

	_msaa = settings.m_MSAA;
	_engine_settings = settings;
	_game_settings = game_settings;


	// #TODO: Move this out of the renderer. The driver interface should be initialized by the engine
	m_RI = GetRI();
    m_RI->Init();

	//create_factories(settings, cmdline);
    _device = m_RI->m_Device.Get();
    m_DeviceCtx = m_RI->m_Context.Get();
    _factory = m_RI->m_Factory.Get();

	_debug_tool = std::make_unique<RendererDebugTool>(this);
	GameEngine::instance()->get_overlay_manager()->register_overlay(_debug_tool.get());

	m_CBGlobal = ConstantBuffer::create(m_RI, sizeof(GlobalCB), true, BufferUsage::Dynamic, nullptr);
	m_CBModel = ConstantBuffer::create(m_RI, sizeof(ModelCB), true, BufferUsage::Dynamic, nullptr);
	m_CBDebug = ConstantBuffer::create(m_RI, sizeof(DebugCB), true, BufferUsage::Dynamic, nullptr);
	m_CBPost = ConstantBuffer::create(m_RI, sizeof(PostCB), true, BufferUsage::Dynamic, nullptr);


	// Setup Light buffer
	{
		_light_buffer = GPUStructuredBuffer::create(m_RI, sizeof(ProcessedLight), c_MaxLights,true, BufferUsage::Dynamic);
	}

	// Create our cubemap 

	// #TODO: Enable after re-creating the cubemap files
#if 0
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
#endif

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
		m_ViewportPos = { 0, 0 };
		m_ViewportWidth = m_DrawableAreaWidth;
		m_ViewportHeight = m_DrawableAreaHeight;

		ResizeSwapchain(m_ViewportWidth, m_ViewportHeight);
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


	m_DeviceCtx->ClearState();
	m_DeviceCtx->Flush();

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

		ENSURE_HR(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flag, featureLevels, UINT(std::size(featureLevels)), D3D11_SDK_VERSION, &_device, &featureLevel, &m_DeviceCtx));

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
}

#if FEATURE_D2D
void Renderer::create_d2d_factory(EngineCfg const& settings)
{
	if (settings.m_UseD2D)
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
	//SafeRelease(_user_defined_annotation);

	//SafeRelease(m_DeviceCtx);
	//SafeRelease(_device);
	SafeRelease(_dwrite_factory);
	//SafeRelease(_wic_factory);
	SafeRelease(_d2d_factory);
	//SafeRelease(_factory);
}

void Renderer::release_frame_resources()
{
	// Release all the MSAA copies
	m_RI->ReleaseResource(_non_msaa_output_tex);
	m_RI->ReleaseResource(_non_msaa_output_tex_copy);
	m_RI->ReleaseResource(_non_msaa_output_srv);
	m_RI->ReleaseResource(_non_msaa_output_srv_copy);
	m_RI->ReleaseResource(_non_msaa_output_rtv);

	SafeRelease(_d2d_rt);

	SafeRelease(_swapchain);
	SafeRelease(_swapchain_rtv);
	SafeRelease(_swapchain_srv);

	m_OutputTexture = {};

	m_RI->ReleaseResource(_output_depth);
	m_RI->ReleaseResource(_output_depth_copy);
	m_RI->ReleaseResource(_output_depth_srv);
	m_RI->ReleaseResource(_output_depth_srv_copy);

	m_RI->ReleaseResource(_output_dsv);
}

void Renderer::ResizeSwapchain(u32 w, u32 h)
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

        m_OutputTexture.Create(output_desc, true, true, false);

		// This texture is used to output to a image in imgui
		output_desc.SampleDesc.Count = 1;
		output_desc.SampleDesc.Quality = 0;

        std::string debugName = "non msaa output";
        _non_msaa_output_tex = m_RI->CreateTexture(output_desc, nullptr, debugName);
        _non_msaa_output_srv = m_RI->CreateShaderResourceView(_non_msaa_output_tex, debugName);
		_non_msaa_output_rtv = m_RI->CreateRenderTargetView(_non_msaa_output_tex, debugName);

        debugName = "non msaa output copy";
        _non_msaa_output_tex_copy = m_RI->CreateTexture(output_desc, nullptr, debugName);
        _non_msaa_output_srv_copy = m_RI->CreateShaderResourceView(_non_msaa_output_tex_copy, debugName);
	}

	// Create the 3D depth target
	auto depth_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16_TYPELESS, w, h, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
    _output_depth = m_RI->CreateTexture(depth_desc, nullptr, "Main Depth");
    _output_depth_copy = m_RI->CreateTexture(depth_desc, nullptr, "Main Depth (COPY)");

	auto view_dim = D3D11_DSV_DIMENSION_TEXTURE2D;
	auto srv_view_dim = D3D11_SRV_DIMENSION_TEXTURE2D;
	if(_msaa != MSAAMode::Off)
	{
		view_dim = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		srv_view_dim = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	}

    ID3D11Texture2D* depth = m_RI->GetRawTexture2D(_output_depth);
	auto dsv_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(depth, view_dim, DXGI_FORMAT_D16_UNORM);
    _output_dsv = m_RI->CreateDepthStencilView(_output_depth, dsv_desc, "Renderer::Output Depth");

	auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(depth, srv_view_dim, DXGI_FORMAT_R16_UNORM);
    _output_depth_srv = m_RI->CreateShaderResourceView(_output_depth, srv_desc, "Renderer::Output Depth");
    _output_depth_srv_copy = m_RI->CreateShaderResourceView(_output_depth_copy, srv_desc, "Renderer::Output Depth(COPY)");

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
	m_RI->GetRawResource(m_OutputTexture.GetResource())->QueryInterface(surface.GetAddressOf());

	if (_d2d_factory)
	{
		SUCCEEDED(_d2d_factory->CreateDxgiSurfaceRenderTarget(surface.Get(), rtp, &_d2d_rt));
		set_debug_name(surface.Get(), "[D2D] Output");

		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
}

void Renderer::PreRender(RenderContext& ctx, RenderWorld const& world)
{
	JONO_EVENT();
	GPU_SCOPED_EVENT(&ctx, "PreRender");

	// Update the debug buffers
	{
		extern int g_DebugMode;
		DebugCB* debug_data = (DebugCB*)m_CBDebug->map(ctx);
		debug_data->m_VisualizeMode = g_DebugMode;
        m_CBDebug->unmap(ctx);
	}

	// Update the cameras
	for (auto& cam : world.get_cameras())
	{
		// Update our view camera to properly match the viewport aspect
		cam->set_aspect((f32)m_ViewportWidth / (f32)m_ViewportHeight);
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
		m_Visibility->reset();

		for (std::shared_ptr<RenderWorldInstance> const& inst : world.get_instances())
		{
			if (inst->is_ready())
			{
				m_Visibility->add_instance(inst.get());
			}
		}

		VisibilityParams params{};
		params.frustum[VisiblityFrustum_Main] = get_frustum_world(world, 0);

		for (int i = 0; i < MAX_CASCADES; ++i)
        {
			params.frustum[VisiblityFrustum_CSM0 + i] = Math::Frustum::from_vp(directional_light->get_cascade(i).vp);
        }
		m_Visibility->run(params);
	}

	// Light culling step is scheduled here.
	// 1. CPU frustum culling
	// 2. F+ tiled culling

	// 1. CPU Frustum culling
	{
		// Collect all local lights and update our light buffer
		ScopedBufferAccess access{ ctx, _light_buffer.get() };
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
			_per_tile_info_buffer = GPUByteBuffer::create(m_RI, tiles_x * tiles_y * sizeof(u32), false, BufferUsage::Default);
			_tile_light_index_buffer = GPUByteBuffer::create(m_RI, tiles_x * tiles_y * FPLUS_MAX_NUM_LIGHTS_PER_TILE * sizeof(u32), false, BufferUsage::Default);

			ShaderCreateParams cs_params = ShaderCreateParams::compute_shader("Source/Engine/shaders/ForwardPlus_Cull.hlsl");
			cs_params.params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
			_fplus_cull_shader = ShaderCache::instance()->find_or_create(cs_params);

			_fplus_cb = ConstantBuffer::create(GetRI(), sizeof(FPlusCB), true, BufferUsage::Dynamic);
		}


		ID3D11DeviceContext* dx11Ctx = m_DeviceCtx;

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

		//ctx.SetRenderTargets(0, nullptr, nullptr);
		dx11Ctx->OMSetRenderTargets(0, nullptr, nullptr);

		ComputeItem item{};
        item.srvs = {
			_light_buffer->get_srv(),
			_output_depth_srv
        };
        item.uavs = {
			_tile_light_index_buffer->get_uav(), 
			_per_tile_info_buffer->get_uav()
        };
        item.cbs = {
			_fplus_cb->get_buffer()
        };
        item.shader = _fplus_cull_shader->as<ID3D11ComputeShader>().Get();
        item.dispatchX = tiles_x;
        item.dispatchY = tiles_y;
		ctx.ExecuteComputeItem(item);
	}

}

void Renderer::DrawView(RenderContext& ctx, RenderWorld const& world, RenderPass::Value pass)
{
	GPU_SCOPED_EVENT(&ctx, "Renderer::render_view");

	// Setup global constant buffer
	std::shared_ptr<RenderWorldCamera> camera  = world.get_view_camera();

	ViewParams params{};
	params.proj = camera->get_proj();
	params.view = camera->get_view();
	params.view_position = camera->get_position();

	params.view_direction = camera->get_view_direction().xyz;
	params.pass = pass;

	// Viewport depends on the actual imgui window

	//CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(_output_tex, _output_rtv);
 //   params.viewport.x = viewport.TopLeftX;
 //   params.viewport.y = viewport.TopLeftY;
 //   params.viewport.width = viewport.Width;
 //   params.viewport.height = viewport.Height;
 //   params.viewport.minZ = viewport.MinDepth;
 //   params.viewport.maxZ = viewport.MaxDepth;

	// For the world we always render from the top left corner
	params.viewport.x = 0.0;
	params.viewport.y = 0.0;
	params.viewport.width = (f32)m_ViewportWidth;
	params.viewport.height = (f32)m_ViewportHeight;
    params.viewport.minZ = 0.0f;
    params.viewport.maxZ = 1.0f;

	DrawWorld(ctx, world, params);
}

void Renderer::DrawWorld(RenderContext& ctx, RenderWorld const& world, ViewParams const& params)
{
	std::string passName = RenderPass::ToString(params.pass);
	GPU_SCOPED_EVENT(&ctx, passName.c_str());

	// Setup some defaults. At the moment these are applied for each pass. However
	// ideally we would be able to have more detailed logic here to decided based on pass and mesh/material
	{
		DepthStencilState depth_stencil_state = DepthStencilState::Equal;
		if (params.pass == RenderPass::ZPrePass || RenderPass::IsShadowPass(params.pass))
		{
			depth_stencil_state = DepthStencilState::LessEqual;
		}

		std::array<GraphicsResourceHandle, 4> samplers = {
			Graphics::GetSamplerState(SamplerState::MinMagMip_Linear),
			Graphics::GetSamplerState(SamplerState::MinMagMip_Point),
			Graphics::GetSamplerState(SamplerState::MinMagMip_LinearClamp),
			Graphics::GetSamplerState(SamplerState::MinMagMip_PointClamp)
		};
        ctx.SetSamplers(ShaderStage::Pixel, 0, samplers);

		ctx.OMSetDepthStencilState(Graphics::GetDepthStencilState(depth_stencil_state), 0);
		ctx.RSSetState(Graphics::GetRasterizerState(RasterizerState::CullBack));
        ctx.OMSetBlendState(Graphics::GetBlendState(BlendState::Default), {}, 0xffffffff);
        ctx.RSSetViewports({ params.viewport });
	}

	GlobalCB* global = (GlobalCB*)m_CBGlobal->map(ctx);
	global->ambient.ambient = float4(0.02f, 0.02f, 0.02f, 1.0f);

	global->proj = params.proj;
	global->view = params.view;
	global->inv_view = hlslpp::inverse(params.view);
	global->inv_proj = hlslpp::inverse(params.proj);
	global->inv_view_projection = hlslpp::inverse(hlslpp::mul(params.view, params.proj));
	global->view_direction = float4(params.view_direction.xyz, 0.0f);

	global->view_pos = float4(params.view_position, 1.0f);
	global->vp.vp_top_x = params.viewport.x;
	global->vp.vp_top_y = params.viewport.y;
	global->vp.vp_half_width = params.viewport.width / 2.0f;
	global->vp.vp_half_height = params.viewport.height / 2.0f;

	global->num_directional_lights = std::min<u32>(u32(_num_directional_lights), MAX_DIRECTIONAL_LIGHTS);
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
					info->cascade_distance[j] = float4(z1);
					info->cascade[j] = l->get_cascade(j).vp;
				}
			}
			else
			{
				info->num_cascades = 0;
				for (int j = 0; j < MAX_CASCADES; ++j)
				{
					info->cascade_distance[j] = {};
					info->cascade[j] = {};
				}
			}
		}
	}
	m_CBGlobal->unmap(ctx);

	float4x4 vp = hlslpp::mul(params.view, params.proj);

	{
		JONO_EVENT("GenerateDrawCalls");

		VisibilityFrustum frustum = VisiblityFrustum_Main;
		if (RenderPass::IsShadowPass(params.pass))
		{
			frustum = (VisibilityFrustum)(VisiblityFrustum_CSM0 + (params.pass - RenderPass::Shadow_CSM0));
		}

		std::vector<RenderWorldInstance*> const& instances = m_Visibility->get_visible_instances(frustum);
		m_DrawCalls.reserve(instances.size());
		m_DrawCalls.clear();

		// #TODO: Pull this information from visibile lists
		for (RenderWorldInstance const* inst : instances)
		{
			// If the instance is ready we can consider adding it to the draw list
			if (inst->is_ready())
			{
				Model const* model = inst->_model->get();
				for (Mesh const& mesh : model->GetMeshes())
				{
					DrawCall dc{};
					dc._transform = inst->_transform;
                    dc._index_buffer = model->GetIndexBuffer();
                    dc._vertex_buffer = model->GetVertexBuffer();
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

	ID3D11DeviceContext* dx11Ctx = m_DeviceCtx;
    dx11Ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// #TODO: Remove debug handling into a debug lighting system
	extern int g_DebugMode;
	if (g_DebugMode != 0)
	{
		GraphicsResourceHandle buffers[3] = {
			m_CBGlobal->get_buffer(),
			m_CBDebug->get_buffer()
		};
        ctx.SetConstantBuffers(ShaderStage::Vertex | ShaderStage::Pixel, 0, buffers);
	}
	else
	{
		GraphicsResourceHandle buffers[1] = {
            m_CBGlobal->get_buffer(),
		};
		ShaderStage s = ShaderStage::Vertex | ShaderStage::Pixel;
        ctx.SetConstantBuffers(s, 0, buffers);
	}

	GraphicsResourceHandle prev_index = GraphicsResourceHandle::Invalid();
	GraphicsResourceHandle prev_vertex = GraphicsResourceHandle::Invalid();

	for (DrawCall const& dc : m_DrawCalls) 
	{
		ConstantBufferRef const& model_cb = m_CBModel;
		ModelCB* data = (ModelCB*)model_cb->map(ctx);
		data->world = dc._transform;
		data->wv = hlslpp::mul(data->world, params.view);
		data->wvp = hlslpp::mul(data->world, vp);
		model_cb->unmap(ctx);

		if(prev_index != dc._index_buffer)
		{
            ctx.IASetIndexBuffer(dc._index_buffer, DXGI_FORMAT_R32_UINT, 0);
			prev_index = dc._index_buffer;
		}

		if (prev_vertex != dc._vertex_buffer)
		{
            ctx.IASetVertexBuffers(0, { dc._vertex_buffer }, { sizeof(Model::VertexType) }, { 0 });

			prev_vertex = dc._vertex_buffer;
		}

		ID3D11Buffer* buffers[1] = 
		{
			m_RI->GetRawBuffer(model_cb->get_buffer())
		};
        dx11Ctx->VSSetConstantBuffers(2, 1, buffers);
        dx11Ctx->PSSetConstantBuffers(2, 1, buffers);

		// Setup the material render state
		{
			setup_renderstate(ctx, dc._material, params);

			++_stats.n_draws;
			_stats.n_primitives += u32(dc._index_count / 3);
            dx11Ctx->DrawIndexed((UINT)dc._index_count, (UINT)dc._first_index, (INT)dc._first_vertex);

		}
	}
}

void Renderer::PrepareShadowPass()
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

void Renderer::BeginFrame(RenderContext& ctx)
{
	JONO_EVENT();

	_stats = {};

	GameEngine* engine = GameEngine::instance();
	Viewport vp = Viewport{ 0.0f, 0.0f, engine->GetViewportSize().x, engine->GetViewportSize().y };

    ctx.BeginFrame();
	ctx.SetViewport(vp);
    ctx.ClearTargets(m_OutputTexture.GetRTV(), _output_dsv, float4(0.05f, 0.05f, 0.05f, 1.0f), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::EndFrame(RenderContext& ctx)
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

void Renderer::setup_renderstate(RenderContext& ctx, MaterialInstance const* mat_instance, ViewParams const& params)
{
	auto dx11Ctx = m_DeviceCtx;

	// Bind anything coming from the material
	mat_instance->apply(ctx, this, params);

	// Bind the global textures coming from rendering systems
	if (params.pass == RenderPass::Opaque)
	{
		ID3D11ShaderResourceView* views[] = {
			s_EnableShadowRendering ? _shadow_map_srv.Get() : nullptr,
			m_RI->GetRawSRV(_output_depth_srv_copy),
			_cubemap_srv.Get(),
			m_RI->GetRawSRV(_light_buffer->get_srv()),
			m_RI->GetRawSRV(_tile_light_index_buffer->get_srv()),
			m_RI->GetRawSRV(_per_tile_info_buffer->get_srv())
		};
		m_DeviceCtx->PSSetShaderResources(Texture_CSM, UINT(std::size(views)), views);
	}
}

void Renderer::VSSetShader(ShaderConstRef const& vertex_shader)
{
	if(m_PrevRenderState.VS != vertex_shader)
	{
		if (vertex_shader == nullptr)
		{
			m_DeviceCtx->VSSetShader(nullptr, nullptr, 0);
		}
		else
		{
			ASSERT(vertex_shader->get_type() == ShaderStage::Vertex);
			m_DeviceCtx->VSSetShader(vertex_shader->as<ID3D11VertexShader>().Get(), nullptr, 0);
		}
        m_PrevRenderState.VS = vertex_shader;
	}

}

void Renderer::PSSetShader(ShaderConstRef const& pixel_shader)
{
    if (m_PrevRenderState.PS != pixel_shader)
	{
		if(pixel_shader == nullptr)
		{
			m_DeviceCtx->PSSetShader(nullptr, nullptr, 0);
		}
		else
		{
			ASSERT(pixel_shader->get_type() == ShaderStage::Pixel);
			m_DeviceCtx->PSSetShader(pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
		}

		m_PrevRenderState.PS = pixel_shader;
	}
}

void Renderer::IASetInputLayout(ID3D11InputLayout* layout)
{
    if (m_PrevRenderState.InputLayout != layout)
	{
		m_DeviceCtx->IASetInputLayout(layout);
        m_PrevRenderState.InputLayout = layout;	
	}
}

void Renderer::RSSetState(ID3D11RasterizerState* state)
{
    if (m_PrevRenderState.RS != state)
	{
		m_DeviceCtx->RSSetState(state);
        m_PrevRenderState.RS = state;
	}
}

void Renderer::render_post_predebug(RenderContext& ctx)
{
	GPU_SCOPED_EVENT(&ctx, "Post:PreDebug");

	ShaderCreateParams params = ShaderCreateParams::pixel_shader("Source/Engine/Shaders/default_post_px.hlsl");
	ShaderRef post_shader = ShaderCache::instance()->find_or_create(params);

	params = ShaderCreateParams::vertex_shader("Source/Engine/Shaders/default_post_vx.hlsl");
	ShaderRef post_vs_shader = ShaderCache::instance()->find_or_create(params);

	ctx.IASetIndexBuffer(GraphicsResourceHandle::Invalid(), DXGI_FORMAT_UNKNOWN,0);
    ctx.IASetInputLayout(post_vs_shader->GetInputLayout());
	ctx.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	VSSetShader(post_vs_shader);
	PSSetShader(post_shader);

	ctx.SetShaderResources(ShaderStage::Pixel, 0, { _non_msaa_output_srv_copy });

    ID3D11Buffer* buffer = m_RI->GetRawBuffer(m_CBPost->get_buffer());
	m_DeviceCtx->PSSetConstantBuffers(0, 1, &buffer);

	GraphicsResourceHandle sampler_states[] = {
		Graphics::GetSamplerState(SamplerState::MinMagMip_Linear),
		Graphics::GetSamplerState(SamplerState::MinMagMip_Point)
	};
	ctx.SetSamplers(ShaderStage::Pixel, 0, sampler_states);
	m_DeviceCtx->Draw(3, 0);

}

void Renderer::render_post_postdebug(RenderContext& ctx)
{
	GPU_SCOPED_EVENT(&ctx, "Post:PostDebug");
}

void Renderer::DrawShadowPass(RenderContext& ctx, RenderWorld const& world)
{
	GPU_SCOPED_EVENT(&ctx, "Shadows");

	PrepareShadowPass();

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
        bool renderCascade[4] = { s_EnableCSM0, s_EnableCSM1, s_EnableCSM2, s_EnableCSM3 };
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{

			GPU_SCOPED_EVENT(&ctx, fmt::format("Cascade {}", i).c_str());
			CascadeInfo const& info = light->get_cascade(i);

			m_DeviceCtx->OMSetRenderTargets(0, nullptr, _shadow_map_dsv[i].Get());
			m_DeviceCtx->ClearDepthStencilView(_shadow_map_dsv[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            if (!renderCascade[i])
            {
                continue;
			}

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
			params.pass = RenderPass::Value(RenderPass::Shadow_CSM0 + i);
			params.viewport = Viewport(0.0f, 0.0f, 2048.0f, 2048.0f);
			DrawWorld(ctx, world, params);
		}
	}
}

void Renderer::DrawZPrePass(RenderContext& ctx, RenderWorld const& world)
{
	GPU_SCOPED_EVENT(&ctx, "zprepass");

    ctx.SetTarget(GraphicsResourceHandle::Invalid(), _output_dsv);

	DrawView(ctx, world, RenderPass::ZPrePass);
}

void Renderer::DrawOpaquePass(RenderContext& ctx, RenderWorld const& world)
{
	GPU_SCOPED_EVENT(&ctx, "opaque");

	// Do a copy to our dsv tex for sampling during the opaque pass
	CopyDepth();

	ctx.SetTarget(m_OutputTexture.GetRTV(), _output_dsv);
	DrawView(ctx, world, Graphics::RenderPass::Opaque);
}

void Renderer::DrawPost(RenderContext& ctx, RenderWorld const& world, shared_ptr<OverlayManager> const& overlays, bool doImgui)
{
	GPU_SCOPED_EVENT(&ctx, "Post");

	PostCB* data = (PostCB*)m_CBPost->map(ctx);
	data->m_ViewportWidth = (f32)(m_DrawableAreaWidth);
	data->m_ViewportHeight = (f32)(m_DrawableAreaHeight);
    m_CBPost->unmap(ctx);

	if (!_states)
	{
		_states = std::make_unique<DirectX::CommonStates>(_device);
		_common_effect = std::make_unique<DirectX::BasicEffect>(_device);
		_common_effect->SetVertexColorEnabled(true);

		void const* shader_byte_code;
		size_t byte_code_length;
		_common_effect->GetVertexShaderBytecode(&shader_byte_code, &byte_code_length);
		ENSURE_HR(_device->CreateInputLayout(DirectX::DX11::VertexPositionColor::InputElements, DirectX::DX11::VertexPositionColor::InputElementCount, shader_byte_code, byte_code_length, _layout.ReleaseAndGetAddressOf()));
	}

	m_DeviceCtx->OMSetBlendState(_states->Opaque(), nullptr, 0xFFFFFFFF);
	m_DeviceCtx->OMSetDepthStencilState(_states->DepthRead(), 0);
	m_DeviceCtx->RSSetState(_states->CullNone());

	m_DeviceCtx->IASetInputLayout(_layout.Get());

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
	_common_effect->Apply(m_DeviceCtx);

	if(overlays)
	{
		GPU_SCOPED_EVENT(&ctx, "Post:RenderOverlays");
		overlays->Render3D(m_DeviceCtx);
	}

	Viewport vp{};
	vp.width = static_cast<float>(m_ViewportWidth);
	vp.height = static_cast<float>(m_ViewportHeight);
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.minZ = 0.0f;
	vp.maxZ = 1.0f;
	ctx.SetViewport(vp);

	// Resolve msaa to non msaa for imgui render
	m_DeviceCtx->ResolveSubresource(GetRI()->GetRawResource(_non_msaa_output_tex), 0, GetRI()->GetRawResource(m_OutputTexture.GetResource()), 0, _swapchain_format);

	// Copy the non-msaa world render so we can sample from it in the post pass
	m_DeviceCtx->CopyResource(GetRI()->GetRawResource(_non_msaa_output_tex_copy), GetRI()->GetRawResource(_non_msaa_output_tex));
    ctx.SetTarget(_non_msaa_output_rtv, GraphicsResourceHandle::Invalid());
	render_post_predebug(ctx);

	render_post_postdebug(ctx);

	vp.width = static_cast<float>(m_DrawableAreaWidth);
	vp.height = static_cast<float>(m_DrawableAreaHeight);
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.minZ = 0.0f;
	vp.maxZ = 1.0f;
    ctx.SetViewport(vp);


	// Render main viewport ImGui
	if(doImgui)
	{
		GPU_SCOPED_EVENT(&ctx, "ImGui");
		m_DeviceCtx->OMSetRenderTargets(1, &_swapchain_rtv, nullptr);
		ImDrawData* imguiData = &GetGlobalContext()->m_GraphicsThread->m_FrameData.m_DrawData;
		ImGui_ImplDX11_RenderDrawData(imguiData);
	}

}

void Renderer::CopyDepth()
{
	m_DeviceCtx->CopyResource(m_RI->GetRawTexture2D(_output_depth_copy), m_RI->GetRawTexture2D(_output_depth));
}

} // namespace Graphics
