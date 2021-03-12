#pragma once

namespace serialization {

// Writing binary
template <typename T>
inline void write(IO::IPlatformFileRef const& f, T const& obj) {
	f->write((void*)&obj, sizeof(T));
}

template <typename T>
inline void write(IO::IPlatformFileRef const& f, std::vector<T> const& obj) {
	uint32_t size = obj.size();
	f->write(&size, sizeof(uint32_t));
	f->write((void*)obj.data(), sizeof(T));
}

template <>
inline void write(IO::IPlatformFileRef const& f, std::string const& obj) {
	uint32_t size = obj.size();
	f->write(&size, sizeof(uint32_t));
	f->write((void*)obj.data(), sizeof(char) *size);
}

template<typename T>
inline void read(IO::IPlatformFileRef const& f, T& obj) {
	f->read((void*)&obj, sizeof(T));
}


// Writing of atomic types to binary
bool write_atomic_types(IO::IPlatformFileRef const& file, rttr::variant const& variant);

bool write_variant(IO::IPlatformFileRef const& file, rttr::variant const& variant);

bool write_instance(IO::IPlatformFileRef const& file, rttr::instance const& instance);

// Loading of atomic types from binary
bool load_instance(IO::IPlatformFileRef const& file, rttr::instance const& instance);

}

