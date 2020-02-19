#ifndef SCALE_H
#define SCALE_H
#include <vector>
#include <iostream>
/* Share header file for ioctl */
#include <sys/ioctl.h>
#include "../../kernel_space/motion_interrupt_handler/motion_timer.h"
using namespace std;

#define SCALE_OFFSET 1
//#define SCALE 40327.0

#define TARE_ITR 5

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
    double hx_read();
    void set_status(const bool status_flag);
    bool get_status() const;
    void change_timeout(unsigned long msecs);
    void clear_data();
    void print_data();
    double tare();
    void push_data(double data);
    int fd;
    
    friend ostream &operator<<(ostream &out, Scale &scale);

private:
    vector<double> data_vector;
    bool current_system_status;
    int idx;
    double tare_offset;

};

#endif // SCALE_H

