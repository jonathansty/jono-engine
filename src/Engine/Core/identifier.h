#pragma once

template <typename T> class Identifier;

template<typename T>
bool operator==(Identifier<T>const& lhs, Identifier<T> const& rhs);

template<typename T>
class Identifier
{
public:
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

	//bool operator==(Identifier<T> const& rhs)
	//{
	//	return _hash == rhs._hash;
	//}

private:
	T _hash;

	friend struct std::hash<Identifier<T>>;

	template<typename T>
	friend bool operator==(Identifier<T>const& lhs, Identifier<T> const& rhs);

};

template<typename T>
bool operator==(Identifier<T>const& lhs, Identifier<T> const& rhs)
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