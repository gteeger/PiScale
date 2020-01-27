#ifndef SCALE_H
#define SCALE_H

#include <iostream>
/* Share header file for ioctl */
#include "../../kernel_space/motion_interrupt_handler/motion_timer.h"
#include <sys/ioctl.h>

#define NUMBER_OF_LOAD_CELLS (4)

#define SCALE_OFFSET 1
//#define SCALE 40327.0

#define TARE_ITR 5
#define NUMBER_OF_LOAD_CELLS (4)

#define ARRAY_SIZE (1000)

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

class Scale
{
public:
    Scale();
    ~Scale();
    void init_scale();
    float hx_read();
    void set_status(bool status_flag);
    bool get_status();
    void change_timeout(unsigned long msecs);
    int fd;


private:
    int num_cells;
    volatile float tare_offset;
    float tare();
    volatile int idx;
    volatile bool current_system_status;
    int data_array[ARRAY_SIZE];


};

#endif // SCALE_H
