
#include "server.h"

int main(int argc, char *argv[]){

    int sockfd;
    char buff[BUFSIZE];
    struct sockaddr_in local_addr, remote_addr;
    char host[]="localhost";
    FILE *f;
    char command[100];
    char f_size[MAX_SIZE_LENGTH];
    int idx=0, nr=0, i=0, size=0;
    
    if(argc<3){
        printf("Argumente: port_server nume_fisier\n");
        exit(1);
    }

    if((sockfd = socket(PF_INET, SOCK_STREAM,0))==-1){
        printf("Eroare la scoket!");
        exit(2);
    }

    set_addr(&local_addr, NULL, INADDR_ANY, 0);
    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr))==-1)
    {
        printf("Eroare la bind");
        exit(3);
    }

    if(set_addr(&remote_addr,host,0,atoi(argv[1]))){
        printf("Eroare de adresa!");
        exit(4);
    }

    if(connect(sockfd,(struct sockaddr*)&remote_addr, sizeof(remote_addr))){
        printf("Conectarea la server a esuat");
        exit(5);
    }

    if((f=fopen("file_1","wb"))==NULL){
        printf("Eroare la deschiderea fisierului!");
        exit(6);
    }

    snprintf(command,100,"exista %s\n",argv[2]); // fiecare comanda trebuie sa se termine cu \n

    stream_write(sockfd,command,strlen(command)); // trimit comanda la server

    stream_read(sockfd,f_size,sizeof(char )*MAX_SIZE_LENGTH); // primesc dimensiunea fisierului de la server

    printf("Size = %s\n",f_size);

    if(f_size > 0){ 

        snprintf(command,100,"descarca %s 5000 0\n",argv[2]); // fiecare comanda trebuie sa se termine cu \n
        size = 5000; // descarcam primii 5000 de bytes , 
        stream_write(sockfd,command,strlen(command)); // trimit comanda descarca file 5000 0

        // primesc 5000 de bytes
        while(size>=BUFSIZE && (nr=stream_read(sockfd,buff, BUFSIZE))>0){
            
            fwrite(buff,sizeof(char),nr,f);
            i++;
            size=size-BUFSIZE;
        }

        if(size > 0){
            nr=stream_read(sockfd,buff, size);
            fwrite(buff,sizeof(char),nr,f);
        }


    }
    printf("end\n");
    fclose(f);

    shutdown(sockfd,SHUT_WR);
    exit(0);
}