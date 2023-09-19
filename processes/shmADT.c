// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include "../headers/shmADT.h"

static int write_offset = 0;
static int read_offset = 0;
typedef struct shmCDT{
    char * shm_name;
    int amount_files;
    char shm[SHM_SIZE];
    int shm_fd;
    sem_t * write_count;
} shmCDT;

shmADT createSHM(char* shm_name){
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRWXG);
    ftruncate(shm_fd, sizeof(shmCDT));
    shmADT toReturn = mmap(NULL, sizeof(shmCDT), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    toReturn->shm_fd = shm_fd;
    toReturn->shm_name = shm_name;
    toReturn->amount_files = 0; //para que si llegara a por alguna razon intentar leer antes de que se dijera la cantidad de files no lea cualquier cosa
    toReturn->write_count = sem_open(SEM_WC, O_CREAT, S_IRWXG, 0);
    return toReturn;
}

shmADT openSHM(char* shm_name){
    int shm_fd = shm_open(shm_name, O_RDWR, S_IRWXG);
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
    while (*data){
        *(buffer->shm + write_offset) = *data;
        data++;
        write_offset++; //habria que lanzar un error si se pasa del tamaño (?
    }
    *(buffer->shm + write_offset) = '\0';
    write_offset++;
    sem_post(buffer->write_count);
}

void read_data(shmADT buffer, char* data){
    sem_wait(buffer->write_count);
    while (*(buffer->shm + read_offset)){
        *data = *(buffer->shm + read_offset);
        data++;
        read_offset++;
    }
    *data = '\0';
    read_offset++;
}

