//
// Created by maks on 11.05.2026.
//

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>
#include <elf_defs.h>
#include <android/log.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>

#define TAG __FILE_NAME__
#include <log.h>

#define LOG_DBG

#ifdef LOG_DBG
#define log_dbg LOGI
#else
#define log_dbg(...)
#endif

struct ctx {
    void *load_addr;
    void *dynstr;
    void *dynsym;
    ELF_XWORD nsyms;
};

int fake_dlclose(void *handle)
{
    if(handle) {
        struct ctx *ctx = (struct ctx *) handle;
        if(ctx->dynsym) free(ctx->dynsym);	/* we're saving dynsym and dynstr */
        if(ctx->dynstr) free(ctx->dynstr);	/* from library file just in case */
        free(ctx);
    }
    return 0;
}

bool try_find_map_entry(const char* name, char outpath[1024], uintptr_t *load_addr) {
    FILE *maps = fopen("/proc/self/maps", "r");
    if(!maps) return false;

    char buff[1024] = {0};
    uintptr_t start, end, offset;
    int major, minor, path_offset;
    unsigned long inode;
    char perms[4];
    bool found = false;

    while(fgets(buff, sizeof(buff), maps)) {
        if(!strstr(buff, name)) continue;
        if(strstr(buff, "[anon:")) continue;

        int parsed = sscanf(buff, "%"SCNxPTR"-%"SCNxPTR" %4c %"SCNxPTR" %x:%x %lu %n",
                            &start, &end, perms, &offset, &major, &minor, &inode, &path_offset);

        if(parsed < 7) continue;

        if(perms[0] != 'r') continue;

        size_t len = strlen(buff);
        buff[--len] = 0;

        *load_addr = start;
        memcpy(outpath, &buff[path_offset], len);
        found = true;

        break;
    }

    fclose(maps);
    return found;
}

/* flags are ignored */

void *fake_dlopen(const char *search, int flags)
{
    struct ctx *ctx = 0;
    uintptr_t load_addr, size;
    int k, fd = -1, found = 0;
    void *shoff;
    ELF_EHDR *elf = MAP_FAILED;

#define fatal(fmt,args...) do { LOGE(fmt,##args); goto err_exit; } while(0)
    char libpath[1024];

    if(!try_find_map_entry(search, libpath, &load_addr))
        fatal("Failed to find the library in maps");

    LOGI("%s loaded in Android at 0x%08lx", libpath, load_addr);
    /* Now, mmap the same library once again */

    fd = open(libpath, O_RDONLY);
    if(fd < 0) fatal("failed to open %s %s", libpath, strerror(errno));

    size = lseek(fd, 0, SEEK_END);
    if(size <= 0) fatal("lseek() failed for %s", libpath);

    elf = (ELF_EHDR *) mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    fd = -1;

    if(elf == MAP_FAILED) fatal("mmap() failed for %s", libpath);

    ctx = (struct ctx *) calloc(1, sizeof(struct ctx));
    if(!ctx) fatal("no memory for %s", libpath);

    ctx->load_addr = (void *) load_addr;
    shoff = ((void *) elf) + elf->e_shoff;

    for(k = 0; k < elf->e_shnum; k++, shoff += elf->e_shentsize)  {

        ELF_SHDR *sh = (ELF_SHDR *) shoff;
        log_dbg("%s: k=%d shdr=%p type=%x", __func__, k, sh, sh->sh_type);

        switch(sh->sh_type) {
            case SHT_DYNSYM:
                if(ctx->dynsym) fatal("%s: duplicate DYNSYM sections", libpath); /* .dynsym */
                ctx->dynsym = malloc(sh->sh_size);
                if(!ctx->dynsym) fatal("%s: no memory for .dynsym", libpath);
                memcpy(ctx->dynsym, ((void *) elf) + sh->sh_offset, sh->sh_size);
                ctx->nsyms = (sh->sh_size/sizeof(ELF_SYM)) ;
                break;

            case SHT_STRTAB:
                if(ctx->dynstr) break;	/* .dynstr is guaranteed to be the first STRTAB */
                ctx->dynstr = malloc(sh->sh_size);
                if(!ctx->dynstr) fatal("%s: no memory for .dynstr", libpath);
                memcpy(ctx->dynstr, ((void *) elf) + sh->sh_offset, sh->sh_size);
                break;
        }
        if(ctx->dynstr && ctx->dynsym) break;
    }

    munmap(elf, size);
    elf = 0;

    if(!ctx->dynstr || !ctx->dynsym) fatal("dynamic sections not found in %s", libpath);

#undef fatal

    log_dbg("%s: ok, dynsym = %p, dynstr = %p", libpath, ctx->dynsym, ctx->dynstr);

    return ctx;

    err_exit:
    if(fd >= 0) close(fd);
    if(elf != MAP_FAILED) munmap(elf, size);
    fake_dlclose(ctx);
    return 0;
}

void *fake_dlsym(void *handle, const char *name)
{
    int k;
    struct ctx *ctx = (struct ctx *) handle;
    ELF_SYM *sym = (ELF_SYM *) ctx->dynsym;
    char *strings = (char *) ctx->dynstr;

    for(k = 0; k < ctx->nsyms; k++, sym++)
        if(strcmp(strings + sym->st_name, name) == 0) {
            /* R NB: Actually! Don't subtract the bias. We don't want the offset of the symbol in
             * the library, we want its offset in the VA space of the process! */
            void *ret = ctx->load_addr + sym->st_value;
            LOGI("%s found at %p %x", name, ret, sym->st_value);
            return ret;
        }
    return 0;
}


