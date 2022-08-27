#pragma once

#include <string>
#include <vector>

namespace helpers
{

// Helper function to split strings
void split_string(std::string const& in, std::string const& delim, std::string& lhs, std::string& rhs);

} // namespace helpers

namespace cli
{

// Typedef'd for future refactoring.
using CommandLine = std::vector<std::string>;

// Parsing interface
CommandLine parse(std::string cmdLine);
CommandLine parse(const char** cmdLine, const int argc);

// Convert parsed command line back into string
std::string to_string(CommandLine const& cmd);

// Query API
bool has_arg(CommandLine const& cmd_args, std::string const& arg);

// Retrieves arguments as string
std::string get_arg(CommandLine const& cmd_args, std::string const& arg);

bool get_string(CommandLine const& cmd_args, std::string const& arg, std::string& val);
bool get_bool(CommandLine const& cmd_args, std::string const& arg, bool& val);
bool get_number(CommandLine const& cmd_args, std::string const& arg, int& number);

} // namespace cli