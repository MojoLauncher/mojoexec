//
// Created by whbex on 09.10.2026.
//

#include "swapchain.h"

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
