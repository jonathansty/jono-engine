#include "testbed.stdafx.h"
#include "Serialization.h"

bool serialization::write_atomic_types(IO::IPlatformFileRef const& file, rttr::variant const& variant) {
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



bool serialization::write_variant(IO::IPlatformFileRef const& file, rttr::variant const& variant) {
	rttr::type const& value_type = variant.get_type();
	rttr::type const& wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;

	bool is_wrapper = wrapped_type != value_type;

	if (write_atomic_types(file, is_wrapper ? variant.extract_wrapped_value() : variant)) {
	} else if (variant.is_sequential_container()) {
		throw std::exception("Sequential containers are unsupported.");
	} else if (variant.is_associative_container()) {
		throw std::exception("Associative containers are unsupported");
	} else {
		throw std::exception("Non atomic types are unsupported");
	}

	return true;
}

bool serialization::write_instance(IO::IPlatformFileRef const& file, rttr::instance const& instance) {
	rttr::instance obj = instance.get_type().get_raw_type().is_wrapper() ? instance.get_wrapped_instance() : instance;
	rttr::array_range<rttr::property> properties = obj.get_derived_type().get_properties();
	for (auto const& prop : properties) {
		rttr::variant prop_value = prop.get_value(instance);
		if (!prop_value)
			continue;

		const auto name = prop.get_name();

		try {
			if (!write_variant(file, prop_value)) {
				throw std::exception("Failed to serialize property.");
			}
		} catch (std::exception e) {
			std::cerr << e.what() << std::endl;
		}
	}

	return true;
}



rttr::variant serialization::read_atomic_types(IO::IPlatformFileRef const& file, rttr::type const& t) {
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

bool serialization::read_instance(IO::IPlatformFileRef const& file, rttr::instance instance) {
	rttr::instance obj = instance.get_type().get_raw_type().is_wrapper() ? instance.get_wrapped_instance() : instance;
	rttr::array_range<rttr::property> properties = obj.get_derived_type().get_properties();
	for (auto const& prop : properties) {
		rttr::variant prop_value = prop.get_value(instance);
		if (!prop_value)
			continue;

		auto name = prop.get_name();

		const auto val = prop.get_value(obj);

		std::cout << name << std::endl;
		rttr::variant basic_type = read_atomic_types(file, prop.get_type());
		if (basic_type.convert(prop.get_type())) {
			std::string result = basic_type.to_string();
			prop.set_value(obj, basic_type);
		}

	}

	return false;
}
