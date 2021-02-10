#pragma once

#include <vector>
#include <filesystem>
#include <string>


namespace process {
	namespace fs = std::filesystem;
	void run_process(fs::path executable_path, std::vector<std::string> arguments);
}