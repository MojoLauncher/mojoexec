//
// Created by whbex on 18.10.2026.
//

#ifndef MOJOLAUNCHER_PATHS_H
#define MOJOLAUNCHER_PATHS_H

#include "elf_defs.h"

/* Library search path */
#ifdef PLATFORM_64
#define SEARCH_PATH "/system/lib64"
#define BLOB_SEARCH_PATH "/system/lib64:/vendor/lib64:/system_ext/lib64"
#else
#define SEARCH_PATH "/system/lib"
#define BLOB_SEARCH_PATH "/system/lib:/vendor/lib:/system_ext/lib"
#endif

#endif //MOJOLAUNCHER_PATHS_H
