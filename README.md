# PiScale

### Prerequisites
1) You have to enable the avia-hx711 driver using ```menuconfig```  and rebuild the kernel. ([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/linux/kernel/building.md))
2) You have to add the custom device tree overlay in the device tree folder to your personal device. I built the .dtbo using the ```dtc``` command and made sure it was added to the device tree.([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/configuration/device-tree.md))
3) Make the included script executible using ```chmod u+x piscale_script.sh```


![alt text](https://i.imgur.com/e3NxqSr.jpg)

Legend:
1) 30 kg Load Sensor
2) [HX711](https://www.sparkfun.com/products/13879)
3) [MPU-6050](https://www.sparkfun.com/products/11028)
4) [Raspberry Pi 3 A+](https://www.raspberrypi.org/products/raspberry-pi-3-model-a-plus/4)

The pins connections are:

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

Enter ```$ ./piscale_script.sh``` to build everything, insert modules into kernel, and run application.

Weights will be read to a .txt file in the user_space folder and also displayed to the console.

### Description
The PiScale system is meant to handle the kernel side of a scale built on any linux device. The PiScale reads raw data from an HX711 weight sensor and outputs to a file. There are many userspace programs available for parsing data from an HX711. Parsed data will be availible in a future update.

By design, the whole process is handled in a very efficient way. Despite the userspace program starting 3 threads, each with a while(1) loop, the system consumes a negligible amount of computing power. Two seperate kernel modules are built and added into the kernel. The first module activates highly sensitive motion interrupts in the mpu 6050. Whenever a slight amount of motion is detected, the INT pin on the mpu 6050 goes high momentarily.

The second kernel module **handles** the motion interrupt by registering an interrupt and attaching a handler function. The module uses ```wake_up_interruptible``` and ```wait_event_interruptible``` to sleep while waiting for a motion interrupt. When an interrupt is received, the system is signalled to be active for 40 seconds (time can be changed in header file). The timer restarts everytime a motion interrupt is received and the system remains active for another 40 seconds. During the active period, the user space application is woken-up and a producer-consumer situation safely takes place without any race conditions. Raw data is read from the HX711 into a shared buffer and written to an external .txt file. After the active period, all threads are put to sleep, waiting to be signalled by another motion interrupt. 

## Flowchart of system

![](https://i.imgur.com/eD3pFyH.png)

To be done:
1) IOCTL to control timeout duration
2) Parse raw data
3) [Add LCD](https://www.digikey.ca/product-detail/en/adafruit-industries-llc/181/1528-1502-ND/5774228?utm_adgroup=&mkwid=shWzDs5xh&pcrid=311490127651&pkw=&pmt=&pdv=c&productid=5774228&slid=&gclid=Cj0KCQiAiNnuBRD3ARIsAM8KmlslutH_HnAIhLKwgoaq1fndYZQv7aj8ZTiZlcJMMY0J2rC1_IaQU5UaAp50EALw_wcB)

Feel free to contact me at gabe.teeger@gmail.com if you have any questions.

