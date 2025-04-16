#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/mman.h>
#include <linux/version.h>
#include <linux/migrate.h>


#define MAP_PAGE_COUNT 10
#define MAPLEN (PAGE_SIZE * MAP_PAGE_COUNT)
#define MAP_DEV_MAJOR 0
#define MAP_DEV_NAME "MAPDRV"


/**
 * @brief address sapce operations
*/
bool mapdrv_aop_isolate_page(struct page *page, isolate_mode_t mode);
int mapdrv_aop_migrate_page(struct address_space *mapping, 
        struct page *newpage, struct page *oldpage, enum migrate_mode mode);
void mapdrv_aop_putback_page(struct page *page);

struct address_space_operations mapdrv_aops = {
    .isolate_page = mapdrv_aop_isolate_page,
    .migratepage = mapdrv_aop_migrate_page,
    .putback_page = mapdrv_aop_putback_page,
};

struct address_space mapdrv_as = {
    .a_ops = &mapdrv_aops,
};

/**
 * @brief device file operations
 * 
 */
static int mapdrv_fop_mmap(struct file *file, struct vm_area_struct *vma);
static int mapdrv_fop_open(struct inode *inode, struct file *file);

static struct file_operations mapdrv_fops = {
    .owner = THIS_MODULE,
    .mmap = mapdrv_fop_mmap,
    .open = mapdrv_fop_open,
};

/**
 * @brief vma operations
 * 
 */
void mapdrv_vma_open(struct vm_area_struct *vma);
void mapdrv_vma_close(struct vm_area_struct *vma);

/* vm area nopage function,
   The parameters of this function vary with different kernel versions
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0))  
    unsigned int mapdrv_vma_fault(struct vm_fault *vmf);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))  
    int mapdrv_vma_fault(struct vm_fault *vmf);
#else
    int mapdrv_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
#endif

static struct vm_operations_struct mapdrv_vm_ops = {
    .open = mapdrv_vma_open,
    .close = mapdrv_vma_close,
    .fault = mapdrv_vma_fault,
};

//////////////////////////////////////////////////////////////////////////////
extern struct mm_struct init_mm;
static char *vmalloc_area = NULL;
MODULE_LICENSE("GPL");

static int __init mapdrv_mod_init(void)
{
    int result;
    unsigned long virt_addr;
    int i = 1;
    struct page *page = NULL;

    result = register_chrdev(MAP_DEV_MAJOR, MAP_DEV_NAME, &mapdrv_fops);
    if (result < 0)
    {
        printk("MAPDRV: register_chrdev failed, return code is %d\n", result);
        return result;
    }
    vmalloc_area = vmalloc(MAPLEN);
    virt_addr = (unsigned long)vmalloc_area;
    for (virt_addr = (unsigned long)vmalloc_area; virt_addr < (unsigned long)vmalloc_area + MAPLEN; virt_addr += PAGE_SIZE)
    {
        page = vmalloc_to_page((void *)virt_addr);
        //SetPageReserved(page);
        __SetPageMovable(page, &mapdrv_as);
        sprintf((char *)virt_addr, "test %d", i++);
    }
    /* printk("vmalloc_area at 0x%lx (phys 0x%lx)\n",(unsigned long)vmalloc_area,(unsigned long)vmalloc_to_pfn((void *)vmalloc_area) << PAGE_SHIFT);  */
    printk("MAPDRV: vmalloc area apply complete!");
    return 0;
}

static void __exit mapdrv_mod_exit(void)
{
    unsigned long virt_addr;
    
    /* unreserve all pages */
    for (virt_addr = (unsigned long)vmalloc_area; virt_addr < (unsigned long)vmalloc_area + MAPLEN; virt_addr += PAGE_SIZE)
    {
        ClearPageReserved(vmalloc_to_page((void *)virt_addr));
    }
    
    /* and free the two areas */
    if (vmalloc_area)
    {
        vfree(vmalloc_area);
    }
    
    unregister_chrdev(MAP_DEV_MAJOR, MAP_DEV_NAME);
    return;
}

//////////////////////////////////////////////////////////////////////////////
static int mapdrv_fop_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long size = vma->vm_end - vma->vm_start;

    if (size > MAPLEN)
    {
        printk("MAPDRV: size too big\n");
        return -ENXIO;
    }
    /*  only support shared mappings. */
    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED))
    {
        printk("MAPDRV: writeable mappings must be shared, rejecting\n");
        return -EINVAL;
    }
    
    /* do not want to have this area swapped out, lock it */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)) 
    vm_flags_set(vma, VM_LOCKONFAULT);
#else 
    vma->vm_flags |= VM_LOCKONFAULT;
#endif

    if (offset == 0)
    {
        vma->vm_ops = &mapdrv_vm_ops;
    }
    else
    {
        printk("MAPDRV: offset out of range\n");
        return -ENXIO;
    }
    return 0;
}

static int mapdrv_fop_open(struct inode *inoe, struct file *file)
{

    printk("MAPDRV: process: %s (%d)\n", current->comm, current->pid);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

void mapdrv_vma_open(struct vm_area_struct *vma)
{
    printk("MAPDRV: mapping vma is opened\n");
}


void mapdrv_vma_close(struct vm_area_struct *vma)
{
    printk("MAPDRV: mapping vma is closed\n");
}

/* page fault handler */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))
    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0))
unsigned int mapdrv_vma_fault(struct vm_fault *vmf)
    #else
