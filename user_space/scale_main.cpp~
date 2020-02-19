#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <sys/ioctl.h>
#include "include/Scale.h"
#include "../kernel_space/motion_interrupt_handler/motion_timer.h"


using namespace std;

sig_atomic_t exitRequested;
pthread_cond_t is_sys_awake = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int fd;
Scale* s1;

void ctrlc_handler(int s)
{
    exitRequested = TRUE;
    cout << "Signal\n";
    //use ioctl to inform kernel module about abort
    ioctl(fd,ABORT_SIG, NULL);

}


void* do_status_read(void *ptr)
{
    const char *name = "/dev/motion_int";
    bool buf;
    fd = open(name, O_RDONLY);
    s1->fd = fd;
    while(!exitRequested)
    {
        read(fd, &buf, 1);
        s1->set_status(buf);
        pthread_cond_signal(&is_sys_awake);
    }
    close(fd);
    return NULL;
}

void* do_hx_read(void *ptr)
{

    while(!exitRequested)
    {
        if(s1->get_status() == OFF){
            pthread_cond_wait(&is_sys_awake, &lock);
        }
        else{
            s1->hx_read();
        }
    }
    return NULL;
}

int main()
{
    pthread_t tid1, tid2;
    struct sigaction sigIntHandler;
    exitRequested = FALSE;

    sigIntHandler.sa_handler = ctrlc_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    s1 = new Scale;
    s1 -> init_scale();

    pthread_create(&tid1, NULL, do_status_read, NULL);
    pthread_create(&tid2, NULL, do_hx_read, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    delete(s1);

    return 0;

}
