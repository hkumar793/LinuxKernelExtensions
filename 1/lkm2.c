#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To print the child process PID with their state for a specified PID.");

static int target_pid = 0;
module_param(target_pid, int, 0644);

static const char *get_task_state(long state) {
    switch (state) {
    case TASK_RUNNING:
        return "TASK_RUNNING";
    case TASK_INTERRUPTIBLE:
        return "TASK_INTERRUPTIBLE";
    case TASK_UNINTERRUPTIBLE:
        return "TASK_UNINTERRUPTIBLE";
    case __TASK_STOPPED:
        return "TASK_STOPPED";
    case __TASK_TRACED:
        return "TASK_TRACED";
    default:
        return "UNKNOWN_STATE";
    }
}

static int task_child_init(void)
{
    struct task_struct *parent_task;

    for_each_process(parent_task)
    {
        if (parent_task->pid == target_pid)
        {
            struct task_struct *child_task;

            list_for_each_entry(child_task, &parent_task->children, sibling)
            {
                printk(KERN_INFO "[LKM2] Child Task PID: %d, State: %s\n",
                       child_task->pid, get_task_state(child_task->__state));
            }
            break; 
        }
    }

    return 0;
}

static void task_child_exit(void)
{
    printk(KERN_INFO "[LKM2] Module LKM2 Unloaded\n");
}

module_init(task_child_init);
module_exit(task_child_exit);
