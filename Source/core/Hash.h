#pragma once

#include "Span.h"

namespace Hash
{
template <class T>
inline void combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher{};
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

const uint32_t Prime = 0x01000193;
const uint32_t Seed = 0x811C9DC5;

inline uint32_t fnv1a(unsigned char b, uint32_t hash = Seed)
{
	return (b ^ hash) * Prime;
}

inline uint32_t fnv1a(const void* data, size_t numBytes, uint32_t hash = Seed)
{
	assert(data);
	const unsigned char* ptr = reinterpret_cast<const unsigned char*>(data);
	while (numBytes--)
	{
		hash = (*ptr++ ^ hash) * Prime;
	}
	return hash;
}
template <typename T>
inline uint32_t fnv1a(T const& data, uint32_t hash = Seed)
{
	return fnv1a((void*)&data, sizeof(T), hash);
}

template <typename T>
inline uint32_t fnv1a(Span<T> const& data, uint32_t hash = Seed)
{
	if (data.empty())
		return hash;

	return fnv1a(data.data(), data.size() * sizeof(T), hash);
}

inline uint32_t fnv1a(std::string const& data, uint32_t hash = Seed)
{
	return fnv1a(data.data(), data.size() * sizeof(data[0]), hash);
}

} // namespace Hash