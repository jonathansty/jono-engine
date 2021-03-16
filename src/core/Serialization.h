#pragma once

#include "PlatformIO.h"

#include <assert.h>

namespace serialization {

// Writing binary
template <typename T>
inline void write(IO::IFileRef const& f, T const& obj) {
	f->write((void*)&obj, sizeof(T));
}

template <typename T>
inline void write(IO::IFileRef const& f, std::vector<T> const& obj) {
	uint32_t size = obj.size();
	f->write(&size, sizeof(uint32_t));
	f->write((void*)obj.data(), sizeof(T));
}

template <>
inline void write(IO::IFileRef const& f, std::string const& obj) {
	uint32_t size = static_cast<uint32_t>(obj.size());
	f->write(&size, sizeof(uint32_t));
	f->write((void*)obj.data(), sizeof(char) *size);
}

template<typename T>
inline T read(IO::IFileRef const& f) {
	T el{};
	f->read((void*)&el, sizeof(T));
	return el;
}

template <>
inline std::string read(IO::IFileRef const& f) {
	uint32_t size;
	f->read(&size, sizeof(uint32_t));

	char tmp[512];
	memset(tmp, 0, sizeof(char) * 512);
	assert(size < 512);
	f->read((void*)tmp, sizeof(char) * size);
	return tmp;
}


// Writing of atomic types to binary
bool write_atomic_types(IO::IFileRef const& file, rttr::variant const& variant);
bool write_instance(IO::IFileRef const& file, rttr::instance const& instance);

// Loading of atomic types from binary
rttr::variant read_atomic_types(IO::IFileRef const& file, rttr::type const& type);
bool read_instance(IO::IFileRef const& file, rttr::instance obj);


/// <summary>
/// Binary serialization construct that writes/reads directly to a binary file using raw memory copies.
/// This only works for built-in types and a select few implementations 
/// </summary>
/// <typeparam name="T">Type of the object to serialize</typeparam>
/// <param name="file"></param>
/// <param name="obj">Input/output data</param>
template <typename T, IO::Mode Enum>
void serialize(IO::IFileRef const& file, T& obj) {
	assert(file->is_binary());
	if constexpr (Enum == IO::Mode::Read) {
		obj = read<T>(file);
	}
	else {
		write<T>(file, obj);
	}
}

/// <summary>
/// Serializes `instance` into `file`. Only supports serializing if the file was opened as binary. 
/// It asserts when a file is not opened as binary.
/// </summary>
/// <param name="file">The file to serialize from/to</param>
/// <param name="instance">The data to serialize</param>
/// <returns>Boolean to indicate success or failure</returns>
template <IO::Mode Mode>
bool serialize_instance(IO::IFileRef const& file, rttr::instance instance) {
	assert(file->is_binary() && Mode == file->get_mode());

	// Read type hash of the instance
	rttr::instance obj = instance.get_type().get_raw_type().is_wrapper() ? instance.get_wrapped_instance() : instance;
	rttr::type t = obj.get_derived_type().get_raw_type();

	u64 type_hash = Identifier64(t.get_name().begin()).get_hash();
	serialize<u64, Mode>(file, type_hash);

	u32 n_properties = t.get_properties().size();
	serialize<u32, Mode>(file, n_properties);

	// Make sure that the ID matches
	Identifier64 id = Identifier64(t.get_name().begin());
	assert(type_hash == id.get_hash());

	auto it_prop = t.get_properties().begin();
	for (u32 i = 0; i < n_properties; ++i) {

		// First use the property retrieved from the type. If we are 
		// writing we want to write the correct property, when 
		// reading this will get overriden with the right property from the file.
		std::string name = it_prop->get_name().begin();
		serialize<std::string, Mode>(file, name);

		// Check that our runtime type contains the property
		rttr::property prop = t.get_property(name);
		if (!prop.is_valid()) {
			continue;
		}


		// todo(jonathan): refactor this code to possibly not have constexprs in this function
		rttr::type const& t_prop = prop.get_type();
		if constexpr(Mode == IO::Mode::Read) {
			if (rttr::variant value = read_atomic_types(file, t_prop); value.is_valid()) {
				if (value.convert(t_prop)) {
					prop.set_value(obj, value);
				}
				continue;
			} else { // Handle the properties
				rttr::variant val = prop.get_value(obj);
				serialize_instance<Mode>(file, val);
				prop.set_value(obj, val);
			}
		}
		else if constexpr(Mode == IO::Mode::Write) {
			rttr::variant prop_value = prop.get_value(obj);
			if(write_atomic_types(file, prop_value)) {
			} else if (serialize_instance<Mode>(file, prop_value)) {
			} else {
				return false;
			}
		}

		++it_prop;
	}

	return true;
}

}

