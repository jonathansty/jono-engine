#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <algorithm>

#include <fmt/core.h>
#include <fmt/format.h>

#include "CommandLine.h"

#include "parsing/parser.h"

using std::string;
namespace fs = std::filesystem;

#define SHOW_CMD

std::vector<fs::path> gather_dir_files(fs::path dir) {
	std::vector<fs::path> result{};
	for (auto& p : fs::recursive_directory_iterator(dir)) {
		if (p.is_directory()) {
			continue;
		}
		result.push_back(p);
	}
	return result;
}

std::vector<fs::path> gather_files_from_txt(fs::path dir) {
	std::vector<fs::path> result{};

	std::ifstream file{ dir };
	if(file.fail()) {
		throw std::exception("Failed to read input file.");
	}
	std::string line;
	while (std::getline(file, line)) {
		result.push_back(line);
	}
	return result;
}

fs::path get_tracking_file(fs::path original_file, fs::path output_directory, fs::path root) {
	fs::path folder = original_file.parent_path();
	folder = fs::relative(folder, root);

	string ext = original_file.extension().string();
	string new_name = original_file.stem().string() + ".tracking";
	fs::path output_file_path = fs::path(output_directory) / folder / new_name;
	return output_file_path;
};

std::vector<fs::path> filter_files(std::vector<fs::path> const& files, fs::path output, fs::path root, bool force_dirty) {
	std::vector<fs::path> filtered{};
	for(fs::path const& file : files) {

		if (file.extension().compare(".h") != 0 && file.extension().compare(".hpp")) {
			continue;
		}

		if(!force_dirty) {
			auto input_last_write = fs::last_write_time(file);
			auto output_file_path = get_tracking_file(file, output, root);
			if (fs::exists(output_file_path)) {
				auto output_last_write = fs::last_write_time(output_file_path);
				if (output_last_write != input_last_write) {
					filtered.push_back(file);
				}
			} else {
				filtered.push_back(file);
			}
		} else {
			filtered.push_back(file);
		}
	}
	return filtered;
}


int main(int argc, const char** argv) try {

	fs::path current = fs::current_path();
	std::cout << "Starting reflection generation..." << std::endl;

	auto command_line = cli::parse(argv, argc);
#ifdef SHOW_CMD
	fmt::print("Command line: {}\n", cli::to_string(command_line));
#endif
	string input = "";
	cli::get_string(command_line, "-directory", input);

	string file_input = "";
	cli::get_string(command_line, "-file", file_input);

	string output = "";
	cli::get_string(command_line, "-output", output);

	string root = "";
	cli::get_string(command_line, "-root", root);

	bool force_dirty = false;
	cli::get_bool(command_line, "-force-dirty", force_dirty);

	std::cout << " - Current working dir: " << current << std::endl;
	std::cout << " - Dir: " << input << std::endl;
	std::cout << " - Output: " << output << std::endl;
	std::cout << " - Root: " << root << std::endl;

	fs::current_path(root);

	std::vector<fs::path> files;
	if (file_input.empty()) {
		files = gather_dir_files(input);
	} else {
		files = gather_files_from_txt(file_input);
	}

	files = filter_files(files, output, root, force_dirty);

	printf("\tFound %zd files to rebuild...\n", files.size());
	fs::path merged_output = root / fs::path("obj/reflection/gen_reflection.cpp");


	parser::database database{};
	for (auto const& original_file_path : files) {
		try {
			fs::path tracking_file = get_tracking_file(original_file_path, output, root);

			// Create a directory for the parent path.
			auto parent = tracking_file.parent_path();
			if (!fs::exists(parent)) {
				fs::create_directories(parent);
			}

			// Resolve our file to absolute
			tracking_file = fs::absolute(tracking_file);

			if (parser::parse(original_file_path, database) != parser::parser_error::success) {
				throw std::exception("Failed to parse correctly!");
			}

			parser::dump(database, merged_output);

			// Handle the tracking file
			if(!fs::exists(tracking_file)) {
				std::ofstream file{ tracking_file };
				file.close();
			}

			auto last_write_time = fs::last_write_time(original_file_path);
			fs::last_write_time(tracking_file, last_write_time);

		} catch (std::exception const& e) {
			fmt::print("[EXCEPTION]: File processing failed with \"{}\"\n", e.what());
		}
	}
} catch (std::exception e) {
	fmt::print("[EXCEPTION]: Application crashed with the following exception:\n{}\n", e.what());
}