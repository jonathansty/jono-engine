#pragma once

struct MemoryAllocationInfo
{
	size_t size;
};

enum class MemoryCategory : u32
{
	None,
	Core,
	Graphics,
	Game,
	ResourceLoading,
	Debug,
	Count
};
ENUM_UNDERLYING_TYPE(MemoryCategory)

const char* MemoryCategoryToString(MemoryCategory category);



class MemoryTracker final
{
public:

	std::atomic<s64> _current_allocs = 0;
	std::atomic<u64> _total_memory_usage = 0;

	std::atomic<u64> _total_allocated = 0;
	std::atomic<u64> _total_freed = 0;

	MemoryCategory _current_category = MemoryCategory::None;

	std::unordered_map<void*, MemoryAllocationInfo> _allocation_map[*MemoryCategory::Count];
	u64 _total_per_category[*MemoryCategory::Count];

	std::unordered_map<void*, MemoryCategory> _allocation_to_category;

	void TrackAllocation(void* mem, size_t size);
	void TrackDeallocation(void* mem);


	void* operator new(size_t size);
	void operator delete(void* mem);

	void DumpLeakInfo();

	std::mutex _tracking_lock;
};

MemoryTracker* get_memory_tracker();

struct MemoryTag final
{
	MemoryCategory _prev;
	MemoryTag(MemoryCategory category)
	{
		_prev = get_memory_tracker()->_current_category;
		get_memory_tracker()->_current_category = category;
		
	}

	~MemoryTag()
	{
		get_memory_tracker()->_current_category = _prev;
	}
};
#define MEMORY_TAG(cat) MemoryTag _mem_tag##__COUNTER__ = MemoryTag(cat)


