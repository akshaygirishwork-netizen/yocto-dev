#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h> // For irqreturn_t, IRQ_HANDLED
#include <linux/gpio.h>      // For gpio_request, gpio_direction_input, gpio_to_irq
#include <linux/irq.h>       // For IRQF_TRIGGER_* flags
#include <linux/workqueue.h>
#include <linux/timer.h>

#define MY_IOCTL_MAGIC 'A'

#define IOCTL_CLEAR_BUFFER _IO(MY_IOCTL_MAGIC, 1)
#define IOCTL_GET_SIZE _IOR(MY_IOCTL_MAGIC, 2, int)
#define IOCTL_SET_VALUE _IOW(MY_IOCTL_MAGIC, 3, int)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akshay");
MODULE_DESCRIPTION("Simple Hello World Kernel Module");

int value = 10;
static DEFINE_MUTEX(my_mutex);

dev_t dev_number;    // major + minor number stored here
struct cdev my_cdev; // charector device structure

char kbuf[1024];
int data_size = 0;

module_param(value, int, 0644);
MODULE_PARM_DESC(value, "An integer value");

static struct kobject *my_kobj;

static struct class *my_class;
static struct device *my_device;

static DECLARE_WAIT_QUEUE_HEAD(wq); // poll()
static int data_available = 0;

static int my_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "mychardev buffer size = %d\n", data_size);
    seq_printf(m, "mychardev value = %d\n", value);
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_proc_show, NULL);
}

static const struct proc_ops my_proc_ops = {
    .proc_open = my_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// Sysfs show function (read)
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", value);
}

// Sysfs store function (write)
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d", &value);
    printk(KERN_INFO "mychardev: sysfs value updated to %d\n", value);
    return count;
}

// Define the attribute (read/write)
static struct kobj_attribute value_attribute =
    __ATTR(value, 0664, value_show, value_store);

// FILE OPERATIONS
// =============================
int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mychardev: device opened\n");
    return 0;
}

int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mychardev: device closed\n");
    return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    ssize_t ret = 0;

    mutex_lock(&my_mutex);

    if (*off >= data_size)
    {
        data_available = 0; // No more data
        ret = 0;
        goto out;
    }

    if (len > data_size - *off)
        len = data_size - *off;

    if (copy_to_user(buf, kbuf + *off, len))
    {
        ret = -EFAULT;
        goto out;
    }

    *off += len;

    if (*off >= data_size)
        data_available = 0; // All data read

    ret = len;

out:
    mutex_unlock(&my_mutex);
    return ret;
}

ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    ssize_t ret = 0;

    mutex_lock(&my_mutex);

    if (len > 1024)
        len = 1024;

    if (copy_from_user(kbuf, buf, len))
    {
        ret = -EFAULT;
        goto out;
    }

    data_size = len;
    *off = len;

    data_available = 1;         // Data ready
    wake_up_interruptible(&wq); // Wake any waiting process

    ret = len;

out:
    mutex_unlock(&my_mutex);
    return ret;
}

__poll_t my_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;

    // Add process to wait queue
    poll_wait(file, &wq, wait);

    // Check if data is available
    if (data_available)
        mask |= POLLIN | POLLRDNORM; // Data available for read

    // Always writable
    mask |= POLLOUT | POLLWRNORM;

    return mask;
}

long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int temp;

    switch (cmd)
    {
    case IOCTL_CLEAR_BUFFER:
        mutex_lock(&my_mutex);
        data_size = 0;
        memset(kbuf, 0, sizeof(kbuf));
        mutex_unlock(&my_mutex);
        printk(KERN_INFO "mychardev: buffer cleared\n");
        break;

    case IOCTL_GET_SIZE:
        if (copy_to_user((int __user *)arg, &data_size, sizeof(int)))
            return -EFAULT;
        printk(KERN_INFO "mychardev: returned data_size = %d\n", data_size);
        break;

    case IOCTL_SET_VALUE:
        if (copy_from_user(&temp, (int __user *)arg, sizeof(int)))
            return -EFAULT;
        value = temp;
        printk(KERN_INFO "mychardev: value updated to %d\n", value);
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

// Map file operations to our functions
struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .poll = my_poll,
    .unlocked_ioctl = my_ioctl,
};

static struct workqueue_struct *my_wq;

static void my_work_handler(struct work_struct *work)
{
    printk(KERN_INFO ">>> Workqueue running (bottom half)\n");
}
static DECLARE_WORK(my_work, my_work_handler);

static int gpio = 17;
static int irq;

static struct timer_list my_timer;
static int timer_interval = 10000;

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO ">> GPIO interrupt triggered\n");
    // Queue bottom half work
    queue_work(my_wq, &my_work);

     // Start timer only when first interrupt occurs
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_interval));

    return IRQ_HANDLED;
}

static void my_timer_handler(struct timer_list *t)
{
    printk(KERN_INFO ">>> Timer expired\n");
}

static int hello_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, "mychardev");

    if (ret < 0)
    {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }
    printk(KERN_INFO "allocated major number =%d,minor number=%d\n", MAJOR(dev_number), MINOR(dev_number));

    // 2. Initialize cdev
    cdev_init(&my_cdev, &fops);

    // 3. Add cdev to kernel
    ret = cdev_add(&my_cdev, dev_number, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to add cdev\n");
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    printk(KERN_INFO "cdev added successfully\n");

    // Create device class
    my_class = class_create(THIS_MODULE, "mychardev_class");
    if (IS_ERR(my_class))
    {
        printk(KERN_ERR "Failed to create class\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(my_class);
    }

    // Create device node /dev/mychardev
    my_device = device_create(my_class, NULL, dev_number, NULL, "mychardev");
    if (IS_ERR(my_device))
    {
        printk(KERN_ERR "Failed to create device\n");
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "/dev/mychardev created successfully\n");

    // Create /sys/kernel/mychardev directory
    my_kobj = kobject_create_and_add("mychardev", kernel_kobj);
    if (!my_kobj)
    {
        printk(KERN_ERR "mychardev: failed to create kobject\n");
        return -ENOMEM;
    }

    // Create value attribute
    if (sysfs_create_file(my_kobj, &value_attribute.attr))
    {
        printk(KERN_ERR "mychardev: failed to create sysfs file\n");
        kobject_put(my_kobj);
        return -ENOMEM;
    }
    proc_create("mychardev_info", 0444, NULL, &my_proc_ops);

    if (gpio_request(gpio, "my_gpio"))
    {
        printk(KERN_ERR "Failed to request GPIO %d\n", gpio);
    }
    gpio_direction_input(gpio);

    irq = gpio_to_irq(gpio);

    request_irq(irq, my_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq", NULL);
    printk(KERN_INFO "IRQ Number=%d\n", irq);

    my_wq = create_singlethread_workqueue("my_wq");

    timer_setup(&my_timer, my_timer_handler, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_interval));

    return 0;
}

static void hello_exit(void)
{
    remove_proc_entry("mychardev_info", NULL);
    kobject_put(my_kobj);
    device_destroy(my_class, dev_number);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_number, 1);

    free_irq(irq, NULL);
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
    del_timer_sync(&my_timer);

    printk(KERN_INFO "mychardev: module removed\n");
}

module_init(hello_init);
module_exit(hello_exit);