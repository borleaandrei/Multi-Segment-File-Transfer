
#include "server.h"

#define SERVER_NUMBER 5
pthread_mutex_t mutex;
int segment_size;
char filename[100],host[] = "localhost";

typedef struct download_info {
    int port;
    int address;
    int segment_size;
} download_info;


void* threadfunc( void* sgmt )
{
    int seg_id,sockfd, i, nr;
    download_info *info = (download_info *)sgmt;
    char seg_fname[256],command[100], buff[BUFSIZE];
    struct sockaddr_in local_addr, remote_addr;
    FILE *f;

    seg_id = info->address/info->segment_size + 1;
	snprintf(seg_fname, 255, "file_%d", seg_id);
    if((f = fopen(seg_fname,"wb")) == 0){
        perror("file error!");
        exit(7);
    }

    //creez conexiunea cu serverul
    if((sockfd = socket(PF_INET, SOCK_STREAM,0))==-1){
        perror("socket error!");
        exit(2);
    }
    set_addr(&local_addr, NULL, INADDR_ANY, 0);
    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr))==-1)
    {
        perror("bind error!");
        exit(3);
    }
    if(set_addr(&remote_addr, host, 0, info->port)){
        perror("set adress errror!");
        exit(4);
    }
    if(connect(sockfd,(struct sockaddr*)&remote_addr, sizeof(remote_addr))){
        perror("connection error!");
        exit(5);
    }

    snprintf(command, 100, "descarca %s %d %d\n", filename, info->segment_size, info->address); 
    stream_write(sockfd, command, strlen(command)); 

    int size = info->segment_size;
    while(size >= BUFSIZE && (nr = stream_read(sockfd, buff, BUFSIZE)) > 0){
        fwrite(buff, sizeof(char), nr, f);
        i++;
        size = size - BUFSIZE;
    }

    if(size > 0){
        nr = stream_read(sockfd, buff, size);
        fwrite(buff,sizeof(char), nr, f);
    }
    printf("transfer end\n");
    fclose(f);
    shutdown(sockfd, SHUT_WR);
    return 0;
}

int main(int argc, char *argv[]){

    char command[100], f_size[MAX_SIZE_LENGTH];
    int ports[SERVER_NUMBER], segment_number, number_of_ports = 0, sockfd, i, j, file_size;
    struct sockaddr_in local_addr, remote_addr;
    pthread_t threads[SERVER_NUMBER * MAX_CHANNELS];
    download_info info[SERVER_NUMBER * MAX_CHANNELS];
    pthread_mutex_init(&mutex, NULL);

    //<client> nume_fisier nr_segmente port1..portn
    if(argc<3){
        perror("<client>: nume_fisier nr_segmente port1 ... portn\n");
        exit(1);
    }

    strcpy(filename,argv[1]);
    segment_number = atoi(argv[2]);

    if((sockfd = socket(PF_INET, SOCK_STREAM,0))==-1){
        perror("socket error!");
        exit(2);
    }

    set_addr(&local_addr, NULL, INADDR_ANY, 0);
    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr))==-1)
    {
        perror("bind error!");
        exit(3);
    }

    for(i = 3; i < argc; i++){
        //memorez serverele care contin fisierul
        if(set_addr(&remote_addr, host, 0, atoi(argv[i]))){
            perror("set adress errror!");
            exit(4);
        }
        if(connect(sockfd,(struct sockaddr*)&remote_addr, sizeof(remote_addr))){
            perror("connection error!");
            exit(5);
        }

        snprintf(command, 100, "exista %s\n", filename);
        stream_write(sockfd,command,strlen(command));
        stream_read(sockfd, f_size, sizeof(char )*MAX_SIZE_LENGTH);
        if(atoi(f_size) > 0){     
            file_size = atoi(f_size);
            ports[number_of_ports++] = atoi(argv[i]); 
        }

    }

    int segment_size = file_size / segment_number;
    int segment_per_server = segment_number / number_of_ports;
    int segment_for_last_server = segment_per_server + segment_number % number_of_ports;
    int segment_start_address = 0;
    int thread_number = 0;

    printf("segment size:%d\n",segment_size);
    printf("segment_per_server:%d\n",segment_per_server);
    printf("segment_for_last_server:%d\n",segment_for_last_server);

    for(i = 0; i < number_of_ports - 1; i++){

        for(j = 0; j < segment_per_server; j++ ) {

            pthread_mutex_lock(&mutex);
            info[thread_number].port = ports[i];
            info[thread_number].segment_size = segment_size;
            info[thread_number].address = segment_start_address;
            if ( pthread_create( &threads[thread_number++], NULL, threadfunc, (void*) &info[thread_number]) != 0 ) {
	            perror("Couldn't create a new thread!");
	            exit(EXIT_FAILURE);
	        }
            segment_start_address += segment_size;
            pthread_mutex_unlock(&mutex);

        }
    }

    for(j = 0; j < segment_for_last_server; j++ ) {
        pthread_mutex_lock(&mutex);
        info[thread_number].port = ports[number_of_ports-1];
        info[thread_number].segment_size = segment_size;
        info[thread_number].address = segment_start_address;
        if ( pthread_create( &threads[thread_number++], NULL, threadfunc, (void*) &info[thread_number]) != 0 ) {
	        perror("Couldn't create a new thread!");
	        exit(EXIT_FAILURE);
	    }
        segment_start_address += segment_size;
        pthread_mutex_unlock(&mutex);
    }

    for(j = 0; j < thread_number; j++ ) {
        pthread_join(threads[j], NULL);
    }



    //TODO formarea fisierului descarcat




    return 0;
}
