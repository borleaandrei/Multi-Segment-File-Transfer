pthread_mutex_t mutex;

#define SERVER_NUMBER 5

typedef struct download_info {
    int port;
    int address;
    int segment_size;
    int seg_id;
} download_info;
