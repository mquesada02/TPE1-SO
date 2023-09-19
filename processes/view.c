// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "../headers/shmADT.h"

#define PARAMETROS 1 
#define LINESIZE 100
#define ERROR -1
int main(int argc, char* argv[]){ 
    sem_t * write_count = sem_open(SEM_WC,O_CREAT,0777,0);
    shmADT buffer = NULL;
    char shm_name[LINESIZE];
    if (argc == 1) {
        size_t size;
        read(STDIN_FILENO,shm_name,LINESIZE);
        buffer = createSHM(shm_name);
    } else {
        buffer = createSHM(argv[1]);
    }

    if (buffer == MAP_FAILED) {
        perror("Error accessing shared memory");
        closeSHM(buffer);
        exit(ERROR);
    }
    
    setvbuf(stdout,NULL,_IONBF,SHM_SIZE);
    while (get_files_left(buffer)){ 
        sem_wait(write_count);
        read_data(buffer);
        file_read(buffer);
    }

    int close_ret = closeSHM(buffer);
    if (close_ret < 0){
        perror("Error cerrando la memoria compartida");
        exit(ERROR);
    }

    return 0;
}
 
