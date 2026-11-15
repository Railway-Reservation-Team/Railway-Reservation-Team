#ifndef ATOMIC_FILE_REPLACE_H
#define ATOMIC_FILE_REPLACE_H

#include <cstdio>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

inline bool replaceFileAtomically(
    const std::string& temporaryFile,
    const std::string& destinationFile) {

#ifdef _WIN32
    return MoveFileExA(
        temporaryFile.c_str(),
        destinationFile.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
    ) != 0;
#else
    return std::rename(
        temporaryFile.c_str(),
        destinationFile.c_str()
    ) == 0;
#endif
}

#endif
