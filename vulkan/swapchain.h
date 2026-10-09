//
// Created by whbex on 09.10.2026.
//


#ifndef POJAVLAUNCHER_SWAPCHAIN_H
#define POJAVLAUNCHER_SWAPCHAIN_H

#include "vk_wrapper.h"

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_swapchain(
        VkDevice                                    device,
        const VkSwapchainCreateInfoKHR*             pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkSwapchainKHR*                             pSwapchain
);

#endif //POJAVLAUNCHER_SWAPCHAIN_H
