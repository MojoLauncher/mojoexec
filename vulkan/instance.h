//
// Created by whbex on 09.10.2026.
//

#ifndef POJAVLAUNCHER_INSTANCE_H
#define POJAVLAUNCHER_INSTANCE_H

#include "vk_wrapper.h"

VKAPI_ATTR VkResult VKAPI_CALL mjvk_create_instance(
        const VkInstanceCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocationCallbacks,
        VkInstance *pInstance);


#endif //POJAVLAUNCHER_INSTANCE_H
