#include "parser.h"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <sstream>

#include "parsing/Cursor.h"


namespace parser {

std::string to_string(CXString str) {
	auto result = std::string(clang_getCString(str));
	return result;
}

CXChildVisitResult visit_children(CXCursor c, CXCursor parent, std::vector<CXCursor>& out_children) {
	out_children.push_back(c);

	if(c.kind == CXCursor_InclusionDirective) {
		return CXChildVisitResult::CXChildVisit_Continue;	
	}
	else if (c.kind == CXCursor_LastPreprocessing) {
		return CXChildVisitResult::CXChildVisit_Break;
	 } 
	else {
		return CXChildVisitResult::CXChildVisit_Recurse;
	}
}

void print_node(Cursor cursor) {
	auto loc = clang_getCursorLocation(cursor);
	auto cursorSpelling = cursor.get_spelling();
	auto cursorKindSpelling = cursor.get_kind_spelling();

	CXFile file;
	uint32_t line, column, offset;
	clang_getSpellingLocation(loc, &file, &line, &column, &offset);
	CXString file_location = clang_getFileName(file);

	if (file != nullptr) {
		printf("%s:%d:%d - Cursor \'%s\' of kind \'%s\'\n", to_string(file_location).c_str(), line, column, cursorSpelling.c_str(), cursorKindSpelling.c_str());
	}

	clang_disposeString(file_location);
}


void print_location(CXSourceLocation loc) {
	CXFile file;
	uint32_t line, column, offset;
	clang_getSpellingLocation(loc, &file, &line, &column, &offset);
	CXString file_location = clang_getFileName(file);

	if (file != nullptr) {
		fmt::print("{}:{}:{}'\n", to_string(file_location).c_str(), line, column);
	}

	clang_disposeString(file_location);
};

parser_error parse(fs::path const& source, database& output) {

	std::unordered_map<std::string, MetaType> classes;

	if(!fs::exists(source)){
		return parser_error::source_file_missing;
	}

	CXIndex index = clang_createIndex(0, 1);

	const char* command_line_args[] = { "-x", "c++", "-std=c++17", "-D __REFLECTION_PARSER__", 0 };
	CXTranslationUnit unit = clang_createTranslationUnitFromSourceFile(index, source.string().c_str(), std::size(command_line_args) - 1, command_line_args, 0, 0);
	if (unit == nullptr) {
		return parser_error::failed_clang_parse;
	}

	Cursor cursor = clang_getTranslationUnitCursor(unit);

	// Gather a list of CXCursors to allow better traversal
	std::vector<Cursor> children;
	cursor.visit_children([](CXCursor c, CXCursor parent, CXClientData client_data) {
		std::vector<CXCursor>* children = (std::vector<CXCursor>*)(client_data);
		return visit_children(c, parent, *children);
	}, &children);


	std::sort(children.begin(), children.end(), [](CXCursor const& lhs, CXCursor const& rhs) {
		return clang_getCursorLocation(lhs).int_data < clang_getCursorLocation(rhs).int_data;
	});


	fmt::memory_buffer out{};
	for (auto it = children.begin(); it != children.end(); ++it) {
		auto current_cursor = *it;

		CXCursorKind kind = current_cursor.get_kind();
		if (kind == CXCursor_MacroExpansion) {

			std::string spelling = current_cursor.get_spelling();
			if (spelling.compare("rg_reflect") == 0) {
				auto next = Cursor(*(++it));

				// Gather all members
				std::vector<Cursor> children;
				next.visit_children([](CXCursor c, CXCursor parent, CXClientData data) {
					std::vector<Cursor>& children = *(std::vector<Cursor>*)(data);
					Cursor cursor = c;
					if(cursor.get_kind() == CXCursor_FieldDecl) {
						children.push_back(cursor);
					}
					return CXChildVisitResult::CXChildVisit_Continue;
				},
				&children);

				MetaType data{};
				data.name = next.get_spelling();
				data.type = next.get_kind() == CXCursor_ClassDecl ? MetaType::Type::eClass : MetaType::Type::eStruct;
				for(auto& child : children) {
					MetaField field{};
					field.name = child.get_spelling();
					field.offset = clang_Cursor_getOffsetOfField(child) / 8;

					data.fields[field.name] = std::move(field);
				}
				output.m_types[data.name] = std::move(data);
			}
		}
	}
	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);

	return parser_error::success;
}

void dump(database const& db, fs::path const& out) {
	std::ofstream file{ out };
	if (file.fail()) {
		throw std::exception(fmt::format("Failed to open the file at \"{}\"", out.string()).data());
	}

	file << "// This file is generated by reflection-generator. DO NOT CHANGE! \n";
	file << "#include \"reflection.h\"" << "\n";
	for (auto const& d : db.m_types) {
		std::string const& className = d.second.name;
		MetaType::field_container const& fields = d.second.fields;

		fmt::memory_buffer class_body;

		// Implement some getters
		fmt::format_to(class_body, "\tstd::string is_class() const {{ return {0}; }}\n", d.second.type == MetaType::Type::eClass);
		fmt::format_to(class_body, "\tstd::string is_struct() const {{ return {0}; }}\n", d.second.type == MetaType::Type::eStruct);
		fmt::format_to(class_body, "\tstd::string get_name() const {{ return \"{0}\"; }}\n", className);

		fmt::format_to(class_body, "\tstd::vector<const char*> const members() const {{\n\t\treturn {{");
		for (auto child : d.second.fields) {
			fmt::format_to(class_body, "\n\t\t\t\"{}\",", child.second.name);
		}
		fmt::format_to(class_body, "\n\t\t}}; \n\t}}");

		// Print final class construction
		file << fmt::format("template<> \nclass meta_type<{0}> : public base_meta_type {{\n {1} \n}};\n", className, fmt::to_string(class_body));
	}
}

} // namespace parser
