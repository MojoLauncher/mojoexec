//
// Created by maks on 05.06.2023.
//
#include "nsbypass.h"
#include "android_namespace_func.h"
#include <dlfcn.h>
#include <android/dlext.h>
#include <android/log.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <string.h>
#include <stdio.h>
#include <linux/limits.h>
#include <errno.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <elf.h>
#include <elf_defs.h>
#include <inttypes.h>
#include "paths.h"

#define TAG __FILE_NAME__
#include <log.h>

/* Library search path */
#ifdef PLATFORM_64
#define SEARCH_PATH "/system/lib64" // used inside dlopen unique
#define LINKER_SEARCH_PATH "/system/lib64:/vendor/lib64:/system_ext/lib64" // used in namespace linker
#else
#define SEARCH_PATH "/system/lib" // used inside dlopen unique
#define LINKER_SEARCH_PATH "/system/lib:/vendor/lib:/system_ext/lib" // used in namespace linker
#endif


struct android_namespace_t *linker_ns_load(const char* lib_search_path, const char* name) {
    android_ldfuncs_t ldfuncs;
    if(!locate_namespace_funcs(&ldfuncs)) {
        return false;
    }

    printf("MojoExec: creating new namespace %s : %s\n", name, lib_search_path);

    struct android_namespace_t *ns = ldfuncs.create_namespace(name,
                                                lib_search_path,LINKER_SEARCH_PATH,
                                                      3 /* TYPE_SHAFED | TYPE_ISOLATED */,
                                                      "/system/:/system_ext/:/data/:/vendor/:/apex/", NULL);
    // THIS IS VERY IMPORTANT and how I trolled FoldCraft:
    // You need to link the new driver_namespace with NULL and and add ld-android.so
    // in the link list, to pass through the driver_namespace correctly.
    // Not doing this fucks up internal __loader symbol lookup
    // inside the new driver_namespace, thus breaking it on
    // a lot of android versions
    // FoldCraft got trolled because they copied the
    // old broken code verbatim and didn't even test it thoroughly
    ldfuncs.link_namespaces(ns, NULL, "ld-android.so");
    // Also establish links to use the libnativeloader(_lazy).so libraries
    // from the global namespace. This is a workaround for an EMUI issue where
    // the newly loaded libnativeloader_lazy for some unknown reason links
    // to itself and causes a deadlock when loading the vulkan driver.
    ldfuncs.link_namespaces(ns, NULL, "libnativeloader.so");
    ldfuncs.link_namespaces(ns, NULL, "libnativeloader_lazy.so");
    if(ldfuncs.dl_handle && ldfuncs.close) ldfuncs.close(ldfuncs.dl_handle);
    return ns;
}

void* linker_ns_dlopen(struct android_namespace_t * namespace, const char* name, int flag) {
    android_dlextinfo dlextinfo;
    dlextinfo.flags = ANDROID_DLEXT_USE_NAMESPACE;
    dlextinfo.library_namespace = namespace;
    return android_dlopen_ext(name, flag, &dlextinfo);
}

bool patch_elf_soname(int patchfd, int realfd, size_t size, const char* patchname) {
    char* target = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, patchfd, 0);
    if(!target) return false;
    if(read(realfd, target, size) != size) goto fail;


    ELF_EHDR *ehdr = (ELF_EHDR*)target;
    ELF_SHDR *shdr = (ELF_SHDR*)(target + ehdr->e_shoff);
    for(ELF_HALF i = 0; i < ehdr->e_shnum; i++) {
        ELF_SHDR *hdr = &shdr[i];
        if(hdr->sh_type == SHT_DYNAMIC) {
            char* strtab = target + shdr[hdr->sh_link].sh_offset;
            // If there's a warning below, it's bogus, ignore it
            ELF_DYN *dynEntries = (ELF_DYN*)(target + hdr->sh_offset);
            for(ELF_XWORD k = 0; k < (hdr->sh_size / hdr->sh_entsize);k++) {
                ELF_DYN* dynEntry = &dynEntries[k];
                if(dynEntry->d_tag == DT_SONAME) {
                    char* soname = strtab + dynEntry->d_un.d_val;
                    size_t soname_len = strlen(soname);
                    size_t patchname_len = strlen(patchname);
                    if(patchname_len != soname_len) goto fail;

                    strcpy(soname, patchname);
                    munmap(target, size);
                    return true;
                }
            }
        }
    }

    fail:
    munmap(target, size);
    return false;
}

#define PAGE_ALIGN(addr)        (((addr)+pagesize-1)&(~(pagesize-1)))

void* linker_ns_dlopen_unique(struct android_namespace_t * namespace, const char* tmpdir, const char* name, const char* patch_name, int flags) {
    int pagesize = getpagesize();
    char pathbuf[PATH_MAX];
    static uint16_t patchid;
    int patch_fd, real_fd;
    size_t fsize, totalsize;

    snprintf(pathbuf, PATH_MAX, "%s/%s", SEARCH_PATH, name);
    real_fd = open(pathbuf, O_RDONLY);
    if(real_fd == -1) {
        printf("MojoExec: failed to open original library: %s\n", pathbuf);
        return NULL;
    }

    struct stat64 real_stat;
    if (fstat64(real_fd, &real_stat)) goto fail_real;
    fsize = real_stat.st_size;
    totalsize = PAGE_ALIGN(fsize);

    patch_fd = (int) syscall(__NR_memfd_create, patch_name, MFD_CLOEXEC);
    if(patch_fd == -1) {
        // TODO: use ASharedMemory as fallback
        // NOTE: use page-aligned size (totalsize) for ashmem
        snprintf(pathbuf, PATH_MAX, "%s/%"PRIu16"", tmpdir, patchid++);
        patch_fd = open(pathbuf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    }
    if(patch_fd == -1) {
        printf("MojoExec: failed to open patched library: %s\n", pathbuf);
        goto fail_real;
    }

    if(ftruncate64(patch_fd, totalsize) == -1) {
        printf("MojoExec: failed to set patched library size!\n");
        goto fail_both;
    }

    bool patch_result = patch_elf_soname(patch_fd, real_fd, fsize, patch_name);
    close(real_fd);
    if(!patch_result) {
        printf("MojoExec: failed to apply soname patch %s!\n", patch_name);
        close(patch_fd);
        return NULL;
    }

    android_dlextinfo extinfo;
    extinfo.flags = ANDROID_DLEXT_USE_NAMESPACE | ANDROID_DLEXT_USE_LIBRARY_FD;
    extinfo.library_fd = patch_fd;
    extinfo.library_namespace = namespace;
    return android_dlopen_ext(patch_name, flags, &extinfo);

    fail_both:
    close(patch_fd);
    fail_real:
    close(real_fd);
    return NULL;
}
