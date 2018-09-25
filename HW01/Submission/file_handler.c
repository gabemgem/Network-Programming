#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "packet_handler.h"
#include "file_handler.h"


int file_open_read(char *filename, int *filedata){
    *filedata = open(filename, O_RDONLY);

    if (*filedata == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }else{
        return 0;
    }
}

int file_buffer_from_pos(transaction *transaction){

    if ((lseek(transaction->filedata, transaction->filepos, SEEK_SET)) == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }

    int num_bytes = read(transaction->filedata, &transaction->filebuffer, 512);
    return num_bytes;
}

int file_open_write(char *filename, int *filedata){
    *filedata = open(filename, O_RDWR | O_CREAT, 0666);

    if (*filedata == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }
    return 0;
}

int file_append_from_buffer(packet *packet, transaction *transaction){

    if ((write(transaction->filedata, packet->data, packet->data_l)) == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }

    return 0;
}

int file_close(int *filedata){

    if ((close(*filedata)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }else{
        return 0;
    }
}
