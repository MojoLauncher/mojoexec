//
// Created by whbex on 09.10.2026.
//


#ifndef POJAVLAUNCHER_SWAPCHAIN_H
#define POJAVLAUNCHER_SWAPCHAIN_H

#include "vk_wrapper.h"

typedef VkResult (VKAPI_ATTR *PFN_vkCreateAndroidSurfaceKHR)(VkInstance,const VkAndroidSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_swapchain(
        VkDevice                                    device,
        const VkSwapchainCreateInfoKHR*             pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkSwapchainKHR*                             pSwapchain
);

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_surface(
        VkInstance                                  instance,
        const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkSurfaceKHR*                               pSurface
);

VKAPI_ATTR void VKAPI_CALL mjvk_destroy_surface(
        VkInstance                                  instance,
        VkSurfaceKHR                                surface,
        const VkAllocationCallbacks*                pAllocator);

#endif //POJAVLAUNCHER_SWAPCHAIN_H
