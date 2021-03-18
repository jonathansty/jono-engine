#include "cli.pch.h"
#include "CommandLine.h"

#include <algorithm>

cli::CommandLine cli::parse(const char **argvs, const int argc) {
	std::string cmdLine = "";
	for (int i = 0; i < argc; ++i)
		cmdLine += (i > 0 ? " " : "") + std::string(argvs[i]);
	return cli::parse(cmdLine);
}

std::string cli::to_string(CommandLine const& cmd) {
	std::string result{};
	for (auto const& c : cmd) {
		result += c + " ";
	}
	return result.substr(0, result.size() - 1);
}

cli::CommandLine cli::parse(std::string cmdLine) {
	std::string delimiter = " ";

	CommandLine result{};
	// Split string
	{
		size_t pos = 0;
		std::string token;
		while ((pos = cmdLine.find(delimiter)) != std::string::npos) {
			result.push_back(cmdLine.substr(0, pos));
			cmdLine.erase(0, pos + delimiter.length());
		}
		if (!cmdLine.empty()) {
			result.push_back(cmdLine);
		}
	}

	return result;
}

bool cli::has_arg(CommandLine const &cmd_args, std::string const &arg) {
	auto it = std::find_if(cmd_args.begin(), cmd_args.end(), [arg](auto const &val) {
		return strstr(val.c_str(), arg.c_str()) != nullptr;
	});

	if (it != cmd_args.end()) {
		return true;
	}

	return false;
}

std::string cli::get_arg(CommandLine const &cmd_args, std::string const &arg) {
	auto it = std::find_if(cmd_args.begin(), cmd_args.end(), [arg](auto const &val) {
		return val.rfind(arg, 0) == 0;
	});
	if (it != cmd_args.end()) {
		return *it;
	} else {
		return {};
	}
}

bool cli::get_bool(CommandLine const &cmd_args, std::string const &arg, bool &val) {
	std::string result;

	if (!get_string(cmd_args, arg, result)) {
		// Failed to retrieve the string value.
		return false;
	}

	// #TODO: parse into bool
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return (char)tolower(c); });

	if(result == "false" || result == "0") {
		val = false;
	} else {
		val = true;
	} 

	return true;
}

bool cli::get_number(CommandLine const& cmd_args, std::string const& arg, int& number) {
	std::string result;
	if (!get_string(cmd_args, arg, result)) {
		// Failed to retrieve the string value.
		return false;
	}

	try
	{
		number = std::stoi(result.c_str());
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool cli::get_string(CommandLine const &cmd_args, std::string const &arg, std::string &val) {
	auto argument = get_arg(cmd_args, arg);

	// Empty argument means we haven't found it in our command line.
	if (argument.empty()) {
		return false;
	}

	std::string lhs;
	helpers::split_string(argument, "=", lhs, val);

	return true;
}

void helpers::split_string(std::string const &in, std::string const &delim, std::string &lhs, std::string &rhs) {
	size_t pos = in.find(delim);
	lhs = in.substr(0, pos);
	rhs = in.substr(pos + delim.size());
}
