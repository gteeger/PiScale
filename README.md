# PiScale 2.0

### Updates:
1) User-space program is now written in C++ with an interface to a Scale class.
2) Pressing ctrl+c while program is running issues a signal to abort the program and notifies kernel space
3) Tare function included
4) Ability to read up to 4 load cells. The outputs from the cells are selected with a multiplexer.
5) IOCTL is used to communicate between user and kernel space to:
  a) Send abort signal
  b) Cycle thru load sensors
  c) Change timeout duration

### Prerequisites
1) You have to enable the avia-hx711 driver using ```menuconfig```  and rebuild the kernel. ([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/linux/kernel/building.md)) As a side note, do not activate the existing invensense mpu-6050 kernel driver. The mpu-6050 is directly programmed by my driver, specifically to activate motion sensing. 
2) You have to add the custom device tree overlay in the device tree folder to your personal device. I built the .dtbo using the ```dtc``` command and made sure it was added to the device tree.([Raspberry Pi tutorial](https://www.raspberrypi.org/documentation/configuration/device-tree.md))
3) Make the included script executible using ```chmod u+x piscale_script.sh```

Hardware Legend:
1) Any load sensors/load cells containing a full Wheatstone Bridge. [(Example)](https://imgur.com/b92cmWE) 
2) [HX711](https://www.sparkfun.com/products/13879)
3) [MPU-6050](https://www.sparkfun.com/products/11028)
4) [Raspberry Pi 3 A+](https://www.raspberrypi.org/products/raspberry-pi-3-model-a-plus/4)
5) [CD4052B Analog Multiplexer](http://www.ti.com/product/CD4052B)

![Multiplexer Circuit](https://i.imgur.com/cEFdBxD.jpg)

Above is an image of the circuit of the 4052 MUX. Each X and Y is connected to the white(A+) and green (A-) wires of a load cell. 

The pin connections are:

| Raspberry Pi | HX711   |
|--------------|---------|
| BCM pin 23   | CLK     |
| BCM pin 24   | DAT     |
| 3v3          | VCC&VDD |

| Raspberry Pi | 4052 MUX                         |
|--------------|----------------------------------|
| BCM pin 22   | Pin 10(MUX SEL A)                |
| BCM pin 27   | Pin 9(MUX SEL B)                 |
| 3v3          | Vdd                              |
| GND          | Pin 7(Vee)+Pin 8(Vss)+Pin 6(INH) |


| Raspberry Pi | MPU6050 |
|--------------|---------|
| I2C1 SDA     | SDA     |
| I2C1 SCL     | SCL     |
| 3v3          | VCC     |
| BCM pin 4    | INT     |

Type ```$ ./piscale_script.sh``` to build everything, insert modules into kernel, and run the application.

### Description
The PiScale system is meant to handle the kernel side of any weight scale built on a linux device. The PiScale reads raw data from an HX711 weight sensor and outputs to a file. There are many userspace programs available for parsing data from an HX711. 

By design, the whole process is handled in a very efficient way. Despite the userspace program starting 2 threads, each with a while(1) loop, the system consumes a negligible amount of computing power. Two separate kernel modules are built and added into the kernel. The first module activates highly sensitive motion interrupts in the mpu 6050. Whenever a slight amount of motion is detected, the INT pin on the mpu 6050 goes high momentarily.

The second kernel module **handles** the motion interrupt by registering an interrupt and attaching a handler function. The module uses ```wake_up_interruptible``` and ```wait_event_interruptible``` to sleep while waiting for a motion interrupt. When an interrupt is received, the system is signalled to be active for 40 seconds (time can be changed in header file). The timer restarts everytime a motion interrupt is received and the system remains active for another 60 seconds (by default). During the active period, the user space application is woken-up. The reading thread sleeps when there is no more data to consume. After the active period, all threads are put to sleep, waiting to be signalled by another motion interrupt. 

## Flowchart of system

![](https://i.imgur.com/eD3pFyH.png)

To be done:
1) [Add LCD](https://www.amazon.ca/SunFounder-Serial-Module-Arduino-Mega2560/dp/B01GPUMP9C/ref=asc_df_B01GPUMP9C/?tag=googleshopc0c-20&linkCode=df0&hvadid=335380394635&hvpos=1o2&hvnetw=g&hvrand=2992788635486907915&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9001527&hvtargid=pla-572925702212&psc=1)

Feel free to contact me at gabe.teeger@gmail.com if you have any questions.

