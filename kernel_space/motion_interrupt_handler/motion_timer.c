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
static int irq_number;
static char dev_status;
volatile bool change;
volatile bool abort_sig;
static DECLARE_WAIT_QUEUE_HEAD(intrpt_waitqueue);
int custom_timeout;

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
    u64 safe_jiffies = get_jiffies_64();
#if DEBUG
    printk(KERN_ALERT "interrupt triggered\n");
#endif
    dev_status = MEASURING;
    mod_timer(&timer, safe_jiffies + custom_timeout);
    change = TRUE;
    wake_up_interruptible(&intrpt_waitqueue);

    return IRQ_HANDLED;
}


static int dev_open(struct inode *inode, struct file *filp)
{

#ifdef DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif

    return 0;
}


static ssize_t dev_read(struct file *filp, char __user * buf,
                        size_t count, loff_t * f_pos)
{
    ssize_t retval = 0;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    wait_event_interruptible(intrpt_waitqueue, change == TRUE||abort_sig == TRUE);

#if DEBUG
    printk(KERN_ALERT "dev status = %d\n", dev_status);
#endif
    retval = copy_to_user(buf, &dev_status, count);
    change = FALSE;
    abort_sig = FALSE;
    return retval;
}

static long scale_ioctl(struct file *filp, unsigned int cmd,
                        unsigned long arg)
{
    bool mux_sel1 = 0, mux_sel2 = 0;
    unsigned long t_out;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    printk(KERN_ALERT "cmd =  %d \n", cmd);
#endif
    switch(cmd)
    {

    case MUX_SEL:

        switch(arg)
        {

        case 0:
            mux_sel1 = LO;
            mux_sel2 = LO;
            break;
        case 1:
            mux_sel1 = HI;
            mux_sel2 = LO;
            break;
        case 2:
            mux_sel1 = LO;
            mux_sel2 = HI;
            break;
        case 3:
            mux_sel1 = HI;
            mux_sel2 = HI;
        default:
            mux_sel1 = LO;
            mux_sel2 = LO;
            break;

        }
        //val = gpio_get_value(MUX_SEL_PIN1);
#if DEBUG
        printk(KERN_ALERT "setting mux sel1 to %d \n", mux_sel1);
        printk(KERN_ALERT "setting mux sel2 to %d \n", mux_sel2);
#endif
        gpio_set_value(MUX_SEL_PIN1, mux_sel1);
        if(arg<2)
        {
            //if arg is always <2, there is only 1 sensor
            gpio_set_value(MUX_SEL_PIN2, mux_sel2);
        }
        break;

    case ABORT_SIG:
        printk(KERN_ALERT "Abort Signal received\n");
        abort_sig = TRUE;
        wake_up_interruptible(&intrpt_waitqueue);
        break;

    case TIMEOUT_CHANGE:
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



static const struct file_operations dev_fops =
{
    .owner = THIS_MODULE,
    .read = dev_read,
    .open = dev_open,
    .unlocked_ioctl = scale_ioctl,
    .release = dev_release,
};


static int __init motion_timer_init(void)
{

    int status;
    int errno1, errno2, errno3, errno4, errno5, errno6;
    unsigned long IRQflags = IRQF_TRIGGER_RISING;
    custom_timeout = DEFAULT_TIMEOUT_JIFFIES;
    change = FALSE;
    abort_sig = FALSE;
#ifdef DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    timer_setup(&timer, timeout, 0);	///////////////////////
    errno1 = gpio_request(GPIO_INT_PIN, "irq_gpio_pin");
    mdelay(50);
    errno2 = gpio_direction_input(GPIO_INT_PIN);

    errno3 = gpio_request(MUX_SEL_PIN1, "MUX_SEL1");
    errno4 = gpio_direction_output(MUX_SEL_PIN1, LO);
    errno5 = gpio_request(MUX_SEL_PIN2, "MUX_SEL2");
    errno6 = gpio_direction_output(MUX_SEL_PIN2, LO);
//cannot set debounce on rpi
    if (errno1 < 0 || errno2 < 0 || errno3 < 0 || errno4 < 0|| errno5 < 0 || errno6 < 0)
    {
#ifdef DEBUG
        printk(KERN_ALERT "cannot map gpio with errno1 %d\n", errno1);
        printk(KERN_ALERT "cannot map gpio with errno2 %d\n", errno2);
        printk(KERN_ALERT "cannot MUX Select %d\n", errno3);
#endif
        return -1;
    }
    irq_number = gpio_to_irq(GPIO_INT_PIN);
    if (irq_number < 0)
    {
        printk(KERN_ALERT "Unable to map IRQ");
        return irq_number;
    }
#ifdef DEBUG
    printk(KERN_ALERT "gpio mapped to irq %d", irq_number);
#endif
    status = request_irq(irq_number,
                         (irq_handler_t) irq_handler,
                         IRQflags, "mpu_int_handler", NULL);
    if (status < 0)
    {
        printk(KERN_ALERT "Unable to request IRQ");
        return status;
    }
    status = register_chrdev(MAJOR_NUM, "motion_timer_dev", &dev_fops);

    dev_status = OFF;

    return status;

}

static void __exit motion_timer_exit(void)
{
#ifdef DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    del_timer(&timer);
    free_irq(irq_number, NULL);
    gpio_free(GPIO_INT_PIN);
    gpio_free(MUX_SEL_PIN1);
    gpio_free(MUX_SEL_PIN2);
    unregister_chrdev(MAJOR_NUM, "motion_timer_dev");
}


module_init(motion_timer_init);
module_exit(motion_timer_exit);

MODULE_AUTHOR("Gabe Teeger");
MODULE_DESCRIPTION("start timer when motion interrupt is received");
MODULE_LICENSE("GPL");
