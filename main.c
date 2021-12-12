#include "SerialConnection.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#define MAX 80


int main(int argc, char** argv)
{
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

        // set socket timeouts
        struct timeval socket_timeout;
        socket_timeout.tv_sec = 10;
        socket_timeout.tv_usec = 0;
        if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO
            ,(char*)&socket_timeout,sizeof(socket_timeout)) < 0)
        {
            printf("ERROR: set socket receive timeout failed [%s]\n",strerror(errno));
        }
        if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO
            ,(char*)&socket_timeout,sizeof(socket_timeout)) < 0)
        {
            printf("ERROR: set socket send timeout failed [%s]\n",strerror(errno));
        }

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
            count = 0;
            msg[len] = '\0';
            printf("serial message %d bytes: %s\n",len,msg);
            //int ret = write(sockfd,msg,sizeof(char)*strlen(msg));
            int sent_bytes = send(sockfd,msg,sizeof(char)*len,0);
            if(sent_bytes == -1)
            {
                printf("ERROR: bytes not sent [%s]\n",strerror(errno));
                continue;

            }
            else if(sent_bytes != len)
            {
                printf("WARNING: %d bytes sent instead of %d\n",sent_bytes,len);
            }
            char server_msg[80];

            int server_msg_bytes = recv(sockfd,server_msg,sizeof server_msg,0);
            if(server_msg_bytes < 1)
            {
                printf("ERROR: failed to recevie non-repudiation confirmation [%s]\n",strerror(errno));
                continue;
            }
            else{
                if(strcmp("ok",server_msg) == 0)
                    printf("non-repudiation confirmed\n");
                else
                    printf("WARNING: non-repudiation cannot be confirmed\n");
            }

            //read(sockfd,server_msg,sizeof(msg));
            //if(strcmp(server_msg,msg)==0)
            //    printf("message sent successfully\n");
            //else
            //    printf("package loss\n");
        }
    return 0;
}