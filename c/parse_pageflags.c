#define _XOPEN_SOURCE 700
#include <fcntl.h> /* open */
#include <stdint.h> /* uint64_t  */
#include <stdio.h> /* printf */
#include <stdlib.h> /* size_t */
#include <unistd.h> /* pread, sysconf */
#include <errno.h>

#define PAGE_FLAGS_LEN 8 // every page flags len in /proc/kpageflags

enum page_flags_type 
{
    PF_LOCKED = 0,
    PF_ERROR = 1,
    PF_REFERENCED = 2,
    PF_UPTODATE = 3,
    PF_DIRTY = 4,
    PF_LRU = 5,
    PF_ACTIVE =6, 
    PF_SLAB = 7,
    PF_WRITEBACK = 8,
    PF_RECLAIM = 9,
    PF_BUDDY = 10,
    PF_MMAP = 11,
    PF_ANON = 12,
    PF_SWAPCACHE = 13,
    PF_SWAPBACKED = 14,
    PF_COMPOUND_HEAD = 15,
    PF_COMPOUND_TAIL = 16,
    PF_HUGE = 17,
    PF_UNEVICTABLE = 18, 
    PF_HWPOISON = 19,
    PF_NOPAGE = 20,
    PF_KSM = 21,
    PF_THP = 22,
    PF_OFFLINE = 23, 
    PF_ZERO_PAGE = 24,
    PF_IDLE = 25,
    PF_PGTABLE = 26,
};

static const char * const page_flags_str [] = 
{
    [PF_LOCKED]         = "locked",
    [PF_ERROR]          = "error",
    [PF_REFERENCED]     = "referenced",
    [PF_UPTODATE]       = "uptodate",
    [PF_DIRTY]          = "dirty",
    [PF_LRU]            = "lru",
    [PF_ACTIVE]         = "active", 
    [PF_SLAB]           = "slab",
    [PF_WRITEBACK]      = "writeback",
    [PF_RECLAIM]        = "reclaim",
    [PF_BUDDY]          = "buddy",
    [PF_MMAP]           = "mmap",
    [PF_ANON]           = "anon",
    [PF_SWAPCACHE]      = "swapcache",
    [PF_SWAPBACKED]     = "swapbacked",
    [PF_COMPOUND_HEAD]  = "compound head",
    [PF_COMPOUND_TAIL]  = "compound tail",
    [PF_HUGE]           = "huge",
    [PF_UNEVICTABLE]    = "unevictable",
    [PF_HWPOISON]       = "hwpoison",
    [PF_NOPAGE]         = "nopage",
    [PF_KSM]            = "ksm",
    [PF_THP]            = "thp",
    [PF_OFFLINE]        = "offline",
    [PF_ZERO_PAGE]      = "zero page",
    [PF_IDLE]           = "idle",
    [PF_PGTABLE]        = "pgtable",
};

#define PF_FLAGS_ARRAY_LEN (sizeof(page_flags_str)/sizeof(page_flags_str[0]))


int print_usage(void);
int print_pageflags(uint64_t flags);

int main(int argc, char **argv)
{
    uint64_t flags = 0;

    if (argc != 2) 
    {
        print_usage();  
        return -1;
    }
    
    flags = strtoull(argv[1], NULL, 0);
    fprintf(stdout, "parse pageflags: %lu, 0x%lx\n", flags, flags);
    (void)print_pageflags(flags);

    return 0;
}

int print_usage(void)
{
    fprintf(stdout, "parse_pageflags [page flags]\n");
    return 0;
}

/// @brief 
/// @param flags 
/// @return 0: success, other value: failed
int print_pageflags(uint64_t flags)
{
    int i = 0;

    fprintf(stdout, "*-------pageflags from stdin -----*\n");
    for (i = 0; i < PF_FLAGS_ARRAY_LEN; i++)
    {
        int enable = 0;

        if (flags & (((uint64_t)1) << i))
        {
            enable = 1;
        }
        else
        {
            enable = 0;
        }

        fprintf(stdout, "%-24s %-8d\n", page_flags_str[i], enable);
    }

    return 0;    
}