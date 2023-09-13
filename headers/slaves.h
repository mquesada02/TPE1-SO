#ifndef SLAVES_H
#define SLAVES_H

#define ERROR -1
#define TRUE 1

#define SLAVE 0

#define SLAVE_INITIAL_CAPACITY 2 /* 2 files */

#define MAX_FILE_LEN 100
#define MD5_LEN 32
#define MAX_PID_LEN 16
#define BUFFER_MAX_SIZE MAX_FILE_LEN+MD5_LEN+MAX_PID_LEN

#define MD5SUM_LEN 7

typedef enum {READ, WRITE} mode;

#endif