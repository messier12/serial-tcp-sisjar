#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>


#define MAX 80
#define SA struct sockaddr



int thread_killer = 0;
struct ThreadArgs
{
    int socket;
    int id;
    char* buffer;
};

void thread_cleanup_handler(void* _args)
{
    struct ThreadArgs* args = (struct ThreadArgs*)_args;
    printf("cleaning thread%d..\n",args->id);
    free(args->buffer);
    free(args);
    printf("cleaning done");
}

   
void loop(struct ThreadArgs* args, const char* logfile);
void thread_run(void* threadargs)
{
    pthread_cleanup_push(thread_cleanup_handler,threadargs);

    struct ThreadArgs* args = (struct ThreadArgs*)threadargs;
    char logfile[100] = {'\0'};
    sprintf(logfile,"log%d.txt",args->id);
    //loop(args,filename);
    //    loop(args->socket, filename,args->buffer);

    // Function designed for chat between client and server.
    //void loop(struct ThreadArgs* args, const char* logfile)

    int sock;
    sock = args->socket;
    char* buff = args->buffer;

    printf("debug\n");
    int n;
    // infinite loop for chat
    FILE* fp = fopen(logfile,"a+");
    printf("START READING\n");

    for (;exit_requests[args->id] == 0;) {
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        //int len = recv(sockfd, buff, sizeof(buff),MSG_DONTWAIT);
        int len = recv(sock, buff, sizeof(buff),0);
        printf("client%d: ",args->id);
        for(int i=0;i<len;i++)
            printf("%c",buff[i]);
        printf("\n");
        send(sock,"ok",3,0);
        if(len<=0)
            continue;
        
        // print buffer which contains the client contents
        printf("From client%d %d bytes: %s\n",args->id,len,buff);

        fprintf(fp,"%s\n",buff);
        fflush(fp);
    }


    fclose(fp);
    pthread_cleanup_pop(1);
}
   

volatile short exit_requests[100];
void end_thread_by_id(int id)
{
    exit_requests[id] = 1;
}
pthread_t thread_ids[100];
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

    int client_id = -1;
    for(int i=0;i<100;i++)
        exit_requests[i] = 0;
   
    while(1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            continue;
            exit(0);
        }
        else
        {
            client_id++;
            printf("server accept the client... ID = %d\n",client_id);
        }


        //prepare the log file

        // Function for chatting between client and server
        //loop(connfd,"log.txt");
        struct ThreadArgs* thread_args =(struct ThreadArgs*)malloc(sizeof(struct ThreadArgs));
        thread_args->buffer = (char*)malloc(sizeof(char)*MAX);
        thread_args->id = client_id;
        thread_args->socket = connfd;
        exit_requests[client_id] = 0;
        int ret = pthread_create(&thread_ids[client_id],NULL,thread_run,thread_args);
        //printf("debug\n");
        if(ret!=0)
        {
            printf("error pthread_create");
            exit(1);
        }
    }
    // After chatting close the socket
    close(sockfd);
    // close the file
}