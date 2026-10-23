//
// Created by whbex on 20.08.2026.
//

#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>


static FILE* (*real_fopen)(const char*, const char*);
static const char* config_path;
static FILE* handle;

__attribute((visibility("default"), used)) FILE* fopen(const char* filename, const char* mode) {
    if(!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    if(config_path && strstr(filename, "qgl_config.txt")) {
        printf("MojoExec: redirecting QGL config load to %s path\n", config_path);
        return real_fopen(config_path, mode);
    }
    return real_fopen(filename, mode);
}

__attribute((visibility("default"), used)) void fhook_pass_path(const char* path) {
    if(!path) return;
    if(config_path) free((void*)path);
    config_path = path;
    printf("MojoExec: accepted QGL config path : %s\n", path);
}