int mapdrv_vma_fault(struct vm_fault *vmf)
    #endif
{
    struct page *page = NULL;
    void *page_ptr = NULL;
    unsigned long offset, virt_start, pfn_start;


    printk("\n");
    /*printk("%-25s %d\n","7)PAGE_SHIFT",PAGE_SHIFT);*/
 
    if ((vmf->vma == NULL) || (vmalloc_area == NULL))
    {
        printk("return VM_FAULT_SIGBUS!\n");
        return VM_FAULT_SIGBUS;
    }

    offset = vmf->address - vmf->vma->vm_start;
    if (offset >= MAPLEN)
    {
        printk("return VM_FAULT_SIGBUS!");
        return VM_FAULT_SIGBUS;
    }
    
    virt_start = (unsigned long)vmalloc_area + (unsigned long)(vmf->pgoff << PAGE_SHIFT);
    pfn_start = (unsigned long)vmalloc_to_pfn((void *)virt_start);

    page_ptr = vmalloc_area + offset;
    page = vmalloc_to_page(page_ptr);
    get_page(page);
    /**
    * After fault() has done its work, it should store a pointer to the page 
    * structure for the faulted-in page in the page field 
    */
    vmf->page = page;
    printk("%s: map 0x%lx (0x%016lx) to 0x%lx , size: 0x%lx, page:%ld \n", __func__, virt_start, pfn_start << PAGE_SHIFT, vmf->address, PAGE_SIZE, vmf->pgoff);
   return 0;
}
#else
int mapdrv_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{

    struct page *page = NULL;
    void *page_ptr = NULL;
    unsigned long offset, virt_start, pfn_start;

    if ((vmf->vma == NULL) || (vmalloc_area == NULL))
    {
        printk("MAPDRV: return VM_FAULT_SIGBUS!\n");
        return VM_FAULT_SIGBUS;
    }

    /* 
    * vm_fault::virtual_address: faulting virtual address
    * vm_area_struct::vm_start: start address within vm_mm
    */
    
    offset = (unsigned long)(vmf->virtual_address - vma->vm_start);
    if (offset >= MAPLEN)
    {
        printk("MAPDRV: return VM_FAULT_SIGBUS!");
        return VM_FAULT_SIGBUS;
    }

    /*
    * we assigned allocated memory to fault page 
    */
    page_ptr = vmalloc_area + offset;
    page = vmalloc_to_page(page_ptr);
    get_page(page);
    /**
    * After fault() has done its work, it should store a pointer to the page 
    * structure for the faulted-in page in the page field 
    */
    vmf->page = page;

    /* *******DEBUG OUTPUT ********* */
    virt_start = (unsigned long)vmalloc_area + (unsigned long)(vmf->pgoff << PAGE_SHIFT);
    pfn_start = (unsigned long)vmalloc_to_pfn((void *)virt_start);
    printk("MAPDRV: %s: map VMALLOC：0x%lx (PFN: 0x%016lx) to vm faulting address: 0x%lx , size: 0x%lx, page:%ld \n",   
            __func__, virt_start, pfn_start << PAGE_SHIFT, (unsigned long)vmf->virtual_address, PAGE_SIZE, vmf->pgoff);
    
    return VM_FAULT_MAJOR;
}
#endif

//////////////////////////////////////////////////////////////////////////////

bool mapdrv_aop_isolate_page(struct page *page, isolate_mode_t mode)
{
#if 0    
    if (mode == ISOLATE_UNMAP_AND_AQUIRE)
    {
        if (page_count(page) == 1)
        {
            if (page_mapcount(page) == 0)
            {
                if (page_mapping(page))
                {
                    if (page_mapping(page)->host)
                    {
                        if (page_mapping(page)->host->mm)
#endif 
    // 当前没有什么额外要做的，因为page已经被隔离
    printk("MAPDRV: %s， page: %p, pfn: %lx, mode :%d\n", 
            __func__, page, page_to_pfn(page), mode);
    return true;
}



/// @brief 当页面已经被隔离之后，内核开始调用migrate_page()函数做迁移
/// @param mapping 
/// @param newpage 
/// @param oldpage 
/// @param mode
/// @return 
int mapdrv_aop_migrate_page(struct address_space *mapping, 
        struct page *newpage, struct page *oldpage, enum migrate_mode mode)
{
    void *s_addr, *d_addr;

    printk("MAPDRV: %s, newpage: %p, pfn %lx, oldpage: %p pfn %lx, mode %d \n", 
            __func__, newpage, page_to_pfn(newpage), oldpage, page_to_pfn(oldpage), mode);

	VM_BUG_ON_PAGE(!PageMovable(oldpage), oldpage);
	VM_BUG_ON_PAGE(!PageIsolated(oldpage), oldpage);

    // 加锁？

    // 1. 有哪些数据结构指向oldpage？ 都需要处理
    // a. 在PLMM中， Plmm_pd::busaddress 指向oldpage, 迁移之后, Plmm_pd::busaddress 需要指向newpage 
    //    可是从page中无法反向指向pd， 所以当前无法知道正在迁移PAGE所对应的PD
    //    考虑使用 struct page::private字段保存page所对应的PD指针, 


    // 2. newpage和oldpage之间的拷贝
    s_addr = kmap_atomic(oldpage);
    d_addr = kmap_atomic(newpage);
    memcpy(d_addr, s_addr, PAGE_SIZE);
    kunmap_atomic(s_addr);
    kunmap_atomic(d_addr);

    // 3. 对新page做get
    __SetPageMovable(newpage, page_mapping(oldpage));

    get_page(newpage);

    // TODO: 
    // 统计页面migrate次数


    return MIGRATEPAGE_SUCCESS;
}

void mapdrv_aop_putback_page(struct page *page)
{
    
}

module_init(mapdrv_mod_init);
module_exit(mapdrv_mod_exit);
