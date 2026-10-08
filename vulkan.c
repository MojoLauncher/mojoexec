//
// Created by whbex on 08.10.2026.
//

#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "mojoexec.h"

static void* mjvk_handle = NULL;

static PFN_vkGetInstanceProcAddr host_VkGetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr host_VkGetDeviceProcAddr;

#define OVERRIDE_VK(procname, function) if(!strcmp(proc, procname)) return (PFN_vkVoidFunction) function;

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_instance(
        const VkInstanceCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocationCallbacks,
        VkInstance *pInstance) {
    printf("MojoExec: creating Vulkan instance!\n");
    PFN_vkCreateInstance proc = (PFN_vkCreateInstance) host_VkGetInstanceProcAddr(NULL, "vkCreateInstance");
    return proc(pCreateInfo, pAllocationCallbacks, pInstance);
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
    PFN_vkCreateSwapchainKHR proc = (PFN_vkCreateSwapchainKHR) host_VkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
    return proc(device, &sci, pAllocator, pSwapchain);
}


VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* proc) {
    if(!mjvk_handle) mjvk_handle = mojoexec_acq_vulkan_handle();
    if(!host_VkGetInstanceProcAddr) host_VkGetInstanceProcAddr = dlsym(mjvk_handle, "vkGetInstanceProcAddr");
    OVERRIDE_VK("vkGetDeviceProcAddr", vkGetDeviceProcAddr);
    OVERRIDE_VK("vkCreateInstance", mjvk_create_instance);
    return host_VkGetInstanceProcAddr(instance, proc);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* proc) {
    if(!mjvk_handle) mjvk_handle = mojoexec_acq_vulkan_handle();
    if(!host_VkGetDeviceProcAddr) host_VkGetDeviceProcAddr = dlsym(mjvk_handle, "vkGetDeviceProcAddr");
    OVERRIDE_VK("vkCreateSwapchainKHR", mjvk_create_swapchain);
    return host_VkGetDeviceProcAddr(device, proc);
}
