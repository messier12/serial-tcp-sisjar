#include "SerialConnection.h"

int main()
{
    struct SerialComm* serial = create_serial("/dev/ttyACM0",9600);
    printf("device: %s\n",serial->device_name);
    //printf("baudrate: %d",serial->baudrate);

    char msg[100];
    printf("Test\n");
    int count=0;
    open_connection(serial);
    printf("open_connection done\n");
    set_interface_attribs(serial);
    printf("set_interface_attribs done\n");
    while(true)
    {
        int len = read_data(serial,msg,10000); //timeout 10 detik
        if(len<0)
        {
            if(count>10)
            {
                printf("error\n");
                return -1;
            }
            count++;
            continue;
        }
        msg[len] = '\0';
        printf("%s\n",msg);
    }
    return 0;
}