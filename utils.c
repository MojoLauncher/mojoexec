//
// Created by whbex on 12.08.2026.
//

#include <malloc.h>
#include <string.h>
#include <mojoexec.h>
#include "driver_helper/nsbypass.h"
#include "utils.h"

void save_jvm_string(JNIEnv* env, char** target, jstring str) {
    if(*target != NULL) free(*target);
    if(str == NULL) {
        *target = NULL;
        return;
    }

    const char* path = (*env)->GetStringUTFChars(env, str, NULL);
    *target = strdup(path);
    (*env)->ReleaseStringUTFChars(env, str, path);
}

struct android_namespace_t *get_primary_namespace() {
    static struct android_namespace_t* primary_namespace = NULL;
    if(primary_namespace) return primary_namespace;
    if(!mojoexec_native_dir) {
        printf("MojoExec: native library path not set! cannot create primary namespace\n");
        return NULL;
    }
    primary_namespace = linker_ns_load(mojoexec_native_dir, "mojoexec-primary");
    return primary_namespace;
}
