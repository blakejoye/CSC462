#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>

#define PROC_NAME "seconds"

static struct proc_dir_entry *proc_file;
static unsigned long jiffies_at_start;

/* This is the function that will be called when the proc file is read */
static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
    int len = 0;
    char buffer[256];
    unsigned long jiffies_diff = jiffies - jiffies_at_start;
    unsigned long seconds = jiffies_diff / HZ;

    len += sprintf(buffer, "Seconds since load: %lu\n", seconds);

    // Copy buffer to user space
    if (*pos > 0 || count < len) return 0;
    if (copy_to_user(usr_buf, buffer, len)) return -EFAULT;
    *pos = len;
    return len;
}

/* Open function for the proc file */
static int proc_open(struct inode *inode, struct file *file) {
    return single_open(file, NULL, NULL);
}

/* Define the proc operations (proc_ops instead of file_operations) */
static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = proc_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

/* Initialize the module and create the /proc file */
static int __init proc_init(void) {
    jiffies_at_start = jiffies;  // Record the current jiffies value at load
    proc_file = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_file) {
        return -ENOMEM;
    }
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

/* Clean up the module and remove the /proc file */
static void __exit proc_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blake");
