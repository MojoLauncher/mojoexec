//
// Created by whbex on 12.08.2026.
//

#include <malloc.h>
#include <string.h>
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
