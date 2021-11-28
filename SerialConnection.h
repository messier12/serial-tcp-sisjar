#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <sys/stat.h>

#define MAX_BUFF_SIZE 24
#define min(a,b) (a<b)?a:b
struct SerialComm{
    int fd;
    int connected;
    int baudrate;
    struct termios tty;
    int wlen;
    char* device_name;
};

struct SerialComm* create_serial(const char* device, int baudrate);
int set_interface_attribs(struct SerialComm* serial);
int open_connection(struct SerialComm* serial);
void close_connection(struct SerialComm* serial);
void write_data(struct SerialComm* serial,const char*, int);
int read_data(struct SerialComm* serial,char* buffer, int timeout);
bool is_device_connected(struct SerialComm* serial);