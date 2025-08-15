#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To show the memory stats using sysfs");

static struct kobject *mem_kobject;

static int pid = -1;
static char unit = 'B'; 

static struct task_struct *know_task_from_pid(int pid) {
    struct pid *pid_struct = find_get_pid(pid);
    if (!pid_struct)
        return NULL;
    return get_pid_task(pid_struct, PIDTYPE_PID);
}

static unsigned long chg_mem(unsigned long memory_size, char unit) {
    switch (unit) {
        case 'K': return memory_size / 1024;
        case 'M': return memory_size / (1024 * 1024);
        default: return memory_size; 
    }
}

static ssize_t show_pid(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", pid);
}

static ssize_t store_pid(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    int new_pid;
    if (kstrtoint(buf, 10, &new_pid) == 0) {
        pid = new_pid;
    }
    return count;
}

static ssize_t show_unit(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%c\n", unit);
}

static ssize_t store_unit(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    if (count == 2 && (buf[0] == 'B' || buf[0] == 'K' || buf[0] == 'M')) {
        unit = buf[0];
    }
    return count;
}

static ssize_t show_vm(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    struct task_struct *task;
    unsigned long virtmem = -1;

    if (pid != -1 && (task = know_task_from_pid(pid)) != NULL) {
        virtmem = task->mm ? task->mm->total_vm << PAGE_SHIFT : 0; 
        put_task_struct(task);
    }
    return sprintf(buf, "%lu\n", chg_mem(virtmem, unit));
}

static ssize_t show_mm(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    struct task_struct *task;
    unsigned long physmem = -1;

    if (pid != -1 && (task = know_task_from_pid(pid)) != NULL) {
        physmem = task->mm ? get_mm_rss(task->mm) << PAGE_SHIFT : 0; 
        put_task_struct(task);
    }
    return sprintf(buf, "%lu\n", chg_mem(physmem, unit));
}

static struct kobj_attribute pid_attr = __ATTR(pid, 0664, show_pid, store_pid);
static struct kobj_attribute unit_attr = __ATTR(unit, 0664, show_unit, store_unit);
static struct kobj_attribute vm_attr = __ATTR(virtmem, 0444, show_vm, NULL);
static struct kobj_attribute mm_attr = __ATTR(physmem, 0444, show_mm, NULL);

static struct attribute *attrs[] = {
    &pid_attr.attr,
    &unit_attr.attr,
    &vm_attr.attr,
    &mm_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int __init memory_stats_init(void) {
    int retval;

    mem_kobject = kobject_create_and_add("mem_stats", kernel_kobj);
    if (!mem_kobject)
        return -ENOMEM;

    retval = sysfs_create_group(mem_kobject, &attr_group);
    if (retval)
        kobject_put(mem_kobject);

    return retval;
}

static void __exit memory_stats_exit(void) {
    kobject_put(mem_kobject);
}

module_init(memory_stats_init);
module_exit(memory_stats_exit);


