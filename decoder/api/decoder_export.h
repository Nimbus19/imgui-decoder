#pragma once

#ifdef _WIN32
#  define LIB_EXPORT __declspec(dllexport)
#  define WINAPI __stdcall
#else
#  define LIB_EXPORT __attribute__((visibility("default")))
#  define WINAPI
#endif

