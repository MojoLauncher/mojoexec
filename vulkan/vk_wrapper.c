//
// Created by whbex on 08.10.2026.
//

// A tiny Vulkan wrapper

#include <dlfcn.h>

#include "mojoexec.h"

#include "vk_wrapper.h"
#include "instance.h"
#include "swapchain.h"

static void* mjvk_handle = NULL;

PFN_vkGetInstanceProcAddr host_vkGetInstanceProcAddr;
PFN_vkGetDeviceProcAddr host_vkGetDeviceProcAddr;

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* proc) {
    if(!mjvk_handle) mjvk_handle = mojoexec_acq_vulkan_handle();
    if(!host_vkGetInstanceProcAddr) host_vkGetInstanceProcAddr = dlsym(mjvk_handle, "vkGetInstanceProcAddr");
    CHECK_VK(vkGetInstanceProcAddr, instance, proc);
    OVERRIDE_VK("vkGetDeviceProcAddr", vkGetDeviceProcAddr);
    OVERRIDE_VK("vkCreateInstance", mjvk_create_instance);
    OVERRIDE_VK("vkCreateSwapchainKHR", mjvk_create_swapchain);
    OVERRIDE_VK("vkCreateAndroidSurfaceKHR", mjvk_create_surface);
    OVERRIDE_VK("vkDestroySurfaceKHR", mjvk_destroy_surface);
    return func;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* proc) {
    if(!mjvk_handle) mjvk_handle = mojoexec_acq_vulkan_handle();
    if(!host_vkGetDeviceProcAddr) host_vkGetDeviceProcAddr = dlsym(mjvk_handle, "vkGetDeviceProcAddr");
    CHECK_VK(vkGetDeviceProcAddr, device, proc);
    OVERRIDE_VK("vkCreateSwapchainKHR", mjvk_create_swapchain);
    OVERRIDE_VK("vkCreateAndroidSurfaceKHR", mjvk_create_surface);
    OVERRIDE_VK("vkDestroySurfaceKHR", mjvk_destroy_surface);
    return func;
}
