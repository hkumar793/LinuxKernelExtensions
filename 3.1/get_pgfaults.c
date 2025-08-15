#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/vmstat.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To check page faults");

unsigned long total_page_fault(void)
{
    unsigned long total = 0;
    int cpu;

    for_each_possible_cpu(cpu) {
        total += per_cpu(vm_event_states.event[PGFAULT], cpu);
    }

    return total;
}

static int show_page_fault(struct seq_file *m, void *v)
{
    unsigned long pgfaults = total_page_fault();
    seq_printf(m, "Page faults: %lu\n", pgfaults);
    return 0;
}

static int open(struct inode *inode, struct file *file)
{
    return single_open(file, show_page_fault, NULL);
}

static const struct proc_ops proc_options = {
    .proc_open = open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init get_page_faults_init(void)
{
    proc_create("get_pgfaults", 0, NULL, &proc_options);
    pr_info("/proc/get_pgfaults created\n");
    return 0;
}

static void __exit get_page_faults_exit(void)
{
    remove_proc_entry("get_pgfaults", NULL);
    pr_info("/proc/get_pgfaults removed\n");
}

module_init(get_page_faults_init);
module_exit(get_page_faults_exit);


