#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsh Kumar");
MODULE_DESCRIPTION("To traslate VA to PA using IOCTL");

#define DEVICE_NAME "ioctl_device_driver"
#define get_PA_from_VA  _IOR('p', 1, unsigned long)
#define wrt_data_to_PA _IOW('p', 2, unsigned long)

static int major;

static unsigned long translate_virtual_to_physical(unsigned long virtual_addr) {
    pgd_t *pgd_entry;
    pud_t *pud_entry;
    pmd_t *pmd_entry;
    pte_t *pte_entry;

    pgd_entry = pgd_offset(current->mm, virtual_addr);
    if (pgd_none(*pgd_entry) || pgd_bad(*pgd_entry)) {
        pr_err("Invalid PGD entry\n");
        return 0;
    }

    p4d_t *p4d_entry = p4d_offset(pgd_entry, virtual_addr);
    if (p4d_none(*p4d_entry) || p4d_bad(*p4d_entry)) {
        pr_err("Invalid P4D entry\n");
        return 0;
    }

    pud_entry = pud_offset(p4d_entry, virtual_addr);
    if (pud_none(*pud_entry) || pud_bad(*pud_entry)) {
        pr_err("Invalid PUD entry\n");
        return 0;
    }

    pmd_entry = pmd_offset(pud_entry, virtual_addr);
    if (pmd_none(*pmd_entry) || pmd_bad(*pmd_entry)) {
        pr_err("Invalid PMD entry\n");
        return 0;
    }

    pte_entry = pte_offset_map(pmd_entry, virtual_addr);
    if (pte_none(*pte_entry)) {
        pr_err("Invalid PTE entry\n"); 
        return 0;
    }

    unsigned long physical_addr = (pte_pfn(*pte_entry) << PAGE_SHIFT) | (virtual_addr & ~PAGE_MASK);
    return physical_addr;
}

static long spock_ioctl_func(struct file *file, unsigned int cmd, unsigned long arg) {
    unsigned long virtual_addr, physical_addr;
    void *virt_addr;
    unsigned char data;

    switch (cmd) {
    case get_PA_from_VA :
        if (copy_from_user(&virtual_addr, (unsigned long __user *)arg, sizeof(virtual_addr)))
            return -EFAULT;
        physical_addr = translate_virtual_to_physical(virtual_addr);
        if (copy_to_user((unsigned long __user *)arg, &physical_addr, sizeof(physical_addr)))
            return -EFAULT;
        break;

    case wrt_data_to_PA:
        if (copy_from_user(&physical_addr, (unsigned long __user *)arg, sizeof(physical_addr)))
            return -EFAULT;
        if (copy_from_user(&data, (unsigned char __user *)(arg + sizeof(physical_addr)), sizeof(data)))
            return -EFAULT;

        virt_addr = phys_to_virt(physical_addr);
        *(unsigned char *)virt_addr = data;
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = spock_ioctl_func,
};

static int __init ioctl_device_driver_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register device\n");
        return major;
    }
    return 0;
}

static void __exit ioctl_device_driver_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(ioctl_device_driver_init);
module_exit(ioctl_device_driver_exit);


