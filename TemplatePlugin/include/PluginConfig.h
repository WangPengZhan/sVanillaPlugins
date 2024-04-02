#pragma once

// The way how the function is called
#if !defined(PLUGIN_CALL)
#if defined(_WIN32)
#define PLUGIN_CALL __stdcall
#else
#define PLUGIN_CALL
#endif /* _WIN32 */
#endif /* PLUGIN_CALL */

// The function exported symbols
#if defined _WIN32 || defined __CYGWIN__
#define PLUGIN_DLL_IMPORT __declspec(dllimport)
#define PLUGIN_DLL_EXPORT __declspec(dllexport)
#define PLUGIN_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define PLUGIN_DLL_IMPORT __attribute__((visibility("default")))
#define PLUGIN_DLL_EXPORT __attribute__((visibility("default")))
#define PLUGIN_DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define PLUGIN_DLL_IMPORT
#define PLUGIN_DLL_EXPORT
#define PLUGIN_DLL_LOCAL
#endif
#endif

#ifdef PLUGIN_LOCAL
#define PLUGIN_PORT
#define PLUGIN_API PLUGIN_DLL_LOCAL
#else
#ifdef PLUGIN_DLL_EXPORTS // defined if we are building the DLL (instead of using it)
#define PLUGIN_PORT PLUGIN_DLL_EXPORT
#define PLUGIN_API PLUGIN_DLL_EXPORT PLUGIN_CALL
#else
#define PLUGIN_PORT PLUGIN_DLL_IMPORT
#define PLUGIN_API PLUGIN_DLL_IMPORT PLUGIN_CALL
#endif // PLUGIN_DLL_EXPORTS
#endif