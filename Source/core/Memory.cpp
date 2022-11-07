#include "Core.pch.h"
#include "Memory.h"
#include <source_location>

std::atomic<bool> s_initialised = false;
char g_reserved_memory[sizeof(MemoryTracker)] = {};
thread_local bool s_should_track = true;
thread_local static MemoryTracker* g_tracker = nullptr;

bool is_tracker_initialised()
{
	return s_initialised;
}

MemoryTracker* get_memory_tracker()
{
	if(g_tracker == nullptr)
	{
		g_tracker = (MemoryTracker*)g_reserved_memory;
	}

	return  g_tracker;
}

void* operator new(size_t size)
{
	void* result = std::malloc(size);

	if(s_initialised)
	{
		get_memory_tracker()->TrackAllocation(result, size);
	}
	return result;
}

void* operator new[](size_t size)
{
	void* result = std::malloc(size);

	if (s_initialised)
	{
		get_memory_tracker()->TrackAllocation(result, size);
	}
	return result;
}


void operator delete(void* mem) 
{
	if (s_initialised)
	{
		get_memory_tracker()->TrackDeallocation(mem);
	}
	std::free(mem);
}

void operator delete[](void* mem) 
{
	if (s_initialised)
	{
		get_memory_tracker()->TrackDeallocation(mem);
	}
	std::free(mem);
}


const char* MemoryCategoryToString(MemoryCategory category)
{
	static const char* s_values[] = {
		"None",
		"Core",
		"Graphics",
		"Game",
		"ResourceLoading",
		"Debug"
	};
	return s_values[*category];
}


void MemoryTracker::init()
{
	if (!s_initialised)
	{
		new (g_reserved_memory) MemoryTracker();
		s_initialised = true;
	}
}

bool MemoryTracker::should_track() const
{
	return s_initialised&& s_should_track;
}

struct ScopedMemTrackDisable
{
	ScopedMemTrackDisable()
	{
		s_should_track = false;
	}
	~ScopedMemTrackDisable()
	{
	
		s_should_track = true;
	}

};
void MemoryTracker::TrackAllocation(void* mem, size_t size)
{
	if (!should_track())
		return;

	std::lock_guard lock{ _tracking_lock };
	ScopedMemTrackDisable mem_track_disable;

	// If this is not in a category then we didn't track this allocation
	{
		_allocation_to_category[mem] = _current_category;

		_current_allocs += 1;
		_total_memory_usage += size;
		_total_allocated += size;
		_total_per_category[*_current_category] += size;

		MemoryAllocationInfo info{};
		info.size = size;
		_allocation_map[*_current_category][mem] = std::move(info);
	}

}

void MemoryTracker::TrackDeallocation(void* mem)
{
	if (!should_track())
		return;

	std::lock_guard lock{ _tracking_lock };
	ScopedMemTrackDisable mem_track_disable;

	if (auto it = _allocation_to_category.find(mem); it != _allocation_to_category.end())
	{
		MemoryCategory const& category = _allocation_to_category[mem];
		MemoryAllocationInfo const& info = _allocation_map[*category][mem];

		_total_memory_usage -= info.size;
		_total_freed += info.size;
		_total_per_category[*category] -= info.size;
		_current_allocs -= 1;

		_allocation_map[*category].erase(mem);
		_allocation_to_category.erase(mem);
	}
}

void MemoryTracker::DumpLeakInfo()
{
	//for(u32 i = 0; i < *MemoryCategory::Count; ++i)
	//{
	//	if (!_allocation_map[i].empty())
	//	{
	//		std::vector<std::pair<void*, MemoryAllocationInfo>> sorted;
	//		sorted.reserve(_allocation_map[i].size());
	//		std::for_each(_allocation_map[i].begin(), _allocation_map[i].end(), [&sorted](auto const& pair)
	//			{ sorted.push_back(pair); });

	//		std::sort(sorted.begin(), sorted.end(), [](auto const& lhs, auto const& rhs) {
	//					return lhs.second.size < rhs.second.size;
	//		});

	//		FAILMSG("Memory leaks detected.");

	//		fmt::print("Total Allocations: {}\n", _current_allocs);
	//		// Dump memory leaks
	//		for (std::pair<void*, MemoryAllocationInfo> mem : sorted)
	//		{
	//			fmt::print("Add: {} | Size: {} \n", mem.first, mem.second.size);
	//		}
	//	}
	//}
}
