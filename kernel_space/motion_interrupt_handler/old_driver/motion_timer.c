/*
 * PiScale
 * Copyright (C) 2019 Gabe Teeger (gteeger)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

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
bool change;
static DECLARE_WAIT_QUEUE_HEAD(intrpt_waitqueue);

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
    mod_timer(&timer, safe_jiffies + DEFAULT_TIMEOUT_JIFFIES);
    change = TRUE;
    wake_up_interruptible(&intrpt_waitqueue);

    return IRQ_HANDLED;
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
    ssize_t retval = 0;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    wait_event_interruptible(intrpt_waitqueue, change == TRUE);

#if DEBUG
    printk(KERN_ALERT "dev status = %d\n", dev_status);
#endif
    retval = copy_to_user(buf, &dev_status, count);
    change = FALSE;
    return retval;
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
    .release = dev_release,
};


static int __init motion_timer_init(void)
{

    int status;
    int errno1, errno2;
    unsigned long IRQflags = IRQF_TRIGGER_RISING;
    change = FALSE;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    timer_setup(&timer, timeout, 0);	///////////////////////
    errno1 = gpio_request(GPIO_INT_PIN, "irq_gpio_pin");
    mdelay(50);
    errno2 = gpio_direction_input(GPIO_INT_PIN);
//cannot set debounce on rpi
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
