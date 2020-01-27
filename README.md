# PiScale

### Updates:
1) User-space program is now written in C++ with an interface to a Scale class.
2) Pressing ctrl+c while program is running issues a signal to abort the program and notifies kernel space to take appropriate action (unlock mutexes, notify waiting tasks).
3) Tare function included as part of Scale class.
4) IOCTL is used to communicate between user and kernel space to:
  a) Send abort signal.
  b) Change timeout duration.
  
### Prerequisites
1) You have to enable the avia-hx711 driver using ```menuconfig```  and rebuild the kernel. ([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/linux/kernel/building.md)) As a side note, do not activate the existing invensense mpu-6050 kernel driver. The mpu-6050 is directly programmed by my driver, specifically to activate motion sensing. 
2) You have to add the custom device tree overlay in the device tree folder to your personal device. I built the .dtbo using the ```dtc``` command and made sure it was added to the device tree.([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/configuration/device-tree.md))
3) Make the included script executible using ```chmod u+x piscale_script.sh```


![alt text](https://i.imgur.com/e3NxqSr.jpg)

Hardware Legend:
1) 30 kg Load Sensor
2) [HX711](https://www.sparkfun.com/products/13879)
3) [MPU-6050](https://www.sparkfun.com/products/11028)
4) [Raspberry Pi 3 A+](https://www.raspberrypi.org/products/raspberry-pi-3-model-a-plus/4)

The pin connections are:

| Raspberry Pi | HX711   |
|--------------|---------|
| BCM pin 23   | CLK     |
| BCM pin 24   | DAT     |
| 5V           | VCC&VDD |

| Raspberry Pi | MPU6050 |
|--------------|---------|
| I2C1 SDA     | SDA     |
| I2C1 SCL     | SCL     |
| 3v3          | VCC     |
| BCM pin 4    | INT     |

Type ```$ ./piscale_script.sh``` to build everything, insert modules into kernel, and run the application.

Weights will be read to a .txt file in the user_space folder and also displayed to the console.

### Description
The PiScale system is meant to handle the kernel side of any weight scale built on a linux device. The PiScale reads raw data from an HX711 weight sensor and outputs to a file. There are many userspace programs available for parsing data from an HX711. 

By design, the whole process is handled in a very efficient way. Despite the userspace program starting 2 threads, each with a while(1) loop, the system consumes a negligible amount of computing power. Two separate kernel modules are built and added into the kernel. The first module activates highly sensitive motion interrupts in the mpu 6050. Whenever a slight amount of motion is detected, the INT pin on the mpu 6050 goes high momentarily.

The second kernel module **handles** the motion interrupt by registering an interrupt and attaching a handler function. The module uses ```wake_up_interruptible``` and ```wait_event_interruptible``` to sleep while waiting for a motion interrupt. When an interrupt is received, the system is signalled to be active for 40 seconds (time can be changed in header file). The timer restarts everytime a motion interrupt is received and the system remains active for another 60 seconds. During the active period, the user space application is woken-up and a producer-consumer situation safely takes place without any race conditions. The consumer thread sleeps when there is no more data to consume.

## Flowchart of system

![](https://i.imgur.com/eD3pFyH.png)

To be done:
1) [Add LCD](https://www.amazon.ca/SunFounder-Serial-Module-Arduino-Mega2560/dp/B01GPUMP9C/ref=asc_df_B01GPUMP9C/?tag=googleshopc0c-20&linkCode=df0&hvadid=335380394635&hvpos=1o2&hvnetw=g&hvrand=2992788635486907915&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9001527&hvtargid=pla-572925702212&psc=1)

Feel free to contact me at gabe.teeger@gmail.com if you have any questions.

