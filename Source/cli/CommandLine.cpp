#include "cli.pch.h"
#include "CommandLine.h"

static cli::CommandLine s_CommandLine;

cli::CommandLine cli::parse(char** argvs, int argc)
{
	std::string cmdLine = "";
	for (int i = 0; i < argc; ++i)
		cmdLine += (i > 0 ? " " : "") + std::string(argvs[i]);
	return cli::parse(cmdLine);
}

std::string cli::to_string(CommandLine const& cmd)
{
	std::string result{};
	for (auto const& c : cmd)
	{
		result += c + " ";
	}
	return result.substr(0, result.size() - 1);
}

cli::CommandLine cli::parse(std::string cmdLine)
{
	std::string delimiter = " ";

	CommandLine result{};
	// Split string
	{
		size_t pos = 0;
		std::string token;
		while ((pos = cmdLine.find(delimiter)) != std::string::npos)
		{
			result.push_back(cmdLine.substr(0, pos));
			cmdLine.erase(0, pos + delimiter.length());
		}
		if (!cmdLine.empty())
		{
			result.push_back(cmdLine);
		}
	}

	return result;
}

bool cli::has_arg(CommandLine const& cmd_args, std::string const& arg)
{
	auto it = std::find_if(cmd_args.begin(), cmd_args.end(), [arg](auto const& val)
			{ return strstr(val.c_str(), arg.c_str()) != nullptr; });

	if (it != cmd_args.end())
	{
		return true;
	}

	return false;
}

CLI_API bool cli::has_arg(std::string const& arg)
{
    return has_arg(s_CommandLine, arg);
}

std::string cli::get_arg(CommandLine const& cmd_args, std::string const& arg)
{
	auto it = std::find_if(cmd_args.begin(), cmd_args.end(), [arg](auto const& val)
			{ return val.rfind(arg, 0) == 0; });
	if (it != cmd_args.end())
	{
		return *it;
	}
	else
	{
		return {};
	}
}

bool cli::get_bool(CommandLine const& cmd_args, std::string const& arg, bool& val)
{
	std::string result;

	if (!get_string(cmd_args, arg, result))
	{
		// Failed to retrieve the string value.
		return false;
	}

	// #TODO: parse into bool
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c)
			{ return (char)tolower(c); });

	if (result == "false" || result == "0")
	{
		val = false;
	}
	else
	{
		val = true;
	}

	return true;
}

bool cli::get_number(CommandLine const& cmd_args, std::string const& arg, int& number)
{
	std::string result;
	if (!get_string(cmd_args, arg, result))
	{
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

CLI_API void cli::set(std::string cmdLine)
{
    s_CommandLine = cli::parse(cmdLine);
}

bool cli::get_string(CommandLine const& cmd_args, std::string const& arg, std::string& val)
{
	auto argument = get_arg(cmd_args, arg);

	// Empty argument means we haven't found it in our command line.
	if (argument.empty())
	{
		return false;
	}

	std::string_view lhs;
	std::string_view rhs;
	Helpers::split_string(argument, "=", lhs, rhs);

	val = rhs;

	return true;
}

void Helpers::split_string(std::string_view const& in, std::string_view const& delim, std::string_view& lhs, std::string_view& rhs)
{
	size_t pos = in.find(delim);
	lhs = in.substr(0, pos);
	rhs = in.substr(pos + delim.size());
}

void Helpers::split_string(std::string_view const& in, std::string_view const& delim, std::vector<std::string_view>& result)
{
	result.clear();
	size_t prev = 0;
	size_t pos = in.find(delim);
	while(pos != std::string::npos)
	{
		result.emplace_back(in.begin() + prev, in.begin() + pos);

		prev = pos + 1;
		pos = in.find(delim, prev + 1);
	}

	if (prev < in.size())
	{
		result.emplace_back(in.begin() + prev, in.end());
	}

}

void Helpers::split_lines(std::string_view const& in, std::vector<std::string_view>& lines)
{
	size_t n = 0;

	size_t lineStart = 0;
	size_t lineEnd = 0;
	while(n < in.size())
	{
		if (in[n] == '\n')
		{
			lineEnd = n - 1;
			lines.emplace_back(in.data() + lineStart, 1 + lineEnd - lineStart);

			lineStart = n + 1;
		}
		++n;
	}

	if(lineStart < in.size())
	{
		lines.emplace_back(in.data() + lineStart, in.size() - lineStart);
	}
}
