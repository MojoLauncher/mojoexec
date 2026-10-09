//
// Created by whbex on 09.10.2026.
//

#include "instance.h"

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_instance(
        const VkInstanceCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocationCallbacks,
        VkInstance *pInstance) {
    // TODO: Enable layers
    CALL_HOST(vkGetInstanceProcAddr, NULL, vkCreateInstance, pCreateInfo, pAllocationCallbacks, pInstance);
}
