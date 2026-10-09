//
// Created by whbex on 09.10.2026.
//

#ifndef POJAVLAUNCHER_UTIL_H
#define POJAVLAUNCHER_UTIL_H

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>

extern PFN_vkGetInstanceProcAddr host_vkGetInstanceProcAddr;
extern PFN_vkGetDeviceProcAddr host_vkGetDeviceProcAddr;

#define __STRINGIFY(str) #str

// Override Vulkan function
#define OVERRIDE_VK(procname, function) \
if(!strcmp(proc, procname)) { \
    printf("MojoExec: overridden " procname "\n"); \
    return (PFN_vkVoidFunction) function;                                 \
} \

// This will always acquire functions without caching hence don't use it in hotspot places
#define CALL_HOST(provider, inst,  proc, ...) \
PFN_ ## proc func = (PFN_ ## proc) host_ ## provider (inst, #proc);\
return func(__VA_ARGS__) \

// In some cases an application can try to fetch unexistent functions
// In this case we should return NULL instead of passing them onto our wrapper functions
#define CHECK_VK(provider, inst, proc) \
PFN_vkVoidFunction func = host_ ## provider (inst, proc); \
if(!func) return VK_NULL_HANDLE;

#endif //POJAVLAUNCHER_UTIL_H
