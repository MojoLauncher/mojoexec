//
// Created by maks on 10.04.2026.
//

#include <android/api-level.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <jni.h>

#include <android/dlext.h>
#include <mojoexec.h>
#include <string.h>
#include "paths.h"

#include "utils.h"


static char* driver_lib = NULL;
static char* driver_path = NULL;
static char* config_override = NULL;

#ifdef ENABLE_QGL_REDIRECT

// This allows us to redirect Qualcomm proprietary config file
void setup_fopen_hook(struct android_namespace_t *driver_ns, const char* path) {
    if(!path) {
        printf("MojoExec: provide a correct path to override qgl config\n");
        return;
    }
    void* fopen_hook = linker_ns_dlopen(driver_ns, "libfopenhook.so", RTLD_GLOBAL | RTLD_NOW);
    if(!fopen_hook) goto fail;
    void (*fhook_pass_handles)(const char*, void*) = dlsym(fopen_hook, "fhook_pass_handles");
    if(!fhook_pass_handles) goto fail;
    fhook_pass_handles(path, &fopen);
    return;

    fail:
    printf("MojoExec: failed to init fopen hook!\n");
}

#endif

#ifdef ENABLE_VULKAN_OVERRIDE


static bool load_vulkan_driver() {
    static bool driver_loaded = false;
    if(driver_lib == NULL) {
        printf("MojoExec: attempted to preload custom driver, yet no driver is specified\n");
        return false;
    }
    if(driver_loaded) return true;

    const char* cache_dir = getenv("TMPDIR");

    // The hooked vulkan loader will always be put into the primary mojoexec namespace
    struct android_namespace_t *main_ns = get_primary_namespace();
    if(!main_ns) {
        printf("MojoExec: failed to load main namespace!\n");
        return NULL;
    }

    // By default, the driver will use the primary namespace which includes launcher libs. However,
    // to avoid 3rd party driver packages from loading launcher libs (and possibly failing),
    // we allow the user to specify custom search paths. This will create another namespace,
    // completely isolated from the launcher.
    struct android_namespace_t *driver_ns = main_ns;
    if(driver_path != NULL) {
        driver_ns = linker_ns_load(driver_path, "mojoexec-driver");
#ifdef ENABLE_QGL_REDIRECT
        if(config_override) setup_fopen_hook(driver_ns, config_override);
#endif
    }
    void* linkerhook = linker_ns_dlopen(main_ns, "liblinkerhook.so", RTLD_LOCAL | RTLD_NOW);
    if(linkerhook == NULL) {
        printf("MojoExec: failed to load linkerhook! %s\n", dlerror());
        return NULL;
    }
    void* driver_handle = linker_ns_dlopen(driver_ns, driver_lib, RTLD_LOCAL | RTLD_NOW);
    if(driver_handle == NULL) {
        printf("MojoExec: Failed to load custom Vulkan driver (%s)!\n%s\n", driver_lib, dlerror());
        goto fail_l;
    } else printf("MojoExec: Loaded Vulkan library : %p\n", driver_handle);

    void* dl_android = linker_ns_dlopen(main_ns, "libdl_android.so", RTLD_LOCAL | RTLD_LAZY);
    if(dl_android == NULL) {
        printf("MojoExec: failed to dlopen dl_android! %s\n", dlerror());
        goto fail_t;
    }

    void* android_get_exported_namespace = dlsym(dl_android, "android_get_exported_namespace");
    void (*linkerhook_pass_handles)(void*, void*, void*) = dlsym(linkerhook, "app__pojav_linkerhook_pass_handles");

    if(linkerhook_pass_handles == NULL || android_get_exported_namespace == NULL) goto fail_d;
    linkerhook_pass_handles(driver_handle, android_dlopen_ext, android_get_exported_namespace);

    void* libvulkan = linker_ns_dlopen_unique(main_ns, cache_dir, "libvulkan.so", "libmjlvlk.so", RTLD_LOCAL | RTLD_NOW);
    if(libvulkan) {
        printf("MojoExec: Loaded mjlvlk, ptr=%p\n", libvulkan);
        driver_loaded = true;
        return true;
    } else {
        printf("MojoExec: failed to reference mjlvlk, %s\n", dlerror());
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
        if(driver_lib != NULL && load_vulkan_driver())
            // Reference the vulkan driver separately to avoid weirdness from libraries calling dlclose
            return linker_ns_dlopen(get_primary_namespace(), "libmjlvlk.so", flags);
    }
#endif
    void* vulkan_ptr = dlopen("libvulkan.so", flags);
    printf("MojoExec: loaded system vulkan, ptr=%p\n", vulkan_ptr);
    return vulkan_ptr;
}

// Does nothing if Turnip is unsupported - Mesa will load system driver automatically
JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_preloadVulkan(JNIEnv *env, jclass clazz) {
#ifdef ENABLE_VULKAN_OVERRIDE
    if(driver_lib == NULL) return;
    if(!load_vulkan_driver()) {
        printf("MojoExec: Failed to preload Vulkan driver!\n");
    }
#endif
}

JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_configureCustomVulkanDriver(JNIEnv *env, jclass clazz,
                                                                jstring library_name,
                                                                jstring library_search_path) {
    save_jvm_string(env, &driver_lib, library_name);
    save_jvm_string(env, &driver_path, library_search_path);
}

// This function is directly tied to Adreno Vulkan driver hence it's located here
JNIEXPORT void JNICALL
Java_git_artdeell_mojoexec_MojoExec_setQglConfigPath(JNIEnv *env, jclass clazz,
                                                          jstring config_path) {
    save_jvm_string(env, &config_override, config_path);
}