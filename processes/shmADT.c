// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "../headers/shmADT.h"

int write_offset = 0;
int read_offset = 0;
typedef struct shmCDT{
    char shm[SHM_SIZE];
    int amount_files;
    char * shm_name;
    int shm_fd;
    sem_t * write_count;
} shmCDT;

shmADT createSHM(char* shm_name){
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0777);
    if (ftruncate(shm_fd, sizeof(shmCDT))== -1) {
        perror("FTruncate error");
        exit(-1);
    }
    shmADT toReturn = mmap(NULL, sizeof(shmCDT), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    toReturn->shm_fd = shm_fd;
    toReturn->shm_name = shm_name;
    toReturn->amount_files = 1;
    toReturn->write_count = sem_open(SEM_WC, O_CREAT, S_IRWXU, 0);
    return toReturn;
}

shmADT openSHM(char* shm_name){
    int shm_fd = shm_open(shm_name, O_RDWR, 0777);
    return mmap(NULL, sizeof(shmCDT), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
}

int closeSHM(shmADT buffer){
    shm_unlink(buffer->shm_name);
    close(buffer->shm_fd);
    sem_close(buffer->write_count);
    sem_unlink(SEM_WC);
    return munmap(buffer, SHM_SIZE);
}

void set_file_amount(shmADT buffer, int amount){
    buffer->amount_files = amount;
}

void file_read(shmADT buffer){
    buffer->amount_files -= 1;
}

int get_files_left(shmADT buffer){
    return buffer->amount_files;
}

void write_data(shmADT buffer, char* data){ //data es un string null terminated
    write_offset += sprintf(buffer->shm+write_offset, "%s", data) + 1;
}

void read_data(shmADT buffer){
    int len = strlen(buffer->shm+read_offset);
    printf("%s\n", buffer->shm+read_offset);
    read_offset += len+1;
}
