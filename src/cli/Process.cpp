#include "Process.h"

#include <fmt/core.h>
#include <fmt/printf.h>

#ifdef WIN64
#include <Windows.h>
#endif


void process::run_process(fs::path executable_path, std::vector<std::string> arguments) {
#if defined(WIN64)
	// Run external process
	{
		std::string cmd_line = executable_path.string() + " ";
		for (std::string const& arg : arguments) {
			cmd_line += arg + " ";
		}
		
		STARTUPINFOA si;
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi;
		if (!CreateProcessA(NULL, (char*)cmd_line.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			fmt::printf("Failed to create process (%d).\n", GetLastError());
			throw std::exception(fmt::format("Failed to create process with error: %d!", GetLastError()).c_str());
		}

		// Block for the process to finish
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
#else
	throw std::exception("Not implemented for this platform.");
#endif
}
