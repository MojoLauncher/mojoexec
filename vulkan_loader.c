//
// Created by maks on 10.04.2026.
//

#include <android/api-level.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <jni.h>

#include <driver_helper/nsbypass.h>
#include <android/dlext.h>
#include <mojoexec.h>

#include "utils.h"

#define DEFAULT_VULKAN_DRIVER "libvulkan_freedreno.so"

static bool custom_driver_enabled = false;
static char* driver_override = NULL;

#ifdef ENABLE_VULKAN_OVERRIDE
bool load_vulkan_driver() {
    static bool driver_loaded = false;
    if(driver_loaded) return true;

    const char* cache_dir = getenv("TMPDIR");
    if(!linker_ns_load(mojoexec_native_dir)) return NULL;
    void* linkerhook = linker_ns_dlopen("liblinkerhook.so", RTLD_LOCAL | RTLD_NOW);
    if(linkerhook == NULL) return NULL;
    char* driver_library = driver_override ? driver_override : DEFAULT_VULKAN_DRIVER;
    void* driver_handle = linker_ns_dlopen(driver_library, RTLD_LOCAL | RTLD_NOW);
    if(driver_handle == NULL) {
        printf("MojoExec: Failed to load custom Vulkan driver (%s)!\n%s\n",driver_library, dlerror());
        goto fail_l;
    }

    void* dl_android = linker_ns_dlopen("libdl_android.so", RTLD_LOCAL | RTLD_LAZY);
    if(dl_android == NULL) goto fail_t;

    void* android_get_exported_namespace = dlsym(dl_android, "android_get_exported_namespace");
    void (*linkerhook_pass_handles)(void*, void*, void*) = dlsym(linkerhook, "app__pojav_linkerhook_pass_handles");

    if(linkerhook_pass_handles == NULL || android_get_exported_namespace == NULL) goto fail_d;
    linkerhook_pass_handles(driver_handle, android_dlopen_ext, android_get_exported_namespace);

    void* libvulkan = linker_ns_dlopen_unique(cache_dir, "libvulkan.so", "libmjlvlk.so", RTLD_LOCAL | RTLD_NOW);
    printf("MojoExec: Loaded mjlvlk, ptr=%p\n", libvulkan);
    if(libvulkan) {
        driver_loaded = true;
        return true;
    }
    fail_d: dlclose(dl_android);
    fail_t: dlclose(driver_handle);
    fail_l: dlclose(linkerhook);
    return false;
}
#endif

void* mojoexec_acq_vulkan_handle() {
    int flags = RTLD_LOCAL | RTLD_NOW;
#ifdef ENABLE_VULKAN_OVERRIDE
    if(android_get_device_api_level() >= 28) { // the loader does not support below that
        if(custom_driver_enabled && load_vulkan_driver())
            // Reference the vulkan driver separately to avoid weirdness from libraries calling dlclose
            return linker_ns_dlopen("libmjlvlk.so", flags);
    }
#endif
    void* vulkan_ptr = dlopen("libvulkan.so", flags);
    printf("MojoExec: loaded system vulkan, ptr=%p\n", vulkan_ptr);
    return vulkan_ptr;
}

JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_overrideVulkanDriver(JNIEnv *env, jclass clazz, jboolean enable) {
    custom_driver_enabled = enable;
}

// Does nothing if Turnip is unsupported - Mesa will load system driver automatically
JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_preloadVulkan(JNIEnv *env, jclass clazz) {
#ifdef ENABLE_VULKAN_OVERRIDE
    if(!driver_override) return;
    if(!load_vulkan_driver()) {
        printf("MojoExec: Failed to preload Vulkan driver!\n");
    }
#endif
}

JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_overrideVulkanDriverPath(JNIEnv *env, jclass clazz, jstring driver_library){
    save_jvm_string(env, &driver_override, driver_library);
}