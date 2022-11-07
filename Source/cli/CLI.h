#pragma once

#ifdef CLI_DLL
#ifdef CLI_EXPORTS
#define CLI_API __declspec(dllexport)
#else
#define CLI_API __declspec(dllimport)
#endif
#else
#define CLI_API
#endif
