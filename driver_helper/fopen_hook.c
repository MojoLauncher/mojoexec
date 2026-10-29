//
// Created by whbex on 20.08.2026.
//

#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>


static const char* config_path;
static FILE* (*real_fopen)(const char*, const char*);

__attribute((visibility("default"), used)) FILE* fopen(const char* filename, const char* mode) {
    if(!real_fopen) return NULL;
    if(config_path && strstr(filename, "qgl_config.txt")) {
        printf("MojoExec: redirecting QGL config load to %s path\n", config_path);
        return real_fopen(config_path, mode);
    }
    return real_fopen(filename, mode);
}

__attribute((visibility("default"), used)) void fhook_pass_handles(const char* path, void* fopen_func) {
    if(!path) return;
    if(config_path) free((void*)path);
    config_path = path;
    real_fopen = fopen_func;
    printf("MojoExec: accepted QGL config path : %s\n", path);
}