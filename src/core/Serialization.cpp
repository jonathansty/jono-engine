#include <rttr/registration>
#include <rttr/type>
#include <rttr/registration_friend>
#include <hlsl++.h>
#include <fmt/core.h>

#include <iostream>

#include "Types.h"
#include "Identifier.h"

#include "Serialization.h"

using hlslpp::float4;
using hlslpp::float3;


bool serialization::write_atomic_types(IO::IFileRef const& file, rttr::variant const& variant) {
	auto const& t = variant.get_type();
	auto const& var = variant;

	using namespace rttr;
	if (t.is_arithmetic()) {
		if (t == type::get<bool>()) {
			write(file, var.to_bool());
		} else if (t == type::get<char>()) {
			write(file, var.to_bool());
		} else if (t == type::get<int8_t>()) {
			write(file, var.to_int8());
		} else if (t == type::get<int16_t>()) {
			write(file, var.to_int16());
		} else if (t == type::get<int32_t>()) {
			write(file, var.to_int32());
		} else if (t == type::get<int64_t>()) {
			write(file, var.to_int64());
		} else if (t == type::get<uint8_t>()) {
			write(file, var.to_uint8());
		} else if (t == type::get<uint16_t>()) {
			write(file, var.to_uint16());
		} else if (t == type::get<uint32_t>()) {
			write(file, var.to_uint32());
		} else if (t == type::get<uint64_t>()) {
			write(file, var.to_uint64());
		} else if (t == type::get<float>()) {
			write(file, var.to_float());
		} else if (t == type::get<double>()) {
			write(file, var.to_double());
		}
		return true;
	} else if (t.is_enumeration()) {
		bool ok = false;
		auto value = var.to_uint64(&ok);
		if (ok) {
			write(file, value);
		}
		return true;
	} else if (t == type::get<std::string>()) {
		write(file, var.to_string());
		return true;
	} else if (t == type::get<WrapperFloat4>()) {
		float4 value = var.get_value<WrapperFloat4>().value;
		write(file, float(value.x));
		write(file, float(value.y));
		write(file, float(value.z));
		write(file, float(value.w));
		return true;
	} else if (t == type::get<WrapperFloat3>()) {
		float3 value = var.get_value<WrapperFloat3>().value;
		write(file, float(value.x));
		write(file, float(value.y));
		write(file, float(value.z));
		return true;
	}


	return false;
}

bool serialization::write_instance(IO::IFileRef const& file, rttr::instance const& instance) {
	rttr::instance obj = instance.get_type().get_raw_type().is_wrapper() ? instance.get_wrapped_instance() : instance;
	rttr::type t = obj.get_type().get_raw_type();
	rttr::array_range<rttr::property> properties = obj.get_derived_type().get_properties();

	// Hash our type and serialize the hash into the binary for lookup later
	Identifier64 type_hash = Identifier64(t.get_name().begin());
	uint32_t n_properties = properties.size();

	write(file, type_hash.get_hash());
	write(file, n_properties);

	for (auto const& prop : properties) {
		rttr::variant prop_value = prop.get_value(instance);
		assert(prop_value.is_valid() && "Property does not have a value on this instance of the object. Check passed in instance for correctness!" );

		// Write out the property name
		const auto prop_name = prop.get_name();
		write(file, prop_name.to_string());

		try {
			if (write_atomic_types(file, prop_value)) {} 
			else if (write_instance(file, prop_value)) {}
			else
				throw std::exception("Failed to serialize property.");

		} catch (std::exception e) {
			std::cerr << e.what() << std::endl;
		}
	}

	return true;
}

rttr::variant serialization::read_atomic_types(IO::IFileRef const& file, rttr::type const& t) {
	rttr::variant variant;

	using namespace rttr;
	if (t.is_arithmetic()) {
		if (t == type::get<bool>()) {
			variant = read<bool>(file);
		} else if (t == type::get<char>()) {
			variant = read<char>(file);
		} else if (t == type::get<int8_t>()) {
			variant = read<int8_t>(file);
		} else if (t == type::get<int16_t>()) {
			variant = read<int16_t>(file);
		} else if (t == type::get<int32_t>()) {
			variant = read<int32_t>(file);
		} else if (t == type::get<int64_t>()) {
			variant = read<int64_t>(file);
		} else if (t == type::get<uint8_t>()) {
			variant = read<uint8_t>(file);
		} else if (t == type::get<uint16_t>()) {
			variant = read<uint16_t>(file);
		} else if (t == type::get<uint32_t>()) {
			variant = read<uint32_t>(file);
		} else if (t == type::get<uint64_t>()) {
			variant = read<uint64_t>(file);
		} else if (t == type::get<float>()) {
			variant = read<float>(file);
		} else if (t == type::get<double>()) {
			variant = read<double>(file);
		}
	} else if (t.is_enumeration()) {
		variant = read<uint64_t>(file);
	} else if (t == type::get<std::string>()) {
		variant = read<std::string>(file);
	} else if (t == type::get<WrapperFloat4>()) {
		float4 value{};
		value.x = read<float>(file);
		value.y = read<float>(file);
		value.z = read<float>(file);
		value.w = read<float>(file);
		WrapperFloat4 result;
		result.value = value;
		variant = result;
	} else if (t == type::get<WrapperFloat3>()) {
		float3 value = {};
		value.x = read<float>(file);
		value.y = read<float>(file);
		value.z = read<float>(file);
		WrapperFloat3 result;
		result.value = value;
		variant = result;
	}

	return variant;
}

bool serialization::read_instance(IO::IFileRef const& file, rttr::instance instance) {

	// Read type hash of the instance
	u64 type_hash = read<u64>(file);
	u32 n_properties = read<u32>(file);

	rttr::instance obj = instance.get_type().get_raw_type().is_wrapper() ? instance.get_wrapped_instance() : instance;
	rttr::type t = obj.get_type().get_raw_type();

	// Make sure that the ID matches
	Identifier64 id = Identifier64(t.get_name().begin());
	assert(type_hash == id.get_hash());

	for (u32 i = 0; i < n_properties; ++i) {

		std::string name = read<std::string>(file);

		// Check that our runtime type contains the property
		rttr::property prop = t.get_property(name);
		if (!prop.is_valid()) {
			continue;
		}

		// Extract atomic types first
		rttr::type const& t_prop = prop.get_type();
		if (rttr::variant value = read_atomic_types(file, t_prop);  value.is_valid()) {
			if (value.convert(t_prop)) {
				prop.set_value(obj, value);
			}
			continue;
		} else { // Handle the properties
			rttr::variant val = prop.get_value(obj);
			read_instance(file, val);
			prop.set_value(obj, val);
		}

	}


	return true;
}

