#!/bin/bash

sudo mknod -m 666 /dev/motion_int c 117 0
cd kernel_space/motion_interrupt_handler
make
sudo insmod motion_timer.ko
make clean
cd ../mpu6050_motion_interrupt
make
sudo insmod mpu6050.ko
make clean
cd ../../user_space
gcc pi_scale.c -o pi_scale -lpthread
./pi_scale
