//
// Created by sergiu on 2/28/18.
//

#ifndef MULTI_SEGMENT_FILE_TRANSFER_SERVER_H
#define MULTI_SEGMENT_FILE_TRANSFER_SERVER_H

#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>

#include "netio.h"

#define SERVER_ROOT "/home/server_PAD"
#define SERVER_PORT 5680
#define BUFSIZE 1024

#define TRUE  1
#define FALSE 0

#define MAX_CHANNELS 100

/*Returns TRUE if filename exists somewhere in SERVER_ROOT, FALSE otherwise*/
uint8_t file_exists(char *filename);

/*Returns the number of available channels (ports to be opened) for file transfers*/
uint16_t no_available_channels(void);

/*Writes to the filedescriptor all the available files for transfer*/
void available_files(int filedescriptor);

#endif //MULTI_SEGMENT_FILE_TRANSFER_SERVER_H
