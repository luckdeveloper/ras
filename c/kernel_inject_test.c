#include <linux/init.h>  
#include <linux/module.h>  
#include <linux/gfp.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL"); 

static struct timer_list my_timer;
struct page *page;
void *ptr_page;
phys_addr_t page_phys_addr;
void my_timer_callback(struct timer_list *t) {
    if(page)
    {
	    page_phys_addr = virt_to_phys(ptr_page);
printk("kmalloc 分配的虚拟地址: %p, 虚拟地址对应的物理地址：%pa\n", page, &page_phys_addr);
//	    memcpy(page,"page in memory alloc",20);
	    printk("DATA IN PAGE:[%s]\n",page);
    }

    // 重新设置定时器，以便在 10 秒后再次触发回调函数
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(10000));
}



static int hello_init(void)  
{
	page=alloc_page(GFP_KERNEL);
	if(page)
{
	memcpy(page,"page in memory alloc",20);
	ptr_page=page_address(page); //虚拟地址
	page_phys_addr = virt_to_phys(ptr_page);
	printk("kmalloc 分配的虚拟地址: %p, 虚拟地址对应的物理地址：%pa\n", page, &page_phys_addr);
}

// 初始化定时器并设置回调函数
    timer_setup(&my_timer, my_timer_callback, 0);
    my_timer.expires = jiffies + msecs_to_jiffies(10000);
    add_timer(&my_timer);
    // 将数据拷贝到 kmalloc 分配的内存块中
    //    memcpy(ptr, data, len);
//   printk(KERN_INFO "Data in ptr: %s\n", (char*)ptr);
    return 0;  
}  
static void hello_exit(void)  
{
       del_timer_sync(&my_timer);  // 删除定时器
  //  if (page) {
//put_page(page);  // 释放内存
    //}
	
  printk(KERN_ALERT "Goodbye, cruel world\n");  
}  
  
module_init(hello_init);  
module_exit(hello_exit);
