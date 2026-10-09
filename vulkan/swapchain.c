//
// Created by whbex on 09.10.2026.
//

#include <android/native_window.h>
#include <malloc.h>
#include "native_window.h"
#include "swapchain.h"
#include "unordered_map/int_hash.h"

static unordered_map *surfaces;

static VkSurfaceTransformFlagBitsKHR util_RotationToTransform(uint32_t hint){
    switch(hint) {
        case ANATIVEWINDOW_TRANSFORM_ROTATE_90: return VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR;
        case ANATIVEWINDOW_TRANSFORM_ROTATE_180: return VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR;
        case ANATIVEWINDOW_TRANSFORM_ROTATE_270: return VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR;
        default: return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
}



static int32_t query_transform(ANativeWindow *window) {
    __ANativeWindow *anw = (__ANativeWindow*) window;
    int32_t flag = ANATIVEWINDOW_TRANSFORM_IDENTITY;
    if(anw->query) {
        anw->query(anw, ANATIVEWINDOW_QUERY_TRANSFORM_HINT, &flag);
    }
    return flag;
}

static void set_transform(ANativeWindow *window, int32_t transform) {
    __ANativeWindow *anw = (__ANativeWindow*) window;
    if(anw->perform) {
        anw->perform(anw, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, transform);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_swapchain(
        VkDevice                                    device,
        const VkSwapchainCreateInfoKHR*             pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkSwapchainKHR*                             pSwapchain
) {
    VkSwapchainCreateInfoKHR sci = *pCreateInfo;
    ANativeWindow *current_window = surfaces ? unordered_map_get(surfaces,
                                                                 (void *) pCreateInfo->surface) : NULL;
    if(current_window) {
        int32_t flag = query_transform(current_window);
        sci.preTransform = util_RotationToTransform(flag);
        printf("MojoExec: current transform %d on window : %p\n", flag, current_window);
    }
    PFN_vkCreateSwapchainKHR func = (PFN_vkCreateSwapchainKHR) host_vkGetDeviceProcAddr(device,
                                                                                        "vkCreateSwapchainKHR");
    VkResult res = func(device, &sci, pAllocator, pSwapchain);
    if(current_window) set_transform(current_window, ANATIVEWINDOW_TRANSFORM_IDENTITY);
    return res;
}

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_surface(
        VkInstance                                  instance,
        const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkSurfaceKHR*                               pSurface
        ) {
    if(!surfaces) surfaces = alloc_intmap_safe();
    PFN_vkCreateAndroidSurfaceKHR func = (PFN_vkCreateAndroidSurfaceKHR) host_vkGetInstanceProcAddr(
            instance, "vkCreateAndroidSurfaceKHR");
    VkResult res = func(instance, pCreateInfo, pAllocator, pSurface);
    if(surfaces && res == VK_SUCCESS) {
        unordered_map_put(surfaces, (void *) *pSurface, pCreateInfo->window);
    }
    return res;
}

VKAPI_ATTR void VKAPI_CALL mjvk_destroy_surface(
        VkInstance                                  instance,
        VkSurfaceKHR                                surface,
        const VkAllocationCallbacks*                pAllocator) {
    if(surfaces) unordered_map_remove(surfaces, (void *) surface);
    CALL_HOST(vkGetInstanceProcAddr, instance, vkDestroySurfaceKHR, instance, surface, pAllocator);
}
