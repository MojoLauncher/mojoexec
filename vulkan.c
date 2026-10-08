//
// Created by whbex on 08.10.2026.
//

#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "mojoexec.h"

static void* mjvk_handle = NULL;

static PFN_vkGetInstanceProcAddr host_vkGetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr host_vkGetDeviceProcAddr;

#define __STRINGIFY(str) #str

#define OVERRIDE_VK(procname, function) \
if(!strcmp(proc, procname)) { \
    printf("MojoExec: overridden " procname "\n"); \
    return (PFN_vkVoidFunction) function;                                 \
} \

// This will always acquire functions without caching hence don't use it in hotspot places
#define CALL_HOST(provider, inst,  proc, ...) \
PFN_ ## proc func = (PFN_ ## proc) host_ ## provider (inst, #proc);\
return func(__VA_ARGS__) \

#define CHECK_VK(provider, inst, proc) \
PFN_vkVoidFunction func = host_ ## provider (inst, proc); \
if(!func) return VK_NULL_HANDLE;


VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_instance(
        const VkInstanceCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocationCallbacks,
        VkInstance *pInstance) {
    printf("MojoExec: creating Vulkan instance!\n");
    CALL_HOST(vkGetInstanceProcAddr, NULL, vkCreateInstance, pCreateInfo, pAllocationCallbacks, pInstance);
}

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_swapchain(
        VkDevice                                    device,
        const VkSwapchainCreateInfoKHR*             pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkSwapchainKHR*                             pSwapchain
        ) {
    VkSwapchainCreateInfoKHR sci = *pCreateInfo;
    sci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    printf("MojoExec: creating Vulkan swapchain!\n");
    CALL_HOST(vkGetDeviceProcAddr, device, vkCreateSwapchainKHR, device, &sci, pAllocator, pSwapchain);
}


VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* proc) {
    if(!mjvk_handle) mjvk_handle = mojoexec_acq_vulkan_handle();
    if(!host_vkGetInstanceProcAddr) host_vkGetInstanceProcAddr = dlsym(mjvk_handle, "vkGetInstanceProcAddr");
    CHECK_VK(vkGetInstanceProcAddr, instance, proc);
    OVERRIDE_VK("vkGetDeviceProcAddr", vkGetDeviceProcAddr);
    OVERRIDE_VK("vkCreateInstance", mjvk_create_instance);
    OVERRIDE_VK("vkCreateSwapchainKHR", mjvk_create_swapchain);
    return func;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* proc) {
    if(!mjvk_handle) mjvk_handle = mojoexec_acq_vulkan_handle();
    if(!host_vkGetDeviceProcAddr) host_vkGetDeviceProcAddr = dlsym(mjvk_handle, "vkGetDeviceProcAddr");
    CHECK_VK(vkGetDeviceProcAddr, device, proc);
    OVERRIDE_VK("vkCreateSwapchainKHR", mjvk_create_swapchain);
    return func;
}
