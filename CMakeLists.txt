cmake_minimum_required(VERSION 3.22.1)

project(mojoexec DESCRIPTION "Android porting utility library")

add_library(mojoexec SHARED
        vulkan_loader.c
        mojoexec.c
        utils.c
        driver_helper/nsbypass.c
        driver_helper/fake_dlfcn.c
        driver_helper/func_locator.c)

target_include_directories(mojoexec PRIVATE ".")
target_include_directories(mojoexec PUBLIC include)

target_link_libraries(mojoexec PRIVATE log)

if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    target_sources(mojoexec PRIVATE driver_helper/arm64_func_locator.c)
    target_compile_definitions(mojoexec PRIVATE USE_ARM64_LOCATOR)
    target_compile_definitions(mojoexec PRIVATE ENABLE_VULKAN_OVERRIDE)

    add_library(linkerhook SHARED
            driver_helper/hook.c
    )
    target_link_options(linkerhook PRIVATE "-Wl,-z,global")
endif()

target_link_options(mojoexec PRIVATE "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/version.script")

set_target_properties(mojoexec PROPERTIES
        LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/version.script"
)