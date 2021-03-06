//
// Created by sergiu on 2/28/18.
//

#include "server.h"

pthread_mutex_t mutex;
int threadNumber = 0;

/*"file_name file_name_path file_size"*/
extern FILE *index_file;

int readline_init(int sfd) {
    int val;
    socklen_t len = sizeof(int);

    getsockopt(sfd, SOL_SOCKET, SO_RCVLOWAT, &val, &len);
    if (val != 1) {
        len = sizeof(int);
        setsockopt(sfd, SOL_SOCKET, SO_RCVLOWAT, (void *) &val, len);
        getsockopt(sfd, SOL_SOCKET, SO_RCVLOWAT, &val, &len);
        if (val != 1) {
            return -1;
        }
    }
    return 0;
}

int readline(int connfd, char *line, int *idx, char *buf, int maxlen) {
    int ret, n;
    for (; *idx < maxlen; *idx += ret) {
        ret = read(connfd, buf + *idx, maxlen - *idx);
        if (ret <= 0)
            return ret;
        for (n = *idx; n < *idx + ret; n++)
            if (buf[n] == '\n') {
                memcpy(line, buf, n + 1);
                memmove(buf, buf + n + 1, ret - n - 1);
                *idx = ret - n - 1;
                return n + 1;
            }
    }
    return -1;
}

/*Takes as input filename, for whom path will be completed with its absolute path (in regard to DIR_PATH)
 * and return the size of the filename
 * path may be NULL is filename path is not of interest*/
int does_file_exist(char *filename, char **path) {
    // Open index file in readonly mode - no protection needed
    index_file = fopen(INDEX_FILE_NAME, "r");

    char file_name[255], file_name_path[512];
    int file_name_size = 0;

    // TODO: improve search time (how index file is organized etc.)
    while (fscanf(index_file, "%s %s %d", file_name, file_name_path, &file_name_size) == 3) {
        if (strcmp(file_name, filename) == 0) {
            // Return the path only if the user requiers it
            if (path != NULL) {
                *path = (char *) malloc(sizeof(char) * strlen(file_name_path) + 1);
                strcpy(*path, file_name_path);
            }
            fclose(index_file);
            return file_name_size;
        }
    }

    fclose(index_file);
    // File was not found in the index
    return -1;
}

void *server_function(void *arg) {
    int ret, n, i = 0;
    char line[BUFSIZE];
    char buf[BUFSIZE];
    int idx = 0, flags = 0;
    int *connfd = (int *) arg;
    // Command issued by client
    char *command;
    char f_size[MAX_SIZE_LENGTH];

    while (1) {
        /*Read the request issued by the client*/
        while (0 < (ret = readline(*connfd, line, &idx, buf, BUFSIZE))) {
            printf("Received: %s\n", line);
            // Needed to use the reentrant version of strtok
            char *command_r = line;
            // Split the given commands by spaces in order to identify the command and individual parameters
            command = strtok_r(line, (const char *) " \r\n\t", &command_r);

            // Empty command
            if (strlen(command) == 0)
                continue;

            // exista nume_fisier
            if (strcmp(command, "exista") == 0) {
                // Get the filename parameter
                command = strtok_r(NULL, (const char *) " \r\n\t", &command_r);

                // Transform file_size to string into the f_size char array
                memset((void *) f_size, '\0', MAX_SIZE_LENGTH);
                int size = does_file_exist(command, NULL);
                snprintf(f_size, MAX_SIZE_LENGTH, "%d", size);

                printf("Response: %s\n", f_size);

                stream_write(*connfd, f_size, sizeof(char) * MAX_SIZE_LENGTH);

                continue;
            }

            // descarca nume_fis nr_seg marime_seg adr_inceput
            if (strcmp(command, "descarca") == 0) {
                // Get the filename
                char *filename = strtok_r(NULL, (const char *) " \r\n\t", &command_r);
                // Get segment size
                char *size_s = strtok_r(NULL, (const char *) " \r\n\t", &command_r);
                int size = atoi(size_s);
                // Get start adress of the segment
                char *addr_s = strtok_r(NULL, (const char *) " \r\n\t", &command_r);
                int addr = atoi(addr_s);

                // TODO add response to client in case of failure
                char *path[1];
                *path = (char *) malloc(
                        sizeof(char));// No harm done, just so does_file_exist function will not detect this as NULL
                FILE *to_transfer;
                if (does_file_exist(filename, path) == -1) {
                    perror("Download request failed!");
                    free(*path);
                    continue;
                }

                if ((to_transfer = fopen(*path, "rb")) == NULL) {
                    perror("Download request failed!");
                    free(*path);
                    continue;
                }

                printf("Download request: filename = %s segment_size = %d start_addr = %d path = %s\n", filename, size,
                       addr, *path);

                if (fseek(to_transfer, addr, SEEK_SET) != 0) {
                    perror("Download request failed!");
                    free(path);
                    continue;
                }

                // Write in batches of BUFSIZE in order not to fill RAM in case a very large segment was requested
                char buff[1024] = {'\0'};
                printf("send %d %d\n",size, BUFSIZE);
                while(size >= BUFSIZE)
                {
                    fread(buff, sizeof(char), BUFSIZE, to_transfer);
                    // TODO check that everything has transfered
                    stream_write(*connfd, buff, BUFSIZE);
                    size -= BUFSIZE;
                    
                }
                printf("end stream_write\n");

                fread(buff, sizeof(char), size, to_transfer);
                stream_write(*connfd, buff, BUFSIZE);

                fclose(to_transfer);

                free(*path);
            }
        }
    }
    pthread_mutex_lock(&mutex);
    --threadNumber;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    /*Start a server and listen at SERVER_PORT*/
    int sockfd, *connfd;
    struct sockaddr_in local_addr, remote_addr;
    socklen_t rlen;
    pthread_t thread;
    pthread_attr_t attr;

    if(argc < 2){
        printf("You have to give as argument the server port!\n");
        exit(1);
    }

    create_index_file();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    pthread_mutex_init(&mutex, NULL);

    if (-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0))) {
        perror("The socket could not be created!");
        exit(EXIT_FAILURE);
    }

    if (-1 == readline_init(sockfd)) {
        perror("Initialization of readline failed!");
        exit(EXIT_FAILURE);
    }
    set_addr(&local_addr, NULL, INADDR_ANY, (short)atoi(argv[1]));

    if (-1 == bind(sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr))) {
        perror("Error at bind()!");
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(sockfd, 5)) {
        perror("Error at listen()!");
        exit(EXIT_FAILURE);
    }

    rlen = sizeof(remote_addr);

    while (1) {
        pthread_mutex_lock(&mutex);
        //If the maximum number of connections is reached, do not accept the incoming connection
        if (threadNumber + 1 >= MAX_CHANNELS) {
            pthread_mutex_unlock(&mutex);
            perror("Maximum number of channels reached! Connection refused!");
            sleep(1);
            continue;
        }
        pthread_mutex_unlock(&mutex);

        connfd = (int *) malloc(sizeof(int));
        if (connfd == NULL) {
            perror("Error at malloc()!");
            exit(EXIT_FAILURE);
        }

        if ((*connfd = accept(sockfd, (struct sockaddr *) &remote_addr, &rlen)) < 0) {
            perror("Error at accept()!");
            exit(EXIT_FAILURE);
        }
        if (0 != pthread_create(&thread, &attr, server_function, (void *) connfd)) {
            perror("Couldn't create a new thread!");
            exit(EXIT_FAILURE);
        }

        //Increment number of active threads/channels
        pthread_mutex_lock(&mutex);
        ++threadNumber;
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}