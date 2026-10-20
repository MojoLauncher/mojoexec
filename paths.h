//
// Created by whbex on 18.10.2026.
//

#ifndef MOJOLAUNCHER_PATHS_H
#define MOJOLAUNCHER_PATHS_H

#include "elf_defs.h"

/* Library search path */
#ifdef PLATFORM_64
#define SEARCH_PATH "/system/lib64" // used inside dlopen unique
#define MAIN_SEARCH_PATH "/system/lib64:/system_ext/lib64" // used in mojoexec-driver namespace
#define BLOB_SEARCH_PATH "/system/lib64:/vendor/lib64:/system_ext/lib64" // used in blob-driver namespace
#else
#define SEARCH_PATH "/system/lib" // used inside dlopen unique
#define MAIN_SEARCH_PATH "/system/lib:/system_ext/lib" // used in mojoexec-driver namespace
#define BLOB_SEARCH_PATH "/system/lib:/vendor/lib:/system_ext/lib" // used in blob-driver namespace
#endif

#endif //MOJOLAUNCHER_PATHS_H
