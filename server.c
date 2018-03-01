//
// Created by sergiu on 2/28/18.
//

#include "server.h"

pthread_mutex_t mutex;
int threadNumber = 0;

int readline_init(int sfd)
{
    int val;
    socklen_t len = sizeof(int);

    getsockopt(sfd, SOL_SOCKET, SO_RCVLOWAT, &val, &len);
    if(val != 1)
    {
        len = sizeof(int);
        setsockopt(sfd, SOL_SOCKET, SO_RCVLOWAT, (void*)&val, len);
        getsockopt(sfd, SOL_SOCKET, SO_RCVLOWAT, &val, &len);
        if(val != 1)
        {
            return -1;
        }
    }
    return 0;
}

int readline(int connfd, char *line, int *idx, char *buf, int maxlen)
{
    int ret , n;
    for (; *idx < maxlen; *idx += ret)
    {
        ret = read(connfd, buf + *idx, maxlen - *idx);
        if (ret <= 0 )
        return ret ;
        for (n = *idx; n < *idx + ret; n++)
            if (buf[n] == '\n')
            {
                memcpy (line, buf, n+1);
                memmove(buf, buf + n + 1, ret - n - 1);
                *idx = ret - n - 1;
                return n + 1 ;
            }
    }
    return -1;
}


void * server_function(void *arg)
{
    int ret;
    char line[BUFSIZE];
    char buf[BUFSIZE];
    int idx = 0;
    int *connfd = (int *)arg;

    while(1)
    {
        /*Read the request issued by the client*/
        while(0 < (ret = readline(*connfd, line, &idx, buf, BUFSIZE)))
        {

        }
    }
    pthread_mutex_lock(&mutex);
    --threadNumber;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(void)
{
    /*Start a server and listen at SERVER_PORT*/
    int sockfd, *connfd;
    struct sockaddr_in local_addr, remote_addr;
    socklen_t rlen;
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    pthread_mutex_init(&mutex, NULL);

    if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
    {
        perror("The socket could not be created!");
        exit(EXIT_FAILURE);
    }

    if(-1 == readline_init(sockfd))
    {
        perror("Initialization of readline failed!");
        exit(EXIT_FAILURE);
    }
    set_addr(&local_addr, NULL, INADDR_ANY, SERVER_PORT);

    if(-1 == bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)))
    {
        perror("Error at bind()!");
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(sockfd, 5))
    {
        perror("Error at listen()!");
        exit(EXIT_FAILURE);
    }

    rlen = sizeof(remote_addr);

    while(1)
    {
        pthread_mutex_lock(&mutex);
        /*If the maximum number of connections is reached, do not accept the incoming connection*/
        if(threadNumber + 1 >= MAX_CHANNELS)
        {
            pthread_mutex_unlock(&mutex);
            perror("Maximum number of channels reached! Connection refused!");
            sleep(1);
            continue;
        }
        pthread_mutex_unlock(&mutex);

        connfd = (int *)malloc(sizeof(int));
        if(connfd == NULL)
        {
            perror("Error at malloc()!");
            exit(EXIT_FAILURE);
        }

        if((*connfd = accept(sockfd, (struct sockaddr *)&remote_addr, &rlen)) < 0)
        {
            perror("Error at accept()!");
            exit(EXIT_FAILURE);
        }
        if(0 != pthread_create(&thread, &attr, server_function, (void *)connfd))
        {
            perror("Couldn't create a new thread!");
            exit(EXIT_FAILURE);
        }

        /*Increment number of active threads/channels*/
        pthread_mutex_lock(&mutex);
        ++threadNumber;
        pthread_mutex_unlock(&mutex);
    }
}