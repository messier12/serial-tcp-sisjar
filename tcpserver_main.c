#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>


#define MAX 80
#define SA struct sockaddr



int thread_killer = 0;
struct ThreadArgs
{
    int socket;
    int id;
    char* buffer;
};

int client_id;
volatile short exit_requests[100];

void thread_cleanup_handler(void* _args)
{
    struct ThreadArgs* args = (struct ThreadArgs*)_args;
    printf("cleaning thread%d..\n",args->id);
    exit_requests[args->id] = 0;
    free(args->buffer);
    free(args);
    printf("cleaning done");
}


//void end_thead_by_id(int id);
//void signal_handler(int sig_number, siginfo_t* sig_info, void* context){
//    struct ThreadArgs* args = (struct ThreadArgs*)context;
//    if(sig_number == SIGPIPE)
//    {
//        printf("thread%d accessed a closed socket%d",args->id,args->id);
//        end_thread_by_id(args->id);
//    }
//    if(sig_number == SIGKILL || sig_number == SIGTERM){
//        printf("exitting..\n");
//        for(int i=0;i<=client_id;i++)
//            end_thread_by_id(i);
//        exit(1);
//    }
//}
   
//void loop(struct ThreadArgs* args, const char* logfile);
void thread_run(void* threadargs)
{
    pthread_cleanup_push(thread_cleanup_handler,threadargs);
    struct sigaction sig_action;

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

    //printf("debug\n");
    int n;
    // infinite loop for chat
    FILE* fp = fopen(logfile,"a+");
    printf("START READING\n");


    for (;exit_requests[args->id] == 0;) {
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        //int len = recv(sockfd, buff, sizeof(buff),MSG_DONTWAIT);
        int len = recv(sock, buff, sizeof(buff),0);
        //printf("client%d: ",args->id);
        //for(int i=0;i<len;i++)
        //    printf("%c",buff[i]);
        //printf("\n");
        if(len<=0)
        {
            if(errno == SIGPIPE)
            {
                printf("Thread%d: socket %d is closed\n",args->id,args->socket);
                end_thread_by_id(args->id);
            }
            continue;
        }

        int sent_bytes = send(sock,"ok",3,0);
        if(sent_bytes == -1)
        {
            printf("error %d %s",errno,strerror(errno));
        }
        
        printf("From client%d %d bytes: %s\n",args->id,len,buff);

        fprintf(fp,"%s\n",buff);
        fflush(fp);
    }

    fclose(fp);
    pthread_cleanup_pop(1);
}
   

void end_thread_by_id(int id)
{
    exit_requests[id] = 1;
    //if(pthread_kill(thread_ids[id],S)==0)
        //if(pthread_cancel(thread_ids[id])!=0)
            printf("ERROR: end thread %d failed\n",id);
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

    client_id = -1;
    for(int i=0;i<100;i++)
        exit_requests[i] = 0;
   
    while(1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            continue;
            //exit(0);
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
        //pthread_create(&thread_ids[client_id],NULL,thread_run,thread_args);
        //printf("debug\n");
        if(pthread_create(&thread_ids[client_id],NULL,thread_run,thread_args)!=0)
        {
            printf("error pthread_create");
            //exit(1);
        }
    }
    // After chatting close the socket
    for(int i=0;i<client_id;i++)
        end_thread_by_id(i);

    close(sockfd);
    return 0;
}