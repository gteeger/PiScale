#include <iostream>
#include <sys/ioctl.h>
#include "Scale.h
#include "../../kernel_space/motion_interrupt_handler/motion_timer.h"

Scale::Scale() {}

Scale::~Scale() {}


void Scale::init_scale()
{
    set_status(OFF);
    tare_offset = tare();
}
float Scale::tare()
{

    float tare_val = 0;
    //reset tare offset
    tare_offset = 0;

    //do a few reads,
    for (int i = 0; i < TARE_ITR; i++)
    {
        long unsigned ret_val = hx_read();
        tare_val += ret_val / SCALE_OFFSET;
        printf("tare_val val = %f\n", tare_val);

    }
    float offset = tare_val / TARE_ITR;
    printf("tare offset = %f\n", offset);
    return offset;
}


float Scale::hx_read()
{
    long unsigned ret_val;
    float print_val;
    FILE *proc =
        fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw",
              "r");
    if (!proc)
    {
        fprintf(stderr, "error: process open failed.\n");
        return -1;
    }

    for(int i = 0; i < NUMBER_OF_LOAD_CELLS; i++)
    {
        if(fscanf(proc, "%lu", &ret_val))
        {
            print_val = (ret_val / SCALE_OFFSET) - tare_offset;
            printf("hx711 val = %.3f\n", print_val);
            data_array[idx] += print_val;
            ioctl(fd, MUX_SEL, i);
        }
    }
    idx++;
    fclose(proc);
    return print_val;
}

void Scale::set_status(bool status_flag)
{
    current_system_status = status_flag;
}

bool Scale::get_status()
{
    return current_system_status;
}
void Scale::change_timeout(unsigned long msecs){
    ioctl(fd, TIMEOUT_CHANGE, msecs);
}
