#include "engine.pch.h"

#include "Graphics.h"
#include "Renderer.h"

namespace Graphics
{

DeviceContext s_ctx;

std::array<ComPtr<ID3D11DepthStencilState>, DepthStencilState::Num> s_depth_stencil_states;
std::array<ComPtr<ID3D11BlendState>, BlendState::Num> s_blend_states;
std::array<ComPtr<ID3D11RasterizerState>, RasterizerState::Num> s_raster_states;
std::array<ComPtr<ID3D11SamplerState>, SamplerState::Num> s_sampler_states;

void init(DeviceContext const& ctx)
{
	assert(ctx._device.Get() && ctx._ctx.Get());
	s_ctx = ctx;

	auto device = s_ctx._device;

	// Depth Stencil
	{
		CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
		ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[DepthStencilState::GreaterEqual].GetAddressOf()));
		helpers::SetDebugObjectName(s_depth_stencil_states[DepthStencilState::GreaterEqual].Get(), "GreaterEqual");

		ds_desc.DepthFunc = D3D11_COMPARISON_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[DepthStencilState::Equal].GetAddressOf()));
		helpers::SetDebugObjectName(s_depth_stencil_states[DepthStencilState::Equal].Get(), "Equal");

		ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[DepthStencilState::LessEqual].GetAddressOf()));
		helpers::SetDebugObjectName(s_depth_stencil_states[DepthStencilState::LessEqual].Get(), "LessEqual");
	}

	// Blend states
	{
		CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
		SUCCEEDED(device->CreateBlendState(&bs_desc, s_blend_states[0].GetAddressOf()));
		helpers::SetDebugObjectName(s_blend_states[BlendState::Default].Get(), "Default");
	}

	// Rasterizer states
	{
		CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
		rs_desc.CullMode = D3D11_CULL_NONE;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[RasterizerState::CullNone].GetAddressOf()));
		helpers::SetDebugObjectName(s_raster_states[RasterizerState::CullNone].Get(), "CullNone");

		rs_desc.CullMode = D3D11_CULL_FRONT;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[RasterizerState::CullFront].GetAddressOf()));
		helpers::SetDebugObjectName(s_raster_states[RasterizerState::CullFront].Get(), "CullFront");

		rs_desc.CullMode = D3D11_CULL_BACK;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[RasterizerState::CullBack].GetAddressOf()));
		helpers::SetDebugObjectName(s_raster_states[RasterizerState::CullBack].Get(), "CullBack");
	}

	// Samplers
	{
		CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SUCCEEDED(device->CreateSamplerState(&sampler, s_sampler_states[SamplerState::MinMagMip_Linear].GetAddressOf()));
	}
}

void deinit()
{
	auto clear_fn = []<typename T>(T& ptr)
	{
		ptr.Reset();
	};
	std::for_each(s_depth_stencil_states.begin(), s_depth_stencil_states.end(), clear_fn);
	std::for_each(s_blend_states.begin(), s_blend_states.end(), clear_fn);
	std::for_each(s_raster_states.begin(), s_raster_states.end(), clear_fn);
	std::for_each(s_sampler_states.begin(), s_sampler_states.end(), clear_fn);

	s_ctx = {};
}

ComPtr<ID3D11Device> get_device()
{
	return s_ctx._device;
}

ComPtr<ID3D11DeviceContext> get_ctx()
{
	return s_ctx._ctx;
}

ComPtr<ID3D11BlendState> get_blend_state(BlendState::Value blendState)
{
	return s_blend_states[blendState];
}

ComPtr<ID3D11RasterizerState> get_rasterizer_state(RasterizerState::Value rasterizerState)
{
	return s_raster_states[rasterizerState];
}

ComPtr<ID3D11DepthStencilState> get_depth_stencil_state(DepthStencilState::Value depthStencilState)
{
	return s_depth_stencil_states[depthStencilState];
}

ComPtr<ID3D11SamplerState> get_sampler_state(SamplerState::Value samplerState)
{
	return s_sampler_states[samplerState];
}

} // namespace Graphics

