#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/spinlock.h>
#include <linux/signal.h>
#include <linux/delay.h>  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("Demonstrate the control station and soldiers relationship");

#define DEVICE_NAME "chardev"
#define PARENT_CHG _IOW('a', 1, pid_t)
#define KILL_CHILDREN _IOW('a', 2, pid_t)

static int major_number;
static struct class *chardev_class = NULL;
static struct device *chardev_device = NULL;

static int chardev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "chardev: Device opened\n");
    return 0;
}

static int chardev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "chardev: Device closed\n");
    return 0;
}

static long chardev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    pid_t target_pid;
    struct task_struct *target_task;
    struct task_struct *child, *tmp;
    struct pid *pid_struct;

    switch (cmd) {
        case PARENT_CHG:
            if (copy_from_user(&target_pid, (pid_t *)arg, sizeof(pid_t))) {
                printk(KERN_ERR "chardev: Failed to copy PID from user\n");
                return -EFAULT;
            }

            rcu_read_lock();
            pid_struct = find_vpid(target_pid);
            if (!pid_struct) {
                rcu_read_unlock();
                printk(KERN_ALERT "chardev: Invalid PID %d\n", target_pid);
                return -ESRCH;
            }

            target_task = pid_task(pid_struct, PIDTYPE_PID);
            if (!target_task) {
                rcu_read_unlock();
                printk(KERN_ALERT "chardev: Task structure not found for PID %d\n", target_pid);
                return -ESRCH;
            }

            get_task_struct(target_task);
            rcu_read_unlock();

            task_lock(current->real_parent);
            list_del_rcu(&current->sibling);
            task_unlock(current->real_parent);

            task_lock(target_task);
            current->real_parent = target_task;
            current->parent = target_task;
            list_add_tail_rcu(&current->sibling, &target_task->children);
            task_unlock(target_task);

            synchronize_rcu(); 

            put_task_struct(target_task);

            return 0;

        case KILL_CHILDREN:
            pid_t control_station_pid;

            if (copy_from_user(&control_station_pid, (pid_t *)arg, sizeof(pid_t))) {
                printk(KERN_ERR "chardev: Failed to copy control station PID from user\n");
                return -EFAULT;
            }

            rcu_read_lock();
            list_for_each_entry_safe(child, tmp, &current->children, sibling) {
                send_sig(SIGKILL, child, 1);  
                
            }
            rcu_read_unlock();

            printk(KERN_INFO "chardev: All soldiers terminated. Now terminating control station.\n");

            struct task_struct *control_station_task = pid_task(find_vpid(control_station_pid), PIDTYPE_PID);
            if (control_station_task) {
                msleep(1);
                send_sig(SIGTERM, control_station_task, 1);
            }
            return 0;
 
        default:
            printk(KERN_ERR "chardev: Invalid IOCTL command\n");
            return -EINVAL;
    }
}

static struct file_operations fops = {
    .open = chardev_open,
    .release = chardev_release,
    .unlocked_ioctl = chardev_ioctl,
};

static int __init chardev_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "chardev: Failed to register a major number\n");
        return major_number;
    }

    chardev_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chardev_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "chardev: Failed to create class\n");
        return PTR_ERR(chardev_class);
    }

    chardev_device = device_create(chardev_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(chardev_device)) {
        class_destroy(chardev_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "chardev: Failed to create device\n");
        return PTR_ERR(chardev_device);
    }

    return 0;
}

static void __exit chardev_exit(void) {
    device_destroy(chardev_class, MKDEV(major_number, 0));
    class_destroy(chardev_class);
    unregister_chrdev(major_number, DEVICE_NAME);
}

module_init(chardev_init);
module_exit(chardev_exit);


