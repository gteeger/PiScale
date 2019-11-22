#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include "mpu6050.h"

//user can give their own values here
#define I2C_ADAPTER 1
#define MOTION_INT_THRESHOLD 0x01
#define MOTION_INT_DURATION 0x01

#define DEBUG 0

struct i2c_client *mpu_client;


static struct i2c_board_info mpu_info __initdata = {
    I2C_BOARD_INFO("mpu", 0x68)
};

static int __init mpu_init(void)
{
    struct i2c_adapter *i2c_adap;
    __u8 read_byte;
    __u8 write_byte;
#if DEBUG
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
#endif
    i2c_adap = i2c_get_adapter(I2C_ADAPTER);
    mpu_client = i2c_new_device(i2c_adap, &mpu_info);
    if (mpu_client == NULL) {
	printk(KERN_ALERT "cannot create new i2c device\n");
	return -1;
    }
#if DEBUG
    read_byte = i2c_smbus_read_byte_data(mpu_client, WHO_AM_I);
    printk(KERN_ALERT "WHOAMI: %02X \n", read_byte);
#endif
/****************************************************************/

    //first do a reset, PWR_MGMT_1 gets set to 0x00 after reset 0x80 = 0x10000000
    read_byte = i2c_smbus_read_byte_data(mpu_client, PWR_MGMT_1);
    write_byte = read_byte | 0x80;
    if (i2c_smbus_write_byte_data(mpu_client, PWR_MGMT_1, write_byte) < 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }
    mdelay(1000);

    /*
     * PWR_MGMT_1 register value:
     * 
     * bit7: 0 -already reset
     * bit6: 1 - dont wake up from sleep yet
     * bit5: 0 - cycling
     * bit4: X - reserved
     * bit3: 1 - disable temperature sensor
     * bit2: 0 \
     * bit1: 0  => Clock is PLL with x-axis of gyroscope reference 
     * bit0: 1 /
     * 
     * 001X1001 = 0x49
     */
#if DEBUG
    read_byte = i2c_smbus_read_byte_data(mpu_client, PWR_MGMT_1);
    printk(KERN_ALERT "PWR_MGMT_1 output after reset: %02X \n", read_byte);
#endif

    write_byte = read_byte | 0x49;
    if (i2c_smbus_write_byte_data(mpu_client, PWR_MGMT_1, write_byte) < 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }
#if DEBUG
    read_byte = i2c_smbus_read_byte_data(mpu_client, PWR_MGMT_1);
    printk(KERN_ALERT "PWR_MGMT_1 output: %02X \n", read_byte);
#endif

/****************************************************************/

//wake from sleep
    read_byte = i2c_smbus_read_byte_data(mpu_client, PWR_MGMT_1);
    write_byte = read_byte & ~0x40;
    if (i2c_smbus_write_byte_data(mpu_client, PWR_MGMT_1, write_byte) < 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }

    mdelay(1000);
#if DEBUG
    read_byte = i2c_smbus_read_byte_data(mpu_client, PWR_MGMT_1);
    printk(KERN_ALERT "byte output final: %02X \n", read_byte);
#endif

/****************************************************************/
//setDHPFMode not needed

//set accel_on_delay to 3
    read_byte = i2c_smbus_read_byte_data(mpu_client, MOT_DETECT_CTRL);
    write_byte = read_byte | 0x30;
    if (i2c_smbus_write_byte_data(mpu_client, MOT_DETECT_CTRL, write_byte)
	< 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }
    //mdelay(2000);
#if DEBUG
    read_byte = i2c_smbus_read_byte_data(mpu_client, MOT_DETECT_CTRL);
    printk(KERN_ALERT "MOT_DETECT_CTRL output: %02X \n", read_byte);
#endif
/****************************************************************/
//enable motion interrupts
    read_byte = i2c_smbus_read_byte_data(mpu_client, INT_STATUS);
    if (i2c_smbus_write_byte_data(mpu_client, INT_ENABLE, 0x40) < 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }
#if DEBUG
    read_byte = i2c_smbus_read_byte_data(mpu_client, INT_ENABLE);
    printk(KERN_ALERT "INT_ENABLE output: %02X \n", read_byte);
#endif
/****************************************************************/
//set motion detection threshold
    if (i2c_smbus_write_byte_data
	(mpu_client, MOT_THR, MOTION_INT_THRESHOLD) < 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }
    if (i2c_smbus_write_byte_data(mpu_client, MOT_DUR, MOTION_INT_DURATION)
	< 0) {
	printk(KERN_ALERT "Cannot write byte to device");
	return -1;
    }

    return 0;
}

static void __exit mpu_exit(void)
{
    //turn off mpu
    if (i2c_smbus_write_byte_data(mpu_client, PWR_MGMT_1, 0x40) < 0) {
	printk(KERN_ALERT "Cannot write byte to device");
    }
    //unregister device
    i2c_unregister_device(mpu_client);

}

module_init(mpu_init);
module_exit(mpu_exit);

MODULE_AUTHOR("Gabe Teeger");
MODULE_DESCRIPTION("Enable motion interrupts on i2c");

MODULE_LICENSE("GPL");
