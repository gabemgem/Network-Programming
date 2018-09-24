#include "packet_handler.h"

int file_open_read(char *filename, int *filedata);
int file_buffer_from_pos(transaction *transaction);
int file_open_write(char *filename, int *filedata);
int file_append_from_buffer(packet *packet, transaction *transaction);
int file_close(int *filedata);
