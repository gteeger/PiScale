#ifndef MOTION_TIMER_H
#define MOTION_TIMER_H

#include <linux/ioctl.h>
typedef uint8_t __u8;
//its highly likely that these will be attempted to be redefined
#ifndef TRUE
#define TRUE 1 //redefined in header
#endif
#ifndef FALSE
#define FALSE 0 //redefined
#endif
#ifndef RUNNING
#define RUNNING 1
#endif
#ifndef OFF
#define OFF 0 //redefined
#endif
#ifndef ON
#define ON 1 //redefined
#endif

#define HI (1)
#define LO (0)

#define GPIO_INT_PIN (17)  //BCM 17

#define DEFAULT_TIMEOUT_MS (1000) //1 second
#define DEFAULT_TIMEOUT_JIFFIES msecs_to_jiffies(DEFAULT_TIMEOUT_MS)
#define MAJOR_NUM (117)

#define MEASURING (1)

#define IOCTL_MAGIC 'k'

#define TIMEOUT_CHANGE _IOW(IOCTL_MAGIC, 1, __u8)
#define ABORT_SIG _IO(IOCTL_MAGIC, 2)

#define TIMEOUT_LIMIT_MS (2000000) // ~30 mins
#define TIMEOUT_MIN_MS (500) //0.5 seconds
 

#endif
