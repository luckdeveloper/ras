

#include <fcntl.h> /* open */
#include <stdint.h> /* uint64_t  */
#include <stdio.h> /* printf */
#include <stdlib.h> /* size_t */
#include <unistd.h> /* pread, sysconf */
#include <errno.h>

#define CONFIG_MIGRATION
#define CONFIG_DEVICE_PRIVATE
#define CONFIG_MEMORY_FAILURE
#define CONFIG_64BIT

/*
 * MAX_SWAPFILES defines the maximum number of swaptypes: things which can
 * be swapped to.  The swap type and the offset into that swap type are
 * encoded into pte's and into pgoff_t's in the swapcache.  Using five bits
 * for the type means that the maximum number of swapcache pages is 27 bits
 * on 32-bit-pgoff_t architectures.  And that assumes that the architecture packs
 * the type/offset into the pte as 5/27 as well.
 */
#define MAX_SWAPFILES_SHIFT	5

/*
 * Use some of the swap files numbers for other purposes. This
 * is a convenient way to hook into the VM to trigger special
 * actions on faults.
 */

/*
 * Unaddressable device memory support. See include/linux/hmm.h and
 * Documentation/vm/hmm.rst. Short description is we need struct pages for
 * device memory that is unaddressable (inaccessible) by CPU, so that we can
 * migrate part of a process memory to device memory.
 *
 * When a page is migrated from CPU to device, we set the CPU page table entry
 * to a special SWP_DEVICE_{READ|WRITE} entry.
 *
 * When a page is mapped by the device for exclusive access we set the CPU page
 * table entries to special SWP_DEVICE_EXCLUSIVE_* entries.
 */
#ifdef CONFIG_DEVICE_PRIVATE
#define SWP_DEVICE_NUM 4
#define SWP_DEVICE_WRITE (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM)
#define SWP_DEVICE_READ (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+1)
#define SWP_DEVICE_EXCLUSIVE_WRITE (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+2)
#define SWP_DEVICE_EXCLUSIVE_READ (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+3)
#else
#define SWP_DEVICE_NUM 0
#endif

/*
 * NUMA node memory migration support
 */
#ifdef CONFIG_MIGRATION
#define SWP_MIGRATION_NUM 2
#define SWP_MIGRATION_READ	(MAX_SWAPFILES + SWP_HWPOISON_NUM)
#define SWP_MIGRATION_WRITE	(MAX_SWAPFILES + SWP_HWPOISON_NUM + 1)
#else
#define SWP_MIGRATION_NUM 0
#endif

/*
 * Handling of hardware poisoned pages with memory corruption.
 */
#ifdef CONFIG_MEMORY_FAILURE
#define SWP_HWPOISON_NUM 1
#define SWP_HWPOISON		MAX_SWAPFILES
#else
#define SWP_HWPOISON_NUM 0
#endif

#define MAX_SWAPFILES \
	((1 << MAX_SWAPFILES_SHIFT) - SWP_DEVICE_NUM - \
	SWP_MIGRATION_NUM - SWP_HWPOISON_NUM)

#ifdef CONFIG_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif /* CONFIG_64BIT */

#define BITS_PER_XA_VALUE	(BITS_PER_LONG - 1)
#define SWP_TYPE_SHIFT	(BITS_PER_XA_VALUE - MAX_SWAPFILES_SHIFT)
 /*
  * A swap entry has to fit into a "unsigned long", as the entry is hidden
  * in the "index" field of the swapper address space.
  */
typedef struct {
	unsigned long val;
} swp_entry_t;



/*
 * Extract the `type' field from a swp_entry_t.  The swp_entry_t is in
 * arch-independent format
 */
static inline unsigned swp_type(swp_entry_t entry)
{
	return (entry.val >> SWP_TYPE_SHIFT);
}

static inline int is_hwpoison_entry(swp_entry_t entry)
{
	return swp_type(entry) == SWP_HWPOISON;
}

static inline int is_migration_entry(swp_entry_t entry)
{
	return (swp_type(entry) == SWP_MIGRATION_READ ||
			swp_type(entry) == SWP_MIGRATION_WRITE);
}

static inline int is_device_private_entry(swp_entry_t entry)
{
	int type = swp_type(entry);
	return type == SWP_DEVICE_READ || type == SWP_DEVICE_WRITE;
}

static inline int is_device_exclusive_entry(swp_entry_t entry)
{
	return swp_type(entry) == SWP_DEVICE_EXCLUSIVE_READ ||
		swp_type(entry) == SWP_DEVICE_EXCLUSIVE_WRITE;
}

int print_usage(void);

int main(int argc, char* argv[])
{
    unsigned long input = 0;
    swp_entry_t entry;

    if (argc != 2) 
    {
        print_usage();  
        return -1;
    }
    
    input = strtoull(argv[1], NULL, 0);
    entry.val = input;
    
    fprintf(stdout, "swap entry: %lu, 0x%lx\n*", input, input);

    if (is_hwpoison_entry(entry))
    {
        fprintf(stdout, "%-24s %-8s\n", "hwpoison", "yes");
    }
    else if (is_migration_entry(entry))
    {
        fprintf(stdout, "%-24s %-8s\n", "migration", "yes");
    }
    else if (is_device_private_entry(entry))
    {
        fprintf(stdout, "%-24s %-8s\n", "device private", "yes");
    }
    else if (is_device_exclusive_entry(entry))
    {
        fprintf(stdout, "%-24s %-8s\n", "device exclusive", "yes");
    }
    else
    {
        fprintf(stdout, "%-24s %-8s\n", "unkown type", "???");
    }

    return 0;
}

int print_usage(void)
{
    printf("parse_swapentry [swap entry]\n");
    return 0;
}
