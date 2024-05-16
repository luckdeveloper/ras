#define _XOPEN_SOURCE 700
#include <fcntl.h> /* open */
#include <stdint.h> /* uint64_t  */
#include <stdio.h> /* printf */
#include <stdlib.h> /* size_t */
#include <unistd.h> /* pread, sysconf */
#include <errno.h>
#include <stdbool.h>
#include <string.h>

// https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html?highlight=kpageflags
// https://man.archlinux.org/man/proc_kpageflags.5.en
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

typedef struct input_arg
{   
    int is_physical_address;
    union kpageflags_read
    {
        uintptr_t physical_address;
        uint64_t pfn;
    }u;    
}input_arg_s;

#define PF_FLAGS_ARRAY_LEN (sizeof(page_flags_str)/sizeof(page_flags_str[0]))


int parse_args(int argc, char **argv);
uint64_t calculate_pfn(uintptr_t physical_address);
int get_pageflags_by_pfn(uint64_t pfn, uint64_t *ret_flags);
int print_pageflags(uint64_t flags);

input_arg_s g_input_args;

int main(int argc, char **argv)
{
    uintptr_t phy_addr = 0;
    uint64_t pfn = 0;
    uint64_t flags = 0;

  
    if (EXIT_SUCCESS != parse_args(argc, argv))
    {
        return -1;
    }
    
    if (g_input_args.is_physical_address)
    {
        phy_addr = g_input_args.u.physical_address;
        pfn = calculate_pfn(phy_addr);
    }
    else
    {
        pfn = g_input_args.u.pfn;
    }
    
    if (0 != get_pageflags_by_pfn(pfn, &flags))
    {
        fprintf(stderr, "get_pageflags_by_pfn\n");
        return EXIT_FAILURE;
    }

    printf("Page Frame Number (PFN) for physical address 0x%lx: 0x%lx\n", phy_addr, pfn);
    (void)print_pageflags(flags);

    return EXIT_SUCCESS;
}



int parse_args(int argc, char **argv)
{
    char *physical_address = NULL;  
    char *pysical_frame_number = NULL;  
    int found_a = 0;  
    int found_n = 0;  
  
    if (argc < 2) {  
        fprintf(stderr, "Usage: %s -a physical_address -n pysical_frame_number\n", argv[0]);  
        return EXIT_FAILURE;  
    }  
  
    for (int i = 1; i < argc; ++i) 
    {  
        if (strcmp(argv[i], "-a") == 0) 
        {  
            if (i + 1 >= argc || argv[i + 1][0] == '-') {  
                fprintf(stderr, "Error: Missing parameter for -a\n");  
                return EXIT_FAILURE;  
            }  
            physical_address = argv[i + 1];  
            g_input_args.is_physical_address = true;
            g_input_args.u.physical_address = strtoull(physical_address, NULL, 0);

            found_a = 1;  
            i++; // Skip the next argument (which is the parameter for -a)  
        } 
        else if (strcmp(argv[i], "-n") == 0) 
        {  
            if (i + 1 >= argc || argv[i + 1][0] == '-') 
            {  
                fprintf(stderr, "Error: Missing parameter for -n");  
                return EXIT_FAILURE;  
            }  
            pysical_frame_number = argv[i + 1];  
            g_input_args.is_physical_address = false;
            g_input_args.u.pfn = strtoull(pysical_frame_number, NULL, 0);
            found_n = 1;  
            i++; // Skip the next argument (which is the parameter for -n)  
        } 
        else 
        {  
            fprintf(stderr, "Error: Unrecognized option '%s'\n", argv[i]);  
            return EXIT_FAILURE;  
        }  
    }  
  
    if (!found_a && !found_n)
    {  
        fprintf(stderr, "Error: either -a or -n options is required\n");  
        return EXIT_FAILURE;  
    }  
  
    return EXIT_SUCCESS;  
}

size_t get_page_size()
{
    return (size_t)sysconf(_SC_PAGESIZE);
}

uint64_t calculate_pfn(uintptr_t physical_address)
{
    size_t page_size = get_page_size();
    return (uint64_t)(physical_address / page_size);
}

/// @brief 
/// @param pfn 
/// @param ret_flags 
/// @return 0: success, other value: failed
int get_pageflags_by_pfn(uint64_t pfn, uint64_t *ret_flags)
{
    int pageflags_fd = -1;
    off_t seek_ret = -1;
    uint64_t flags = 0;
    ssize_t read_ret = -1;



    pageflags_fd = open("/proc/kpageflags", O_RDONLY);
    if (pageflags_fd < 0)
    {
        fprintf(stderr, "open /proc/kpageflags failed, errorno is %d\n", errno);
        return 1;
    }

    seek_ret = lseek(pageflags_fd, PAGE_FLAGS_LEN*pfn, SEEK_SET);
    if (seek_ret == -1)
    {
        fprintf(stderr, "lseek /proc/kpageflags failed, errorno is %d\n", errno);
        close(pageflags_fd);
        return 1;
    }

    read_ret = read(pageflags_fd, &flags, PAGE_FLAGS_LEN);
    if (read_ret == -1)
    {
        fprintf(stderr, "read /proc/kpageflags failed, errorno is %d\n", errno);
        close(pageflags_fd);
        return 1;
    }

    *ret_flags = flags;
    close(pageflags_fd);
    return 0;

 
}


/// @brief 
/// @param flags 
/// @return 0: success, other value: failed
int print_pageflags(uint64_t flags)
{
    int i = 0;

    fprintf(stdout, "*-------pageflags from /proc/kpageflags-----*\n");
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