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
		auto result = var.to_string(&ok);
		if (ok) {
			write(file, var.to_string());
		} else {
			ok = false;
			auto value = var.to_uint64(&ok);
			if (ok)
				write(file, value);
		}
		return true;
	} else if (t == type::get<std::string>()) {
		write(file, var.to_string());
		return true;
	} else if (t == type::get_by_name("float4")) {
		float4 value = var.get_value<WrapperFloat4>().value;
		write(file, value.x);
		write(file, value.y);
		write(file, value.z);
		write(file, value.w);
		return true;
	} else if (t == type::get_by_name("float3")) {
		float3 value = var.get_value<WrapperFloat3>().value;
		write(file, value.x);
		write(file, value.y);
		write(file, value.z);
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
		write(file, name.to_string());

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

bool serialization::load_instance(IO::IPlatformFileRef const& file, rttr::instance const& instance) {
	return false;
}
