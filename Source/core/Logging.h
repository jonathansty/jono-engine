#pragma once

#include "singleton.h"

// Severity levels used for logging
enum class LogSeverity {
	Verbose,
	Info,
	Warning,
	Error
};


template<typename _Ty, size_t _Size>
class CircularBuffer {

public:
	using SelfType  = CircularBuffer<_Ty, _Size>;
	CircularBuffer() 
		: _start(0)
		, _end(0)
		, _data()
	{
	}

	~CircularBuffer() {}

	void push(_Ty val) {
		// Copy the value to the end value 
		_data[_end] = val;

		// Update our variables. If end runs over start we move start and overwrite the data
		_end = (_end + 1) % _Size;
		if(_end == _start) {
			_start = (_start + 1) % _Size;
		}

	}

	void clear() 
	{
		_start = 0;
		_end = 0;
	}

	bool empty() const { return _start == _end; }


	template<typename _Ty, size_t _Size>
	struct Iterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::atomic_ptrdiff_t;
		using value_type        = _Ty;
		using pointer           = _Ty const*;
		using reference         = _Ty&;

		Iterator(CircularBuffer<_Ty, _Size> const* owner, size_t idx)
		: _idx(idx)
		, _owner(owner)
		{

		}

		Iterator& operator++() {
			_idx = (_idx + 1) % _Size;
		}

		Iterator operator++(int) {
			Iterator tmp = *this;
			++(*this); 
			return tmp;
		}

		pointer operator->() {
			return &_owner->_data[_idx];
		}

		pointer operator*() {
			return &_owner->_data[_idx];
		}


		friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs._idx == rhs._idx; }
		friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs._idx != rhs._idx; }

		private:
			size_t _idx;
			CircularBuffer<_Ty, _Size> const* _owner;
	};

	Iterator<_Ty, _Size> begin() const { return Iterator<_Ty, _Size>(this, _start); }
	Iterator<_Ty, _Size> end() const { return Iterator<_Ty, _Size>(this, _end); }


private:
	size_t _start;
	size_t _end;
	std::array<_Ty, _Size> _data;

};

class Logging : public TSingleton<Logging> 
{
public:
	static constexpr int c_buffer_size = 2048;

	using Severity = LogSeverity;

	struct LogEntry {
		Severity _severity;
		std::string _message;
	};

	void Log(Severity severity, std::string const& msg) {
		Log({ severity, msg });
	}

	void Log(LogEntry const& entry) {
		_hasNewMessages = true;
		_buffer.push(entry);
	}

	CircularBuffer<LogEntry, c_buffer_size> const& GetBuffer() const { return _buffer; }

	bool _hasNewMessages = false;

private:
	CircularBuffer < LogEntry, c_buffer_size> _buffer;
};
