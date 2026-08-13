//
// Created by maks on 30.06.2026.
//

#include <stdbool.h>

#ifndef POJAVLAUNCHER_MOJOEXEC_H
#define POJAVLAUNCHER_MOJOEXEC_H


typedef struct {
    int force_gles_context;
    int override_major_version;
    int disp_width;
    int disp_height;
    float disp_hz;
} mojoexec_renderspec_t;

extern mojoexec_renderspec_t mojoexec_renderspec;
extern const char* mojoexec_native_dir;
void* mojoexec_acq_vulkan_handle();
void* mojoexec_acq_egl_handle();
int mojoexec_override_pretransform(void* native_window);

#endif //POJAVLAUNCHER_MOJOEXEC_H
