#include "engine.pch.h"
#include "Perf.h"

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
	(ctx->GetData(_begin.Get(), &start, sizeof(UINT64), 0));
	(ctx->GetData(_end.Get(), &end, sizeof(UINT64), 0));

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
