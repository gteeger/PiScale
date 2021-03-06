#include "Scale.h"
Scale::Scale() : current_system_status(OFF), idx(0), tare_offset(0) {}

Scale::~Scale(){clear_data();}

void Scale::init_scale()
{
    tare_offset = tare();
}
void Scale::print_data()
{
    cout << *this << endl;

}
void Scale::clear_data()
{
    idx = 0;
    data_vector.clear();
    cout << "Data Cleared\n";
}
double Scale::tare()
{

    double tare_val(0);

    //reset tare offset
    tare_offset = 0;

    //do a few reads,
    for (int i = 0; i < TARE_ITR; i++)
    {
        long unsigned ret_val = hx_read();
        tare_val += ret_val / SCALE_OFFSET;
        printf("tare_val = %f\n", tare_val);
    }
    double offset = tare_val / TARE_ITR;
    printf("tare offset = %f\n", offset);
    return offset;
}
double Scale::hx_read()
{
    long unsigned ret_val(0);
    double print_val(0);
    FILE *proc =
        fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw",
              "r");
    if (!proc)
    {
        fprintf(stderr, "error: process open failed.\n");
        return -1;
    }

    if(fscanf(proc, "%lu", &ret_val))
    {

        print_val = (ret_val / SCALE_OFFSET) - tare_offset;
        printf("hx711 val = %.3f\n", print_val);
    }
    idx++;
    fclose(proc);
    return print_val;
}
void Scale::set_status(const bool status_flag)
{
    current_system_status = status_flag;
}
void Scale::push_data(double data)
{
    data_vector.push_back(data);
}
bool Scale::get_status() const
{
    return current_system_status;
}
void Scale::change_timeout(const unsigned long msecs)
{
    ioctl(fd, TIMEOUT_CHANGE, msecs);
}

ostream &operator<<(ostream &out, Scale &scale){

    vector<double>::iterator it = scale.data_vector.begin();

    for(; it != scale.data_vector.end(); it++)
    {
        out <<"data value: "<< *it << endl;
    }
    return out;
}
