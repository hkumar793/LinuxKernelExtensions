#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/mm.h>
#include <linux/atomic.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To print Virtual Memory and Physical Memory size.");

static int target_pid = 0;
module_param(target_pid, int, 0);

static int mem_info_init(void) {
    struct task_struct *task;
    struct mm_struct *memory;
    unsigned long virtual_mem, physical_mem;

    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (task == NULL) {
        printk(KERN_ERR "[LKM4] No such task with PID: %d\n", target_pid);
        return -EINVAL;
    }

    memory = task->mm;
    if (memory == NULL) {
        printk(KERN_ERR "[LKM4] No memory info available for PID: %d\n", target_pid);
        return -EINVAL;
    }

    virtual_mem = memory->total_vm << PAGE_SHIFT;
    physical_mem = get_mm_rss(memory) * PAGE_SIZE;

    printk(KERN_INFO "[LKM4] Virtual Memory Size: %lu KiB\n", virtual_mem / 1024);
    printk(KERN_INFO "[LKM4] Physical Memory Size: %lu KiB\n", physical_mem / 1024);

    return 0;
}

static void mem_info_exit(void) {
    printk(KERN_INFO "[LKM4] Module Unloaded\n");
}

module_init(mem_info_init);
module_exit(mem_info_exit);
