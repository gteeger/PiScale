#ifndef MOTION_TIMER_H
#define MOTION_TIMER_H

#define TRUE (1)
#define FALSE (0)

#define GPIO_INT_PIN (4) // BCM 4
#define DEFAULT_TIMEOUT_MS (90000) //90 seconds
#define DEFAULT_TIMEOUT_JIFFIES msecs_to_jiffies(DEFAULT_TIMEOUT_MS)
#define MAJOR_NUM (117)

#define MEASURING (1)
#define OFF (0)

#endif
