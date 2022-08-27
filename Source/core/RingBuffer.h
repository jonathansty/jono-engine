#pragma once

template <typename _Ty, size_t _Size>
class RingBuffer
{
public:
	using SelfType = RingBuffer<_Ty, _Size>;
	RingBuffer()
			: _start(0)
			, _end(0)
			, _data()
	{
	}

	~RingBuffer() {}

	void push(_Ty val)
	{
		// Copy the value to the end value
		_data[_end] = val;

		// Update our variables. If end runs over start we move start and overwrite the data
		_end = (_end + 1) % _Size;
		if (_end == _start)
		{
			_start = (_start + 1) % _Size;
		}
	}

	void clear()
	{
		_start = 0;
		_end = 0;
	}

	bool empty() const { return _start == _end; }

	template <typename _Ty, size_t _Size>
	struct Iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::atomic_ptrdiff_t;
		using value_type = _Ty;
		using pointer = _Ty const*;
		using reference = _Ty&;

		Iterator(RingBuffer<_Ty, _Size> const* owner, size_t idx)
				: _idx(idx)
				, _owner(owner)
		{
		}

		Iterator& operator++()
		{
			_idx = (_idx + 1) % _Size;
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp = *this;
			++(*this);
			return tmp;
		}

		pointer operator->()
		{
			return &_owner->_data[_idx];
		}

		pointer operator*()
		{
			return &_owner->_data[_idx];
		}

		friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs._idx == rhs._idx; }
		friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs._idx != rhs._idx; }

	private:
		size_t _idx;
		RingBuffer<_Ty, _Size> const* _owner;
	};

	Iterator<_Ty, _Size> begin() const { return Iterator<_Ty, _Size>(this, _start); }
	Iterator<_Ty, _Size> end() const { return Iterator<_Ty, _Size>(this, _end); }

private:
	size_t _start;
	size_t _end;
	std::array<_Ty, _Size> _data;
};
