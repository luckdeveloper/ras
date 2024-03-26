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

typedef struct {
    uint64_t pfn : 55;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;

/* Parse the pagemap entry for the given virtual address.
 *
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
    size_t nread;
    ssize_t ret;
    uint64_t data;
    uintptr_t vpn;

    vpn = vaddr / sysconf(_SC_PAGE_SIZE);
    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(pagemap_fd, ((uint8_t*)&data) + nread, sizeof(data) - nread,
                vpn * sizeof(data) + nread);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->pfn = data & (((uint64_t)1 << 55) - 1);
    entry->soft_dirty = (data >> 55) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;

    return 0;
}

/* Convert the given virtual address to physical using /proc/PID/pagemap.
 *
 * @param[out] paddr physical address
 * @param[in]  pid   process to convert for
 * @param[in] vaddr virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int virt_to_phys_user(uintptr_t *paddr, pid_t pid, uintptr_t vaddr)
{
    char pagemap_file[BUFSIZ];
    int pagemap_fd = -1;
    int pageflags_fd = -1;
    off_t seek_ret = -1;
    uint64_t flags = 0;
    ssize_t read_ret = -1;
    int i = 0;


    snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
    pagemap_fd = open(pagemap_file, O_RDONLY);
    if (pagemap_fd < 0) {
        return 1;
    }
    PagemapEntry entry;
    if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
        return 1;
    }
    close(pagemap_fd);
    
    *paddr = (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));
    fprintf(stdout, "%-24s 0x%-16llx\n", "physical addr", (unsigned long long)*paddr);
     fprintf(stdout, "*-------pageflags from /proc/self/pgmap----*\n");
    fprintf(stdout, "%-24s %-8d\n", "soft_dirty", entry.soft_dirty);
    fprintf(stdout, "%-24s %-8d\n", "file_page", entry.file_page);
    fprintf(stdout, "%-24s %-8d\n", "swapped", entry.swapped);
    fprintf(stdout, "%-24s %-8d\n", "present", entry.present);

    //get other PAGE flags from /proc/kpageflags
    pageflags_fd = open("/proc/kpageflags", O_RDONLY);
    if (pageflags_fd < 0)
    {
        fprintf(stderr, "open /proc/kpageflags failed, errorno is %d\n", errno);
        return 1;
    }

    seek_ret = lseek(pageflags_fd, PAGE_FLAGS_LEN*entry.pfn, SEEK_SET);
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
    close(pageflags_fd);

    return 0;
}

int main(int argc, char **argv)
{
    pid_t pid;
    uintptr_t vaddr, paddr = 0;

    if (argc < 3) {
        printf("Usage: %s pid vaddr\n", argv[0]);
        return EXIT_FAILURE;
    }
    pid = strtoull(argv[1], NULL, 0);
    vaddr = strtoull(argv[2], NULL, 0);
    if (virt_to_phys_user(&paddr, pid, vaddr)) {
        fprintf(stderr, "error: virt_to_phys_user\n");
        return EXIT_FAILURE;
    };
    // printf("0x%jx\n", (uintmax_t)paddr);
    return EXIT_SUCCESS;
}
