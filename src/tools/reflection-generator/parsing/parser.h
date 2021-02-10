#pragma once

#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>

#include <clang-c/Index.h>
namespace parser {

struct MetaField {
	std::string name;
	uint64_t offset;
};

struct MetaType {
	using field_container = std::unordered_map<std::string, MetaField>;

	// MetaType it's type...
	enum class Type {
		eClass,
		eStruct
	};

	std::string name;
	Type type;
	field_container fields;
};

struct database {
	using KeyType = std::string;
	using type_container = std::unordered_map<KeyType, MetaType>;

	type_container m_types;
};

namespace fs = std::filesystem;

enum class parser_error : uint32_t {
	success = 0,
	failed_to_open_source_file,
	source_file_missing,
	failed_clang_parse,
};

// Loads the file and parses
parser_error parse(fs::path const& source, database& output);

void dump(database const& db, fs::path const& out);

std::string to_string(CXString str);
} // namespace parser