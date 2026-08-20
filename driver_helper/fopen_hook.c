//
// Created by whbex on 20.08.2026.
//

#include <dlfcn.h>
#include <string.h>
#include <stdio.h>


static FILE* (*real_fopen)(const char*, const char*);
static FILE* config_handle;

__attribute((visibility("default"), used)) FILE* fopen(const char* filename, const char* mode) {
    if(config_handle && strstr(filename, "qgl_config.txt")) {
        printf("MojoExec: redirecting QGL config load to %p handle\n", config_handle);
        return config_handle;
    }
    // Fallback to generic fopen
    if(!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    if(!real_fopen) {
        printf("MojoExec: failed to fetch fopen symbol!\n");
        return NULL;
    }
    return real_fopen(filename, mode);
}

__attribute((visibility("default"), used)) void fhook_pass_handles(FILE *config) {
    if(!config) return;
    if(config_handle) fclose(config_handle);
    config_handle = config;
    printf("MojoExec: accepted QGL config handle : %p\n", config);
}