#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "CommandLine.h"

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


void main(int argc, const char** argv) {

	fs::path current = fs::current_path();
	std::cout << "Starting reflection generation..." << std::endl;

	auto command_line = cli::parse(argv, argc);
#ifdef SHOW_CMD
	for(int i = 0; i < argc; ++i) {
		std::cout << command_line[i] << " ";
	}
	std::cout << std::endl;
#endif
	string input = argv[1];
	cli::get_string(command_line, "-directory", input);

	string file_input = "";
	cli::get_string(command_line, "-file", file_input);
	
	string output = "";
	cli::get_string(command_line, "-output",output);

	string root = "";
	cli::get_string(command_line, "-root",root);


	std::cout << " - Current working dir: " << current << std::endl;
	std::cout << " - Dir: " << input << std::endl;
	std::cout << " - Output: " << output << std::endl;
	std::cout << " - Root: " << root << std::endl;

	fs::current_path(root);

	try
	{

		std::vector<fs::path> files;
		if(file_input.empty()) {
			files = gather_dir_files(input);
		} else {
			files = gather_files_from_txt(file_input);
		}

		for(auto& p : files) {
			try {
				fs::path dir = p;
				if (dir.extension() != ".h") {
					continue;
				}
				fs::path folder = dir.parent_path();
				folder = fs::relative(folder, root);

				string ext = dir.extension().string();
				string new_name = dir.stem().string() + ".generated" + ext;
				dir = fs::path(output) / folder / new_name;

				auto parent = dir.parent_path();
				if (!fs::exists(parent)) {
					fs::create_directories(parent);
				}

				dir = fs::absolute(dir);
				std::cout << dir << std::endl;

				std::ofstream file{};
				file.open(dir);
				file << "#pragma once\n";
				file.close();
			} catch (std::exception const& e) {
				std::cout << "File processing failed with \"" << e.what() << "\""
						  << "\n";
			}
		}
	}
	catch (std::exception e)
	{
		std::cout << "EXCEPTION: " << e.what() << std::endl;
		
	}

}