#include "Core.pch.h"
#include "Memory.h"
#include <source_location>

static MemoryTracker* g_Tracker = new MemoryTracker();

thread_local  std::atomic<bool> s_InsideMemoryTracker = false;

void* operator new(size_t size)
{
	void* result = std::malloc(size);

	if (g_Tracker && !s_InsideMemoryTracker)
	{
		s_InsideMemoryTracker = true;
		g_Tracker->TrackAllocation(result, size);
		s_InsideMemoryTracker = false;
	}
	return result;
}

void* operator new[](size_t size)
{
	void* result = std::malloc(size);

	if (g_Tracker && !s_InsideMemoryTracker)
	{
		s_InsideMemoryTracker = true;
		g_Tracker->TrackAllocation(result, size);
		s_InsideMemoryTracker = false;
	}
	return result;
}


void operator delete(void* mem) 
{

	if (g_Tracker && !s_InsideMemoryTracker)
	{
		s_InsideMemoryTracker = true;
		g_Tracker->TrackDeallocation(mem);
		s_InsideMemoryTracker = false;
	}

	std::free(mem);
}

void operator delete[](void* mem) 
{
	if (g_Tracker && !s_InsideMemoryTracker)
	{
		s_InsideMemoryTracker = true;
		g_Tracker->TrackDeallocation(mem);
		s_InsideMemoryTracker = false;
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

MemoryTracker* get_memory_tracker()
{
	return g_Tracker;
}

void MemoryTracker::TrackAllocation(void* mem, size_t size)
{
	std::lock_guard lock{ g_Tracker->_tracking_lock };
	_allocation_to_category[mem] = _current_category;

	_current_allocs += 1;
	_total_memory_usage += size;
	_total_allocated += size;
	_total_per_category[*_current_category] += size;

	MemoryAllocationInfo info{};
	info.size = size;
	_allocation_map[*_current_category][mem] = std::move(info);
}

void MemoryTracker::TrackDeallocation(void* mem)
{
	std::lock_guard lock{ g_Tracker->_tracking_lock };

	MemoryCategory const& category = _allocation_to_category[mem];
	MemoryAllocationInfo const& info = _allocation_map[*category][mem];

	_total_memory_usage -= info.size;
	_total_freed += info.size;
	_total_per_category[*category] -= info.size;
	_current_allocs -= 1;

	_allocation_map[*category].erase(mem);
	_allocation_to_category.erase(mem);
}

void* MemoryTracker::operator new(size_t size)
{
	return std::malloc(size);
}

void MemoryTracker::operator delete(void* mem)
{
	std::free(mem);
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
