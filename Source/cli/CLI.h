#pragma once

#ifdef CLI_DLL
#ifndef CLI_API
#ifdef CLI_EXPORTS
#define CLI_API __declspec(dllexport)
#else
#define CLI_API __declspec(dllimport)
#endif
#endif
#else
#ifndef CLI_API
#define CLI_API
#endif
#endif
