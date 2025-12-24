#pragma once

#ifdef _WIN32
extern constexpr const char *OODLE_DLL_FILENAME = "oo2core_9_win64.dll";
extern const unsigned char oo2core_9_win64_dll[];
extern const unsigned oo2core_9_win64_dll_size;
#else
extern constexpr const char *OODLE_DLL_FILENAME = "liboo2corelinux64.so.9";
extern const unsigned char liboo2corelinux64_so_9[];
extern const unsigned liboo2corelinux64_so_9_size;
#endif