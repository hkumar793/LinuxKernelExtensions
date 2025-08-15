#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/mm_types.h>
#include <linux/rwsem.h>
#include <linux/pgtable.h>
#include <linux/rcupdate.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To analyze the number of huge pages when THP is enabled and disabled");

static int process_id = 0;
module_param(process_id, int, 0444);

static int __init thp_analysis_init(void) {
    struct task_struct *target_task;
    struct mm_struct *memory_info;
    struct vma_iterator area_iterator;
    struct vm_area_struct *memory_area;
    unsigned long total_thp_size = 0;
    int thp_page_count = 0;

    if (process_id <= 0) {
        pr_err("[LKM5] Invalid process ID specified\n");
        return -EINVAL;
    }

    target_task = get_pid_task(find_get_pid(process_id), PIDTYPE_PID);
    if (!target_task) {
        pr_err("[LKM5] Could not find task for process ID %d\n", process_id);
        return -ESRCH;
    }

    memory_info = target_task->mm;
    if (!memory_info) {
        pr_err("[LKM5] Could not access memory descriptor for process ID %d\n", process_id);
        put_task_struct(target_task);
        return -EINVAL;
    }

    down_read(&memory_info->mmap_lock);

    vma_iter_init(&area_iterator, memory_info, 0);

    for (memory_area = vma_next(&area_iterator); memory_area; memory_area = vma_next(&area_iterator)) {
        unsigned long current_addr;

        if (!vma_is_anonymous(memory_area))
            continue;

        for (current_addr = memory_area->vm_start; current_addr < memory_area->vm_end; current_addr += HPAGE_PMD_SIZE) {
            pgd_t *pgd_entry;
            p4d_t *p4d_entry;
            pud_t *pud_entry;
            pmd_t *pmd_entry;

            pgd_entry = pgd_offset(memory_info, current_addr);
            if (!pgd_present(*pgd_entry))
                continue;

            p4d_entry = p4d_offset(pgd_entry, current_addr);
            if (!p4d_present(*p4d_entry))
                continue;

            pud_entry = pud_offset(p4d_entry, current_addr);
            if (!pud_present(*pud_entry))
                continue;

            pmd_entry = pmd_offset(pud_entry, current_addr);
            if (!pmd_present(*pmd_entry))
                continue;

            if (pmd_trans_huge(*pmd_entry)) {
                total_thp_size += HPAGE_PMD_SIZE;
                thp_page_count++;
                continue;
            }
        }
    }

    up_read(&memory_info->mmap_lock);

    pr_info("[LKM5] THP Size: %lu KiB, THP count: %d\n", total_thp_size >> 10, thp_page_count);

    put_task_struct(target_task);
    return 0;
}

static void __exit thp_analysis_exit(void) {
    pr_info("[LKM5] Module LKM5 Unloaded\n");
}

module_init(thp_analysis_init);
module_exit(thp_analysis_exit);
