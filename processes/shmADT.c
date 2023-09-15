#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "shmADT.h"

typedef struct shmCDT{
    int amount_files;
    char shm[SMH_SIZE];
} shmCDT;

shmADT createSHM(char* shm_name){
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRWXG);
    ftruncate(shm_fd, sizeof(shmCDT));
    shmADT toReturn = mmap(NULL, sizeof(shmCDT), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (toReturn == MAP_FAILED) {
        perror("Error creating shared memory");
        exit(errno);
    }
    toReturn->amount_files = 0; //para que si llegara a por alguna razon intentar leer antes de que se dijera la cantidad de files no lea cualquier cosa
    return toReturn;
}

shmADT openSHM(char*shm_name){
    int shm_fd = shm_open(shm_name, O_RDWR, S_IRWXG);
    shmADT toReturn = mmap(NULL, sizeof(shmCDT), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (toReturn == MAP_FAILED) {
        perror("Error accessing shared memory");
        exit(errno);
    }
    return toReturn;
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
    static char write_offset = 0;
    while (*data){
        *(buffer->shm + write_offset) = *data;
        data++;
        write_offset++; //habria que lanzar un error si se pasa del tamaño (?
    }
    *(buffer->shm + write_offset) = '\0';
    write_offset++;
}

void read_data(shmADT buffer, char* data){
    static char read_offset = 0;
    while (*(buffer->shm + read_offset)){
        *data = *(buffer->shm + read_offset);
        data++;
        read_offset++;
    }
    *data = '\0';
    read_offset++;
}

