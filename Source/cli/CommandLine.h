#pragma once

#include <string>
#include <vector>

namespace Helpers
{

// Helper function to split strings
CLI_API void split_string(std::string_view const& in, std::string_view const& delim, std::string_view& lhs, std::string_view& rhs);

CLI_API void split_string(std::string_view const& in, std::string_view const& delim, std::vector<std::string_view>& result);

CLI_API void split_lines(std::string_view const& in, std::vector<std::string_view>& lines);

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
CLI_API bool has_arg(std::string const& arg);

// Retrieves arguments as string
CLI_API std::string get_arg(CommandLine const& cmd_args, std::string const& arg);

CLI_API bool get_string(CommandLine const& cmd_args, std::string const& arg, std::string& val);
CLI_API bool get_bool(CommandLine const& cmd_args, std::string const& arg, bool& val);
CLI_API bool get_number(CommandLine const& cmd_args, std::string const& arg, int& number);

CLI_API void set(std::string cmdLine);

} // namespace cli