#include "SerialConnection.h"



struct SerialComm* create_serial(const char* device, int baudrate)
{
    struct SerialComm* serial = (struct SerialComm*)malloc(sizeof(struct SerialComm));
    serial->device_name = (char*)malloc(sizeof(char)*(strlen(device)+10));
    strcpy(serial->device_name,device);
    serial->baudrate = baudrate;
    printf("Created serial object\n");
    return serial;
}

int set_interface_attribs(struct SerialComm* serial){
    if (tcgetattr(serial->fd,&(serial->tty)) < 0)
    {
      printf("Error from tcgetattr: %s\n", strerror(errno));
      return -1;
    }
    
    cfsetospeed(&(serial->tty), (speed_t)serial->baudrate);
    cfsetispeed(&(serial->tty), (speed_t)serial->baudrate);
    
    serial->tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
    serial->tty.c_cflag &= ~CSIZE;
    serial->tty.c_cflag |= CS8;      /* 8-bit characters */
    serial->tty.c_cflag &= ~PARENB;  /* no parity bit */
    serial->tty.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
    serial->tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */
    
    /* setup for non-canonical mode */
    serial->tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    serial->tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    serial->tty.c_oflag &= ~OPOST;
    
    /* fetch bytes as they become available */
    serial->tty.c_cc[VMIN] = 1;
    serial->tty.c_cc[VTIME] = 1;
    
    if (tcsetattr(serial->fd, TCSAFLUSH, &(serial->tty)) != 0)
    {
      printf("Error from tcsetattr: %s\n", strerror(errno));
      return -1;
    }
    return 0;
    
}

int open_connection(struct SerialComm* serial){
    serial->fd = open(serial->device_name, O_RDWR | O_NOCTTY | O_SYNC);

    if (serial->fd < 0)
    {
        printf("Error opening %s: %s\n", serial->device_name, strerror(errno));
        serial->connected = 0;
        return -1;
    }
    // //qdebug()<<"Connected to STM";
    serial->connected = 1;
    set_interface_attribs(serial);
    tcflush(serial->fd, TCIOFLUSH);
    usleep(10);
    return 0;
}
void close_connection(struct SerialComm* serial){
    usleep(10); // tcflush perlu ini gk tau kenapa..
    tcflush(serial->fd, TCIOFLUSH);
    serial->connected = 0;
    close(serial->fd);
}
void write_data(struct SerialComm* serial,const char* data, int length){
    ; // gk perlu buat tugas sisjar jadi nanti aja.
}
int read_data(struct SerialComm* serial,char* buffer, int timeout){
    // using {} delimiter format
    if(serial->connected != 1)
    {
        printf("SERIAL NOT CONNECTED\n");
        return -1;
    }

    bool begin_read = false;
    bool end_read = false;
    int buffer_idx = 0;
    char readchar[24]; // just to be safe..
    int error_count = 0;
    int read_len = 0;
    clock_t start_time = time(0);
    while(true)
    {
        clock_t now = time(0);
        int elapsed_milisec = (1000*(now-start_time))/CLOCKS_PER_SEC;
        if(elapsed_milisec>timeout){
            printf("READ TIMEOUT\n");
            return -2;
        }
        int ret = read(serial->fd,readchar,1);
        if(ret<0)
        {
            // error_count++;
            // if(error_count > timeout)
            // {
            //     printf("READ ERROR");
            //     return -1;
            // }
            continue;
        }

        if(ret>0&&readchar[0] == '{')
        {
            begin_read = true;
            continue;
        }
        if(begin_read)
        {
            int i;
            for(i=0;i<ret;i++) // using this method for future use.. 
            {
                if(read_len > MAX_BUFF_SIZE)
                {
                    end_read = true;
                    break;
                }
                if(readchar[i]=='}')
                {
                    ret = i;
                    end_read = true;
                    *(buffer+buffer_idx+i) = '\0';
                    break;
                }
                *(buffer+buffer_idx+i) = readchar[i];
                read_len++;
            }
            buffer_idx += min(ret,i);
        }
        if(end_read)
            break;

    }

    return read_len;

}
