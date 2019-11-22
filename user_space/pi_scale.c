#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE (100)

// Declaration of thread condition variable
pthread_cond_t is_hx711_data = PTHREAD_COND_INITIALIZER;
pthread_cond_t is_sys_awake = PTHREAD_COND_INITIALIZER;

// declaring mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temp_lock = PTHREAD_MUTEX_INITIALIZER;
int shared_array[BUFFER_SIZE];
int producer_index;
int consumer_index;
int shared_array_num;		//number of things in shared array at any point in time
int current_system_status;
int total_count;
#define RUNNING 1
#define OFF 0

void *do_status_read()
{
    const char *name = "/dev/motion_int";
    uint8_t buf;
    int status;
    int fd;
    fd = open(name, O_RDONLY);
    do {
	status = read(fd, &buf, 1);
	current_system_status = buf;
	pthread_cond_signal(&is_sys_awake);
    } while (1);

    close(fd);

}

void *do_file_write()
{
    FILE *fp;

    do {
	fp = fopen("weight_reading.txt", "a");
	pthread_mutex_lock(&lock);	//mutex automatically unlocked while waiting
	if (shared_array_num < 1) {
	    //nothing in shared array
	    pthread_cond_wait(&is_hx711_data, &lock);
	}
	fprintf(fp, "weight reading #%d: %d\n", total_count,
		shared_array[consumer_index]);
	consumer_index++;
	total_count++;
	consumer_index %= BUFFER_SIZE;	//circular buffer
	shared_array_num--;
	pthread_mutex_unlock(&lock);
	fclose(fp);
    } while (1);


}

void *do_hx_read()
{
    do {
	if (current_system_status == OFF) {
	    printf("not running\n");
	    pthread_cond_wait(&is_sys_awake, &temp_lock);
	}
	long unsigned ret_val = 0;
	/* validate pipe open for reading */
	FILE *proc =
	    popen("cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw",
		  "r");
	if (!proc) {
	    fprintf(stderr, "error: process open failed.\n");
	    pthread_exit(NULL);
	}

	if (fscanf(proc, "%lu", &ret_val)) {
	    printf("hx711 val = %lu\n", ret_val);
	} else {
	    printf("error reading from hx711\n");
	}
	pthread_mutex_lock(&lock);
	shared_array[producer_index] = ret_val;
	producer_index++;
	producer_index %= BUFFER_SIZE;
	shared_array_num++;
	pthread_mutex_unlock(&lock);
	pthread_cond_signal(&is_hx711_data);
	pclose(proc);
    } while (1);

}


int main(int argc, char *argv[])
{
    int ret = 0;
    total_count = 0;
    consumer_index = 0;
    producer_index = 0;
    shared_array_num = 0;
    current_system_status = OFF;
    pthread_t tid1, tid2, tid3;
    pthread_create(&tid1, NULL, do_status_read, NULL);
    pthread_create(&tid2, NULL, do_file_write, NULL);
    pthread_create(&tid3, NULL, do_hx_read, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    pthread_exit(NULL);
    return ret;
}
