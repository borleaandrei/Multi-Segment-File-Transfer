//
// Created by sergiu on 2/28/18.
//

#ifndef MULTI_SEGMENT_FILE_TRANSFER_SERVER_H
#define MULTI_SEGMENT_FILE_TRANSFER_SERVER_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>

#include "file_utilities.h"
#include "netio.h"

// Create in the folder where the executable is produced a TEST_SERVER directory, and fill it with some test data
#define SERVER_PORT 5693
#define BUFSIZE 1024

#define MAX_SIZE_LENGTH 20

// Server commands
enum
{
    FILE_EXISTS,
    DOWNLOAD_FILE,
    SHA256_FILE,
};

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
