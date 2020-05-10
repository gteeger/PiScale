#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#include "motion_timer.h"

#define DEBUG 1

static struct timer_list timer;
static void timeout(struct timer_list *t);
static irqreturn_t irq_handler(unsigned int irq, void *dev_id,
			       struct pt_regs *regs);
int irq_number;
char dev_status;
volatile bool change;
volatile bool abort_sig;
static DECLARE_WAIT_QUEUE_HEAD(intrpt_waitqueue);
static int custom_timeout;

void bottom_half_tasklet(unsigned long x);
DECLARE_TASKLET(bottom_half, bottom_half_tasklet, 0);

static void timeout(struct timer_list *t)
{

#if DEBUG
    u64 safe_jiffies = get_jiffies_64();
    printk(KERN_ALERT "TIMEOUT!\n");
    printk(KERN_ALERT "JIFFIES: %lld\n", safe_jiffies);
#endif
    dev_status = OFF;
    change = TRUE;
    wake_up_interruptible(&intrpt_waitqueue);

}

static irqreturn_t irq_handler(unsigned int irq, void *dev_id,
			       struct pt_regs *regs)
{
    tasklet_schedule(&bottom_half);
    return IRQ_HANDLED;
}


void bottom_half_tasklet(unsigned long x)
{

    u64 safe_jiffies = get_jiffies_64();
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    dev_status = MEASURING;
    mod_timer(&timer, safe_jiffies + custom_timeout);
    change = TRUE;
    wake_up_interruptible(&intrpt_waitqueue);

}

static int dev_open(struct inode *inode, struct file *filp)
{

#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif

    return 0;
}


static ssize_t dev_read(struct file *filp, char __user * buf,
			size_t count, loff_t * f_pos)
{
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    wait_event_interruptible(intrpt_waitqueue, change == TRUE
			     || abort_sig == TRUE);

#if DEBUG
    printk(KERN_ALERT "dev status = %d\n", dev_status);
#endif

    if (copy_to_user(buf, &dev_status, count)) {
#if DEBUG
	printk(KERN_ALERT "Copy to user space failed");
#endif
	return -EFAULT;
    }
    change = FALSE;
    abort_sig = FALSE;
    return 0;
}

static long scale_ioctl(struct file *filp, unsigned int cmd,
			unsigned long arg)
{
    unsigned long t_out;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    printk(KERN_ALERT "cmd =  %d \n", cmd);
#endif
    switch (cmd) {
    case ABORT_SIG:
	printk(KERN_ALERT "Abort Signal received\n");
	abort_sig = TRUE;
	wake_up_interruptible(&intrpt_waitqueue);
	break;

    case TIMEOUT_CHANGE:
	/*
	 * We have to check whether arg is a valid input
	 * If the user enters a negative number, then
	 * t_out will be a large positve number.
	 */
	if (arg > TIMEOUT_LIMIT_MS) {
	    arg = TIMEOUT_LIMIT_MS;
	}
	if (arg < TIMEOUT_MIN_MS) {
	    arg = TIMEOUT_MIN_MS;
	}
	t_out = msecs_to_jiffies(arg);
	printk(KERN_ALERT "timeout changed to %lu\n", t_out);
	custom_timeout = t_out;
	break;

    default:

	break;

    }


    return 0;
}


static int dev_release(struct inode *inode, struct file *filp)
{
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    return 0;
}



static const struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .open = dev_open,
    .unlocked_ioctl = scale_ioctl,
    .release = dev_release,
};


static int __init motion_timer_init(void)
{

    int status;
    int errno1, errno2;
    unsigned long IRQflags = IRQF_TRIGGER_RISING;
    custom_timeout = DEFAULT_TIMEOUT_JIFFIES;
    change = FALSE;
    abort_sig = FALSE;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    timer_setup(&timer, timeout, 0);
    errno1 = gpio_request(GPIO_INT_PIN, "irq_gpio_pin");
    mdelay(50);
    errno2 = gpio_direction_input(GPIO_INT_PIN);
    if (errno1 < 0 || errno2 < 0) {
#if DEBUG
	printk(KERN_ALERT "cannot map gpio with errno1 %d\n", errno1);
	printk(KERN_ALERT "cannot map gpio with errno2 %d\n", errno2);
#endif
	return -1;
    }
    irq_number = gpio_to_irq(GPIO_INT_PIN);
    if (irq_number < 0) {
	printk(KERN_ALERT "Unable to map IRQ");
	return irq_number;
    }
#if DEBUG
    printk(KERN_ALERT "gpio mapped to irq %d", irq_number);
#endif
    status = request_irq(irq_number,
			 (irq_handler_t) irq_handler,
			 IRQflags, "mpu_int_handler", NULL);
    if (status < 0) {
	printk(KERN_ALERT "Unable to request IRQ");
	return status;
    }
    status = register_chrdev(MAJOR_NUM, "motion_timer_dev", &dev_fops);

    dev_status = OFF;

    return status;

}

static void __exit motion_timer_exit(void)
{
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    del_timer(&timer);
    free_irq(irq_number, NULL);
    gpio_free(GPIO_INT_PIN);
    unregister_chrdev(MAJOR_NUM, "motion_timer_dev");
}


module_init(motion_timer_init);
module_exit(motion_timer_exit);

MODULE_AUTHOR("Gabe Teeger");
MODULE_DESCRIPTION("start timer when motion interrupt is received");
MODULE_LICENSE("GPL");
