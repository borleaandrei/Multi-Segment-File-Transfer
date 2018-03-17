#include "server.h"
#include "client.h"

char filename[100], host[] = "localhost";

void *threadfunc(void *sgmt) {
    int nr, size;
    download_info *info = (download_info *) sgmt;
    char seg_fname[256], command[100], buff[BUFSIZE];
    struct sockaddr_in local_addr, remote_addr;
    FILE *f;
    int verif=0;

    snprintf(seg_fname, 255, "file_%d", info->seg_id);
    if ((f = fopen(seg_fname, "wb")) == 0) {
        perror("file error!");
        exit(7);
    }


    snprintf(command, 100, "descarca %s %d %d\n", filename, info->segment_size, info->address);
    
    verif=stream_write(info->socketfd, command, (int) strlen(command));



    size = info->segment_size;

    printf("[Client] seg_id:%s address:%d segment_size:%d\n", seg_fname, info->address, info->segment_size);
    while (size > 0 && (nr = stream_read(info->socketfd, buff, size >= BUFSIZE ? BUFSIZE : size)) > 0) {
        fwrite(buff, sizeof(char), nr, f);
        size -= nr;
    }

    printf("[Client] transfer end\n");
    fclose(f);
    shutdown(info->socketfd, SHUT_WR);
    return 0;
}

int main(int argc, char *argv[]) {

    char command[100], f_size[MAX_SIZE_LENGTH];
    int sockets[SERVER_NUMBER], segment_number, number_of_ports = 0, sockfd, i, j, file_size = 0;
    struct sockaddr_in local_addr, remote_addr;
    pthread_t threads[SERVER_NUMBER * MAX_CHANNELS];
    download_info info[SERVER_NUMBER * MAX_CHANNELS];
    pthread_mutex_init(&mutex, NULL);
    FILE *f, *g;

    //<client> nume_fisier nr_segmente port1..portn
    if (argc < 4) {
        perror("<client>: nume_fisier nr_segmente port1 ... portn\n");
        exit(1);
    }

    strcpy(filename, argv[1]);
    segment_number = atoi(argv[2]);


    for (i = 3; i < argc; i++) {
        if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
            perror("[Client] socket error!");
            exit(2);
        }

        set_addr(&local_addr, NULL, INADDR_ANY, 0);
        if (bind(sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1) {
            perror("[Client] bind error!");
            exit(3);
        }

        //memorez serverele care contin fisierul
        if (set_addr(&remote_addr, host, 0, (short int) atoi(argv[i]))) {
            perror("[Client] set adress errror!");
            exit(4);
        }
        if (connect(sockfd, (struct sockaddr *) &remote_addr, sizeof(remote_addr))) {
            perror("[Client] connection error!");
            exit(5);
        }

        snprintf(command, 100, "exista %s\n", filename);
        stream_write(sockfd, command, (int) strlen(command));
        stream_read(sockfd, f_size, sizeof(char) * MAX_SIZE_LENGTH);

        // Keep connection on only with servers that have the file
        if (atoi(f_size) > 0) {
            file_size = atoi(f_size);
            // Connection established, remember socket file descriptor for future use
            sockets[number_of_ports++] = sockfd;
        }
            // Close connection for servers that do not have the file
        else {
            shutdown(sockfd, SHUT_WR);
        }

    }

    int segment_size = file_size / segment_number;
    int file_remainder = file_size % segment_number;
    int segment_per_server = segment_number / (number_of_ports-1);

    int segment_start_address = 0;
    int thread_number = 0;

    printf("number of servers:%d\n", number_of_ports);
    printf("segment size:%d\n", segment_size);
    printf("segment_per_server:%d\n", segment_per_server);
    printf("file_size:%d\n", file_size);

    int seg_id = 0;
    // -1 because the last port is treated separately
    for (i = 0; i < number_of_ports - 1; i++) {
        //segment_number -= segment_number / number_of_ports;
        segment_number -= segment_per_server;

        for (j = 0; j < segment_per_server; j++) {

            if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
                perror("[Client] socket error!");
                exit(2);
            }

            set_addr(&local_addr, NULL, INADDR_ANY, 0);
            if (bind(sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1) {
                perror("[Client] bind error!");
                exit(3);
            }

            //memorez serverele care contin fisierul
            if (set_addr(&remote_addr, host, 0, (short int) atoi(argv[i+3]))) {
                perror("[Client] set adress errror!");
                exit(4);
            }
            if (connect(sockfd, (struct sockaddr *) &remote_addr, sizeof(remote_addr))) {
                perror("[Client] connection error!");
                exit(5);
            }        
            file_size -= segment_size;

            pthread_mutex_lock(&mutex);
            info[thread_number].socketfd = sockfd;
            info[thread_number].segment_size = segment_size;
            info[thread_number].address = segment_start_address;
            info[thread_number].seg_id = ++seg_id;
            if (pthread_create(&threads[thread_number], NULL, threadfunc, (void *) &info[thread_number]) != 0) {
                perror("Couldn't create a new thread!");
                exit(EXIT_FAILURE);
            }
            thread_number++;
            segment_start_address += segment_size;
            pthread_mutex_unlock(&mutex);
        }
    }
    //Consider the remaining segments - take them from last server
    if(file_remainder > 0 )
        segment_number ++;

    for(j = 0; j < segment_number; ++j) {

        if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
            perror("[Client] socket error!*");
            exit(2);
        }

        set_addr(&local_addr, NULL, INADDR_ANY, 0);
        if (bind(sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1) {
            perror("[Client] bind error!*");
            exit(3);
        }

        //memorez serverele care contin fisierul
        if (set_addr(&remote_addr, host, 0, (short int) atoi(argv[argc-1]))) {
            perror("[Client] set adress errror!*");
            exit(4);
        }
        if (connect(sockfd, (struct sockaddr *) &remote_addr, sizeof(remote_addr))) {
            perror("[Client] connection error!*");
            exit(5);
        }
        
        pthread_mutex_lock(&mutex);
        info[thread_number].socketfd = sockfd;
        //info[thread_number].segment_size = segment_size;
        info[thread_number].segment_size = file_size < segment_size ? file_size : segment_size;
        info[thread_number].address = segment_start_address;
        info[thread_number].seg_id = ++seg_id;
        if (pthread_create(&threads[thread_number], NULL, threadfunc, (void *) &info[thread_number]) != 0) {
            perror("Couldn't create a new thread!");
            exit(EXIT_FAILURE);
        }
        thread_number++;
        segment_start_address += segment_size;
        pthread_mutex_unlock(&mutex);

        file_size = file_size < segment_size ? file_size : file_size - segment_size;

    }


    for (j = 0; j < thread_number; j++) {
        pthread_join(threads[j], NULL);
    }


    //formarea fisierului descarcat

    if ((f = fopen(filename, "wb")) == NULL) {
        perror("file error 8!");
        exit(8);
    }

    char seg_fname[256];
    char buff[BUFSIZE];
    int nr, read_bytes;
    for (j = 0; j < thread_number; j++) {

        snprintf(seg_fname, 255, "file_%d", j + 1);

        if ((g = fopen(seg_fname, "rb")) == 0) {
            perror("file error 9!");
            exit(9);
        }

        while ((read_bytes = fread(buff, sizeof(char), BUFSIZE, g)) != 0) {
            //check
            fwrite(buff, sizeof(char), read_bytes, f);
        }

        fclose(g);

        if (remove(seg_fname) != 0) {
            perror("Unable to delete the file");
            exit(10);
        }

    }

    fclose(f);


    return 0;
}
