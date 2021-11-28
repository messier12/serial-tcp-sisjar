#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 80
#define SA struct sockaddr
   
// Function designed for chat between client and server.
void loop(int sockfd, const char* logfile)
{
    char buff[MAX];
    int n;
    // infinite loop for chat
    FILE* fp = fopen(logfile,"a+");
    printf("START READING\n");
    for (;;) {
        bzero(buff, MAX);
   
    printf("READING\n");
        // read the message from client and copy it in buffer
        //int len = recv(sockfd, buff, sizeof(buff),MSG_DONTWAIT);
        int len = read(sockfd, buff, sizeof(buff));
        if(len<=0)
            continue;
        
        // print buffer which contains the client contents
        printf("From client %d bytes: %s\n",len,buff);

        fprintf(fp,"%s\n",buff);
        fflush(fp);
        //printf("From client: %s\t To client : ", buff);
        //bzero(buff, MAX);
        //n = 0;
        //// copy server message in the buffer
        //while ((buff[n++] = getchar()) != '\n')
        //    ;
   
        //bzero(buff,sizeof(buff));
        //strcpy(buff,"accepted");
        //// and send that buffer to client
        //write(sockfd, buff, sizeof(buff));
   
        // if msg contains "Exit" then server exit and chat ended.
        //if (strncmp("exit", buff, 4) == 0) {
        //    printf("Server Exit...\n");
        //    break;
        //}
    }
    fclose(fp);
}
   
// Driver function
int main(int argc, char** argv)
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    int PORT = 8080;
    if(argc > 1)PORT = atoi(argv[1]);
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
   
    //prepare the log file
    
    // Function for chatting between client and server
    loop(connfd,"log.txt");
   
    // After chatting close the socket
    close(sockfd);
    // close the file
}