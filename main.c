#include "SerialConnection.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80


int main(int argc, char** argv)
{
    printf("%d",argc);
    //setup the serial stuffs
        char device_name[80] = "/dev/ttyACM0";
        int baudrate = 9600;
        if(argc>1)
            strcpy(device_name,argv[1]);
        if(argc>2)
            baudrate = atoi(argv[2]);
        struct SerialComm* serial = create_serial(device_name,baudrate);
        printf("device: %s\n",serial->device_name);
        printf("baudrate: %d\n",serial->baudrate);
        char msg[100];
        int count=0;
        open_connection(serial);
        printf("open_connection done\n");
        set_interface_attribs(serial);
        printf("set_interface_attribs done\n");

    //setup the tcp-client stuff
        char ipaddr[16] = "127.0.0.1";
        int PORT = 8080;
        if(argc>3)
            PORT = atoi(argv[3]);
        if(argc>4)
            strcpy(ipaddr,argv[4]);
        printf("server: %s\n",ipaddr);
        printf("port: %d\n",PORT);
        int sockfd, connfd;
        struct sockaddr_in servaddr, cli;
        // socket create and varification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully created..\n");
        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(ipaddr);
        servaddr.sin_port = htons(PORT);
   
        // connect the client socket to server socket
        if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            exit(0);
        }
        else
            printf("connected to the server..\n");
    
    //read and send loop
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
        printf("serial message: %s\n",msg);
        write(sockfd,msg,sizeof(char)*strlen(msg));
        char server_msg[80];
        //read(sockfd,server_msg,sizeof(msg));
        //if(strcmp(server_msg,msg)==0)
        //    printf("message sent successfully\n");
        //else
        //    printf("package loss\n");
    }
    return 0;
}