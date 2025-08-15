#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To list all processes in a running or runnable state.");

static int running_proc_init(void)
{
    struct task_struct *process;

    printk(KERN_INFO "[LKM1] Runnable Processes:\n");
    printk(KERN_INFO "[LKM1] PID       PROC\n");
    printk(KERN_INFO "[LKM1] ------------------\n");

    for_each_process(process) {
        if (task_is_running(process)) {
            printk(KERN_INFO "[LKM1] %d %s\n", process->pid, process->comm);
        }
    }
  
    return 0;
}

static void running_proc_exit(void)
{
    printk(KERN_INFO "[LKM1] Module LKM1 Unloaded\n");
}

module_init(running_proc_init);
module_exit(running_proc_exit);
 