#pragma once

#include <string>

template <typename T> class Identifier;

template<typename T>
bool operator==(Identifier<T>const& lhs, Identifier<T> const& rhs);

template<typename T>
class Identifier
{
public:
	Identifier(T v)
			: _hash(v) 
	{}

	Identifier() = default;

	explicit Identifier(std::string const& id)
		: _hash(static_cast<T>(std::hash<std::string>{}(id)))
	{

	}

	Identifier(const char* id)
		: _hash(static_cast<T>(std::hash<std::string>{}(id)))
	{

	}


	~Identifier()
	{
	}

	static Identifier<T> create_guid();


	T get_hash() const { return _hash; }

	template<typename T>
	Identifier(Identifier<T> const& rhs) {
		this._hash = rhs._hash;
	}

	template<typename T>
	Identifier operator=(Identifier<T> const& rhs) {
		this._hash = rhs._hash;
	}

private:
	T _hash;

	friend struct std::hash<Identifier<T>>;

	template<typename Type>
	friend bool operator==(Identifier<Type>const& lhs, Identifier<Type> const& rhs);

};

template <typename T>
Identifier<T> Identifier<T>::create_guid() {
#if defined(WIN64)
	GUID g;
	::CoCreateGuid(&g);
	return Identifier64(GuidToString(g));
#else
	static_assert("Undefined");
#endif
}

template<typename Type>
bool operator==(Identifier<Type>const& lhs, Identifier<Type> const& rhs)
{
	return lhs._hash == rhs._hash;
}




namespace std
{
	template<typename T> struct hash<Identifier<T>>
	{
		std::size_t operator()(Identifier<T> const& s) const noexcept
		{
			return s._hash;
		}
	};
}

using Identifier32 = Identifier<uint32_t>;
using Identifier64 = Identifier<uint64_t>;