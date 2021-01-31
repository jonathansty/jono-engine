#pragma once

#include <vector>
#include <string>

namespace helpers {

void split_string(std::string const &in, std::string const &delim, std::string &lhs, std::string &rhs);

} // namespace helpers

namespace cli {

using CommandLine = std::vector<std::string>;
CommandLine parse(std::string cmdLine);
CommandLine parse(const char** cmdLine, const int argc );

bool has_arg(CommandLine const &cmd_args, std::string const &arg);

std::string get_arg(CommandLine const& cmd_args, std::string const& arg);

bool get_string(CommandLine const &cmd_args, std::string const &arg, std::string &val);
bool get_bool(CommandLine const &cmd_args, std::string const &arg, bool &val);
bool get_number(CommandLine const &cmd_args, std::string const &arg, int &number);

} // namespace cli