std::shared_ptr<ConstantBuffer> ConstantBuffer::create(ID3D11Device* device, u32 size, bool cpu_write /*= false*/, BufferUsage usage /*= BufferUsage::Default*/, void* initialData /*= nullptr*/)
{
	std::shared_ptr<ConstantBuffer> result = std::make_shared<ConstantBuffer>();

	D3D11_BUFFER_DESC buff{};
	buff.ByteWidth = size;
	buff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (cpu_write)
		buff.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

	buff.Usage = D3D11_USAGE_DEFAULT;
	switch (usage)
	{
		case BufferUsage::Dynamic:
			buff.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case BufferUsage::Staging:
			buff.Usage = D3D11_USAGE_STAGING;
			break;
		case BufferUsage::Immutable:
			buff.Usage = D3D11_USAGE_IMMUTABLE;
			break;
	}
	buff.StructureByteStride = 0;
	buff.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = initialData;

	ComPtr<ID3D11Buffer> b;
	if (initialData)
		SUCCEEDED(device->CreateBuffer(&buff, &data, &b));
	else
		SUCCEEDED(device->CreateBuffer(&buff, nullptr, &b));
	result->_buffer = b;
	result->_size = size;
	result->_cpu_writeable = cpu_write;
	result->_usage = usage;

	return result;
}

namespace Perf
{

constexpr u32 s_frames = 2;
static_assert(s_frames >= 2, "Number of buffered frames need to be atleast 2 to implement a double buffering strategy.");

// Query stack to keep track of our disjoint queries
ComPtr<ID3D11Query> s_disjoint_query[s_frames];

// Global state to track performance related states
s64 s_frame = 0;
s64 s_previous[s_frames] = {};

void Timer::begin(ComPtr<ID3D11DeviceContext> const& ctx)
{
	// When the timer is not flushed this means we haven't retrieved the data. This is not valid behaviour.
	assert(_flushed);
	_flushed = false;

	_timer.reset();
	_timer.start();

	ctx->End(_begin.Get());
}

void Timer::end(ComPtr<ID3D11DeviceContext> const& ctx)
{
	_timer.stop();

	ctx->End(_end.Get());
}

void Timer::flush(ComPtr<ID3D11DeviceContext> const& ctx, UINT64& start, UINT64& end, f64& cpuTime)
{
	_flushed = true;
	ENSURE_HR(ctx->GetData(_begin.Get(), &start, sizeof(UINT64), 0));
	ENSURE_HR(ctx->GetData(_end.Get(), &end, sizeof(UINT64), 0));

	cpuTime = _timer.get_delta_time();
}

void initialize(ComPtr<ID3D11Device> const& device)
{
	D3D11_QUERY_DESC desc{};
	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	desc.MiscFlags = 0;
	for (u32 i = 0; i < s_frames; ++i)
	{
		device->CreateQuery(&desc, s_disjoint_query[i].ReleaseAndGetAddressOf());
	}
}

void shutdown()
{
	for (u32 i = 0; i < s_frames; ++i)
	{
		s_disjoint_query[i].Reset();
	}
}

void begin_frame(ComPtr<ID3D11DeviceContext> const& ctx)
{
	ctx->Begin(s_disjoint_query[get_current_frame_resource_index()].Get());
}

void end_frame(ComPtr<ID3D11DeviceContext> const& ctx)
{
	ctx->End(s_disjoint_query[get_current_frame_resource_index()].Get());

	// Update our frame state
	for (int i = 0; i < s_frames - 1; ++i)
	{
		int curr = s_frames - 1 - i;
		int prev = s_frames - 2 - i;
		s_previous[curr] = s_previous[prev];
	}
	s_previous[0] = (s_previous[0] + 1) % s_frames;

	++s_frame;
}

bool collect_disjoint(ComPtr<ID3D11DeviceContext> const& ctx, D3D11_QUERY_DATA_TIMESTAMP_DISJOINT& disjoint)
{
	// TODO: Figure out how to really make this double buffered. In D3D11 our previous frame timers aren't even really done after executing 1 frame
	s64 previous = get_previous_frame_resource_index();
	while (S_OK != ctx->GetData(s_disjoint_query[previous].Get(), &disjoint, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0))
	{
		Perf::precise_sleep(0.001);
	}
	return !disjoint.Disjoint;
}

bool can_collect()
{
	return s_frame > (s_frames - 2);
}

s64 get_frame_count()
{
	return s_frame;
}

s64 get_current_frame_resource_index()
{
	return s_previous[0];
}

s64 get_previous_frame_resource_index()
{
	return s_previous[s_frames - 1];
}

} // namespace Perf
