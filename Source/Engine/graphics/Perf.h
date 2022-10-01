#pragma once

#include "PrecisionTimer.h"

namespace Perf
{

inline void sleep(u32 ms)
{
	Sleep(ms);
}

// Blog post talking about the inaccuracy of the windows sleep function and how they arrived at this solution
// https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
inline void precise_sleep(f64 seconds)
{
	using namespace std;
	using namespace std::chrono;

	static f64 estimate = 5e-3;
	static f64 mean = 5e-3;
	static f64 m2 = 0;
	static int64_t count = 1;

	while (seconds > estimate)
	{
		auto start = high_resolution_clock::now();
		this_thread::sleep_for(milliseconds(1));
		auto end = high_resolution_clock::now();

		f64 observed = (end - start).count() / 1e9;
		seconds -= observed;

		++count;
		double delta = observed - mean;
		mean += delta / count;
		m2 += delta * (observed - mean);
		f64 stddev = sqrt(m2 / (count - 1));
		estimate = mean + stddev;
	}

	// spin lock
	auto start = high_resolution_clock::now();
	while ((high_resolution_clock::now() - start).count() / 1e9 < seconds)
		;
}

// Initializes our buffer with query objects for usage each frame
void initialize(ComPtr<ID3D11Device> const& device);

// Frees the query objects
void shutdown();

// inserts a begin call for our disjoint query
void begin_frame(ComPtr<ID3D11DeviceContext> const& ctx);

// inserts a end call for our disjoint query
void end_frame(ComPtr<ID3D11DeviceContext> const& ctx);

// Waits and collects the disjoint data
bool collect_disjoint(ComPtr<ID3D11DeviceContext> const& ctx, D3D11_QUERY_DATA_TIMESTAMP_DISJOINT& disjoint);

// Indicates if we can already collect our disjoint data
bool can_collect();

// Retrieves the amount of frames tracked in our perf layer
s64 get_frame_count();

// Retrieves the current frame it's resource index.
//
// The resource index being the index into our N buffered arrays
s64 get_current_frame_resource_index();

// Retrieves the previous frame it's resource index.
//
// The resource index being the index into our N buffered arrays
s64 get_previous_frame_resource_index();

// Timer that exposes CPU and GPU functionality
class Timer
{
public:
	Timer()
	{
	}

	Timer(ComPtr<ID3D11Device> const& device)
	{
		D3D11_QUERY_DESC desc{};
		desc.Query = D3D11_QUERY_TIMESTAMP;
		desc.MiscFlags = 0;
		device->CreateQuery(&desc, _begin.ReleaseAndGetAddressOf());
		device->CreateQuery(&desc, _end.ReleaseAndGetAddressOf());
	}

	~Timer() {}

	void begin(ComPtr<ID3D11DeviceContext> const& ctx);
	void end(ComPtr<ID3D11DeviceContext> const& ctx);

	void flush(ComPtr<ID3D11DeviceContext> const& ctx, UINT64& begin, UINT64& end, f64& cpuTime);

private:
	ComPtr<ID3D11Query> _begin;
	ComPtr<ID3D11Query> _end;
	bool _flushed = true;

	PrecisionTimer _timer;
};

// ---------------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------------

#define ENABLE_PROFILING

#ifdef ENABLE_PROFILING
class scoped_gpu_event final
{
public:
	scoped_gpu_event(ID3DUserDefinedAnnotation* annotation, std::wstring name)
			: _name(name), _annotation(annotation)
	{
		annotation->BeginEvent(name.c_str());
	}
	scoped_gpu_event(ID3DUserDefinedAnnotation* annotation, std::string name)
			: _annotation(annotation)
	{
		_name = std::wstring(name.begin(), name.end());
		annotation->BeginEvent(_name.c_str());
	}
	~scoped_gpu_event()
	{
		_annotation->EndEvent();
	}

private:
	std::wstring _name;
	ID3DUserDefinedAnnotation* _annotation;
};

#define COMBINE1(X, Y) X##Y // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)
#define GPU_SCOPED_EVENT(ctx, name) OPTICK_EVENT_DYNAMIC(name); Perf::scoped_gpu_event COMBINE(perfEvent, __LINE__) = Perf::scoped_gpu_event(ctx, name)
#define GPU_MARKER(ctx, name) ctx->SetMarker(name);
#else
#define GPU_SCOPED_EVENT(ctx, name)
#define GPU_MARKER(ctx, name)
#endif

} // namespace Perf
