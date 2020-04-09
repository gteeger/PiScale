# PiScale

### Updates:
April 7th 2020

1) I switched to[these load sensors from Sparkfun](https://www.sparkfun.com/products/10245).
2) PCB designed in KiCAD to integrate various components. See hardware folder for schematic, layout, BOM, and gerbers.
3) The PCB is compatible with either Arduino or Raspberry Pi. There are through holes in the PCB to mount an Arduino Nano.
4) There are numerous taps on the PCB allowing external connection to a Raspberry Pi.
5) Jumpers are used to disconnect the Arduino and Raspberry Pi 3V3 and 5V power supplies.
6) On board buttons can be used to manage interrupts.
7) There is a circuit to properly combine 4 half-bridge load sensors based on the [combinator board](https://www.sparkfun.com/products/13878) from Sparkfun. The correct connections for the 4 load sensors are written in silk screen on the PCB (UL: upper left, UR: upper right, LL: lower left, LR: lower right ).
8) It is also possible to attach a [full bridge load sensor](https://www.sparkfun.com/products/13329) to the pins on the PCB marked E+, E-. A+, A-, B+, B-.
9) There is room for an MPU-6050 to be mounted on the PCB, and pins to connect and LCD. Both the MPU and the LCD pins are connected to I2C SDA and SCL wires.
10) You can connect an external power supply to the PCB (Like a 9V battery) that is connected to the Vin pin of the Arduino.
11) Jumpers are used to connect to either the A or B channel of the HX711 (see the HX711 datasheet for more information on A and B channels).
12) A jumper is used to connect the HX711 to either a 5V or 3V3 power supply.
13) Arduino code is still in progress but Raspberry Pi code still works fine.

 ![](https://imgur.com/0xaxzA0.jpg)
 ![](https://imgur.com/LUfB7Sy.jpg)
 ![PCB Front](https://imgur.com/KrL7wAd.jpg)
 ![PCB Back](https://imgur.com/fgsT8lq.jpg)
Jan 26th 2020
1) User-space program is now written in C++ with an interface to a Scale class.
2) Pressing ctrl+c while program is running issues a signal to abort the program and notifies kernel space to take appropriate action (unlock mutexes, notify waiting tasks).
3) Tare function included as part of Scale class.
4) IOCTL is used to communicate between user and kernel space to:
  a) Send abort signal.
  b) Change timeout duration.
  
### Raspberry Pi Prerequisites
1) You have to enable the avia-hx711 driver using ```menuconfig```  and rebuild the kernel. ([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/linux/kernel/building.md)) As a side note, do not activate the existing invensense mpu-6050 kernel driver. The mpu-6050 is directly programmed by my driver, specifically to activate motion sensing. 
2) You have to add the custom device tree overlay in the device tree folder to your personal device. I built the .dtbo using the ```dtc``` command and made sure it was added to the device tree.([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/configuration/device-tree.md))
3) Make the included script executible using ```chmod u+x piscale_script.sh```


![alt text](https://i.imgur.com/e3NxqSr.jpg)

Hardware Legend:
1) 30 kg Load Sensor
2) [HX711](https://www.sparkfun.com/products/13879) : this is an IC which is a combination of an analog to digital converter and programmable gain amplifier. It uses a bit-banging protocol, so its clock and data lines must be connected to GPIO pins. 
3) [MPU-6050](https://www.sparkfun.com/products/11028) : Accelerometer/Gyroscope. Used as a motion detector.
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
| BCM pin 17   | INT     |

Type ```$ ./piscale_script.sh``` to build everything, insert modules into kernel, and run the application.

Weights will be read to a .txt file in the user_space folder and also displayed to the console.

### Description
The PiScale system is meant to handle the kernel side of any weight scale built on a linux device. The PiScale reads raw data from an HX711 weight sensor and outputs to a file. 

By design, the whole process is handled is very efficient. Despite the userspace program starting 2 threads, each with a while(1) loop, the system consumes a negligible amount of computing power. Two separate kernel modules are built and added into the kernel. The first module activates highly sensitive motion interrupts in the mpu 6050. Whenever a slight amount of motion is detected, the INT pin on the mpu 6050 goes high momentarily.

The second kernel module **handles** the motion interrupt by registering an interrupt and attaching a handler function. The module uses ```wake_up_interruptible``` and ```wait_event_interruptible``` to sleep while waiting for a motion interrupt. When an interrupt is received, the system is signalled to be active for 40 seconds (time can be changed in header file). The timer restarts everytime a motion interrupt is received and the system remains active for another 60 seconds. During the active period, the user space application is woken-up and a producer-consumer situation safely takes place without any race conditions. The consumer thread sleeps when there is no more data to consume.

## Flowchart of system

![](https://i.imgur.com/eD3pFyH.png)

Feel free to contact me at gabe.teeger@gmail.com if you have any questions.

