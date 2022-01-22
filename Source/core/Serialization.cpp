#include "core.pch.h"
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
	write(file, type_hash.get_hash());

	u32 n_properties = u32(properties.size());
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

bool serialization::read_atomic_types(IO::IFileRef const& file, rttr::variant& variant) {
	using namespace rttr;
	rttr::type t = variant.get_type();

	if (t.is_arithmetic()) {
		if (t == type::get<bool>()) {
			variant = read<bool>(file);
			return true;
		} else if (t == type::get<char>()) {
			variant = read<char>(file);
			return true;
		} else if (t == type::get<int8_t>()) {
			variant = read<int8_t>(file);
			return true;
		} else if (t == type::get<int16_t>()) {
			variant = read<int16_t>(file);
			return true;
		} else if (t == type::get<int32_t>()) {
			variant = read<int32_t>(file);
			return true;
		} else if (t == type::get<int64_t>()) {
			variant = read<int64_t>(file);
			return true;
		} else if (t == type::get<uint8_t>()) {
			variant = read<uint8_t>(file);
			return true;
		} else if (t == type::get<uint16_t>()) {
			variant = read<uint16_t>(file);
			return true;
		} else if (t == type::get<uint32_t>()) {
			variant = read<uint32_t>(file);
			return true;
		} else if (t == type::get<uint64_t>()) {
			variant = read<uint64_t>(file);
			return true;
		} else if (t == type::get<float>()) {
			variant = read<float>(file);
			return true;
		} else if (t == type::get<double>()) {
			variant = read<double>(file);
			return true;
		}
	} else if (t.is_enumeration()) {
		variant = read<uint64_t>(file);
		return true;
	} else if (t == type::get<std::string>()) {
		variant = read<std::string>(file);
		return true;
	} else if (t == type::get<WrapperFloat4>()) {
		float4 value{};
		value.x = read<float>(file);
		value.y = read<float>(file);
		value.z = read<float>(file);
		value.w = read<float>(file);
		WrapperFloat4 result;
		result.value = value;
		variant = result;
		return true;
	} else if (t == type::get<WrapperFloat3>()) {
		float3 value = {};
		value.x = read<float>(file);
		value.y = read<float>(file);
		value.z = read<float>(file);
		WrapperFloat3 result;
		result.value = value;
		variant = result;
		return true;
	}

	return false;
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
		rttr::variant value = prop.get_value(obj);
		if (read_atomic_types(file, value)) {} 
		else if (read_container(file, value)) {}
		else if (read_instance(file, value)) {}

		if (value.convert(t_prop)) {
			prop.set_value(obj, value);
		}


	}

	return true;
}

bool serialization::read_container(IO::IFileRef const& file, rttr::instance variant) {
	if (variant.get_type().is_associative_container()) {
		return read_associative_container(file, variant);
	} else if (variant.get_type().is_sequential_container()) {
		return read_sequential_container(file, variant);
	}
	return false;
}

bool serialization::write_container(IO::IFileRef const& file, rttr::variant const& variant) {

	rttr::type t = variant.get_type();
	if (t.is_associative_container()) {
		return write_associative_container(file, variant);
	} 
	else if (t.is_sequential_container()) {
		return write_sequential_container(file, variant);
	}

	return false;
}

bool serialization::write_sequential_container(IO::IFileRef const& file, rttr::variant const& variant) {

	rttr::type obj = variant.get_type().get_wrapped_type();
	rttr::type t_obj = obj.get_raw_type();

	// Write the type hash before doing the container
	Identifier64 type_hash = Identifier64(variant.get_type().get_name().begin());
	write(file, type_hash.get_hash());

	auto view = variant.create_sequential_view();
	serialization::write<std::size_t>(file, view.get_size());

	// Optimize to write the type only once for the entire container
	for (size_t i = 0; i < view.get_size(); ++i) {
		rttr::variant value = view.get_value(i).extract_wrapped_value();
		rttr::variant t = view.get_type();
		if (write_atomic_types(file, value)) {
		} else if (write_container(file, value)) {
		} else if (write_instance(file, value)) {
		} else {
			return false;
		}
	}

	return true;
}

bool serialization::write_associative_container(IO::IFileRef const& file, rttr::variant const& variant) {
	rttr::type obj = variant.get_type().get_wrapped_type();
	rttr::type t_obj = obj.get_raw_type();

	Identifier64 type_hash = Identifier64(variant.get_type().get_name().begin());
	write(file, type_hash.get_hash());

	auto view = variant.create_associative_view();
	serialization::write<std::size_t>(file, view.get_size());

	// Optimize to write the type only once for the entire container
	for (auto const& item : view) {
		// Write key
		if (write_atomic_types(file, item.first)) {
		} else if (write_container(file, item.first)) {
		} else if (write_instance(file, item.first)) {
		} else {
			return false;
		}


		// Write value
		if (write_atomic_types(file, item.second)) {
		} else if (write_container(file, item.second)) {
		} else if (write_instance(file, item.second)) {
		} else {
			return false;
		}
	}

	return true;
}

bool serialization::read_sequential_container(IO::IFileRef const& file, rttr::instance obj) {

	Identifier64 type_hash = Identifier64{ read<u64>(file) };

	rttr::type t = helpers::get_type_by_id(type_hash.get_hash());

	rttr::variant container = t.create();
	assert(container.is_sequential_container());
	auto view = container.create_sequential_view();

	size_t size = read<size_t>(file);
	view.set_size(size);

	for (size_t i = 0; i < size; ++i) {

		rttr::variant value = view.get_value_type().create();
		if (read_atomic_types(file, value)) {
		} else if (read_container(file, value)) {
		} else if (read_instance(file, value)) {
		} else {
			return false;
		}

		view.set_value(i, value);

	}

	return false;
}

bool serialization::read_associative_container(IO::IFileRef const& file, rttr::instance obj) {

	Identifier64 type_hash = Identifier64{ read<u64>(file) };
	rttr::type t = helpers::get_type_by_id(type_hash.get_hash());

	rttr::variant container = t.create();
	auto view = container.create_associative_view();

	size_t size = read<size_t>(file);
	for (size_t i = 0; i < size; ++i) {

		rttr::variant key = view.get_key_type().create();
		if (read_atomic_types(file, key)) {
		} else if (read_container(file, key)) {
		} else if (read_instance(file, key)) {
		} else {
		}


		// Value
		rttr::variant value = view.get_value_type().create();
		if (read_atomic_types(file, value)) {
		} else if (read_container(file, value)) {
		} else if (read_instance(file, value)) {
		} else {
		}

		view.insert(key, value);
	}

	return true;
}

