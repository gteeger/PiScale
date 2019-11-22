# Motion Interrupt on Raspberry Pi 3 A+ with an MPU6050 IMU 

*Before building, specify in the driver which i2c adapter you want to connect to (ex: i2c-1)*


This is a very basic driver to enable motion interrupts on an MPU 6050.

After you insert this driver into the kernel, you're mpu6050's INT pin will go high then low when motion is detected.

There are no loops, the driver simply configures the mpu6050 over I2C.

You can use any linux board but I used a Raspberry Pi with the mpu connected to i2c-1 (verified with i2c-tools). 

Commands:
```
$ make
$ sudo insmod mpu6050.ko
...(mpu is detecting motion)..
$ sudo rmmod mpu6050.ko
...(mpu goes to sleep)...
```

Output from oscilloscope hooked up to interrupt pin of MPU6050 when motion is detected:
![](https://i.imgur.com/LCopkOC.png)
