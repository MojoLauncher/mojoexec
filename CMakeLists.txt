cmake_minimum_required(VERSION 3.22.1)

project(mojoexec DESCRIPTION "Android porting utility library")

add_library(mojoexec SHARED
        vulkan_loader.c
        mojoexec.c
        utils.c
        affinity.c
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
    target_compile_definitions(mojoexec PRIVATE ENABLE_QGL_REDIRECT)

    add_library(linkerhook SHARED
            driver_helper/hook.c
    )
    target_link_options(linkerhook PRIVATE "-Wl,-z,global")
    add_library(fopenhook SHARED
            driver_helper/fopen_hook.c)
    target_include_directories(fopenhook PUBLIC include)
    target_include_directories(fopenhook PRIVATE ".")
    target_link_options(fopenhook PRIVATE "-Wl,-z,global")
    target_link_libraries(fopenhook PRIVATE mojoexec)
endif()

target_link_options(mojoexec PRIVATE "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/version.script")

set_target_properties(mojoexec PROPERTIES
        LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/version.script"
)