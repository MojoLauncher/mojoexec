//
// Created by maks on 30.06.2026.
//

#include <mojoexec.h>
#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <driver_helper/nsbypass.h>
#include <dlfcn.h>

#include "utils.h"

mojoexec_renderspec_t mojoexec_renderspec;

const char* mojoexec_native_dir = NULL;
char* native_egl_path = NULL;
bool egl_use_bypass = false;

JNIEXPORT jboolean JNICALL
Java_git_artdeell_mojoexec_MojoExec_prepareEgl(JNIEnv *env, jclass clazz, jstring egl_path,
                                               jboolean use_bypass, jboolean use_gles,
                                               jint gles_version) {
    save_jvm_string(env, &native_egl_path, egl_path);
    if(native_egl_path == NULL) return false;
    if(use_bypass && !linker_ns_load(mojoexec_native_dir)) return false;
    egl_use_bypass = use_bypass;
    void* preload_handle = mojoexec_acq_egl_handle();
    if(!preload_handle) {
        printf("MojoExec: Failed to load EGL: %s\n", dlerror());
        return false;
    }
    printf("MojoExec: Loaded EGL %s (in namespace: %i)\n", native_egl_path, egl_use_bypass);
    mojoexec_renderspec.force_gles_context = use_gles;
    mojoexec_renderspec.override_major_version = gles_version;
    return true;
}

JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_setNativeLibraryDir(JNIEnv *env, jclass clazz, jstring dir) {
    save_jvm_string(env, (char **) &mojoexec_native_dir, dir);
    return;
}

JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_setDisplayParams(JNIEnv *env, jclass clazz, jint width,
                                                     jint height, jfloat hz) {
    // TODO: implement setDisplayParams()
    mojoexec_renderspec.disp_width = width;
    mojoexec_renderspec.disp_height = height;
    mojoexec_renderspec.disp_hz = hz;
}

void* mojoexec_acq_egl_handle() {
    int flags = RTLD_LOCAL | RTLD_NOW;
    if(egl_use_bypass) return linker_ns_dlopen(native_egl_path, flags);
    else return dlopen(native_egl_path, flags);
}

