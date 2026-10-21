//
// Created by whbex on 12.08.2026.
//

#ifndef POJAVLAUNCHER_UTILS_H
#define POJAVLAUNCHER_UTILS_H

#include <jni.h>
#include "driver_helper/nsbypass.h"

void save_jvm_string(JNIEnv* env, char** target, jstring str);
struct android_namespace_t *get_primary_namespace();

#endif //POJAVLAUNCHER_UTILS_H
