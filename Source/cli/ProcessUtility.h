#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace process
{

namespace fs = std::filesystem;
void run_process(fs::path executable_path, std::vector<std::string> arguments);

} // namespace process