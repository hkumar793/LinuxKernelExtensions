#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/pid.h>
#include <linux/highmem.h>
#include <linux/pgtable.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To print the physical address of mapped virtual address else reason for unmapped.");

static int target_pid = 0;
static unsigned long virtual_addr = 0;

module_param(target_pid, int, 0644);
module_param(virtual_addr, ulong, 0644);

static int addr_translation_init(void)
{
    struct task_struct *task;
    struct pid *pid_obj;
    struct mm_struct *mm_info;
    struct vm_area_struct *vma_info;
    pgd_t *pgd_entry;
    p4d_t *p4d_entry;
    pud_t *pud_entry;
    pmd_t *pmd_entry;
    pte_t *pte_entry;
    unsigned long phys_addr;
    int result;
    char temp_char;

    if (target_pid < 0 || virtual_addr == 0) {
        printk(KERN_ERR "[LKM3] VA not mapped: Invalid parameters. PID: %d, VA: %lx\n", target_pid, virtual_addr);
        return 0;
    }

    pid_obj = find_get_pid(target_pid);
    if (!pid_obj) {
        printk(KERN_ERR "[LKM3] VA not mapped: Failed to find PID %d\n", target_pid);
        return 0;
    }

    task = get_pid_task(pid_obj, PIDTYPE_PID);
    if (!task) {
        printk(KERN_ERR "[LKM3] VA not mapped: Failed to get task_struct for PID %d\n", target_pid);
        return 0;
    }

    mm_info = get_task_mm(task);
    if (!mm_info) {
        printk(KERN_ERR "[LKM3] VA not mapped: Failed to get mm_struct for PID %d\n", target_pid);
        put_task_struct(task);
        return 0;
    }

    down_read(&mm_info->mmap_lock); 

    vma_info = find_vma(mm_info, virtual_addr);
    if (!vma_info || virtual_addr < vma_info->vm_start || virtual_addr >= vma_info->vm_end) {
        printk(KERN_ERR "[LKM3] VA not mapped: VA %lx is not valid in PID %d's address space\n", virtual_addr, target_pid);
        up_read(&mm_info->mmap_lock); 
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    result = access_process_vm(task, virtual_addr, &temp_char, sizeof(temp_char), 0);
    if (result < 0) {
        printk(KERN_ERR "[LKM3] VA not mapped: VA %lx is not accessible in PID %d's address space\n", virtual_addr, target_pid);
        up_read(&mm_info->mmap_lock); 
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    pgd_entry = pgd_offset(mm_info, virtual_addr);
    if (pgd_none(*pgd_entry) || pgd_bad(*pgd_entry)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Invalid PGD entry\n");
        up_read(&mm_info->mmap_lock);
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    p4d_entry = p4d_offset(pgd_entry, virtual_addr);
    if (p4d_none(*p4d_entry) || p4d_bad(*p4d_entry)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Invalid P4D entry\n");
        up_read(&mm_info->mmap_lock);
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    pud_entry = pud_offset(p4d_entry, virtual_addr);
    if (pud_none(*pud_entry) || pud_bad(*pud_entry)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Invalid PUD entry\n");
        up_read(&mm_info->mmap_lock);
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    pmd_entry = pmd_offset(pud_entry, virtual_addr);
    if (pmd_none(*pmd_entry) || pmd_bad(*pmd_entry)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Invalid PMD entry\n");
        up_read(&mm_info->mmap_lock);
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    pte_entry = pte_offset_kernel(pmd_entry, virtual_addr);
    if (!pte_entry || pte_none(*pte_entry)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Invalid PTE entry\n");
        up_read(&mm_info->mmap_lock);
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    if (!pte_present(*pte_entry)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Page is not present\n");
        up_read(&mm_info->mmap_lock);
        mmput(mm_info);
        put_task_struct(task);
        return 0;
    }

    phys_addr = (pte_pfn(*pte_entry) << PAGE_SHIFT) | (virtual_addr & ~PAGE_MASK);
    printk(KERN_INFO "[LKM3] Virtual address: 0x%lx / %lu\n", virtual_addr, virtual_addr);
    printk(KERN_INFO "[LKM3] Physical address: 0x%lx / %lu\n", phys_addr, phys_addr);

    up_read(&mm_info->mmap_lock); 
    mmput(mm_info);
    put_task_struct(task);

    return 0;  
}

static void addr_translation_exit(void)
{
    printk(KERN_INFO "[LKM3]: Module LKM3 Unloaded\n");
}

module_init(addr_translation_init);
module_exit(addr_translation_exit);
