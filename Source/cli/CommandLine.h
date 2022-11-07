#pragma once

#include <string>
#include <vector>

namespace Helpers
{

// Helper function to split strings
void split_string(std::string const& in, std::string const& delim, std::string& lhs, std::string& rhs);

} // namespace helpers

namespace cli
{

// Typedef'd for future refactoring.
using CommandLine = std::vector<std::string>;

// Parsing interface
CLI_API CommandLine parse(std::string cmdLine);
CLI_API CommandLine parse(char** cmdLine, int argc);

// Convert parsed command line back into string
CLI_API std::string to_string(CommandLine const& cmd);

// Query API
CLI_API bool has_arg(CommandLine const& cmd_args, std::string const& arg);

// Retrieves arguments as string
CLI_API std::string get_arg(CommandLine const& cmd_args, std::string const& arg);

CLI_API bool get_string(CommandLine const& cmd_args, std::string const& arg, std::string& val);
CLI_API bool get_bool(CommandLine const& cmd_args, std::string const& arg, bool& val);
CLI_API bool get_number(CommandLine const& cmd_args, std::string const& arg, int& number);

} // namespace cli